// BFSAR 音频导出工具
// 将 BFSAR 归档中的音频文件提取为 .bfwav/.bfstm 格式

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <pulsar.h>
#include <pulsar/bfsar/bfsar_wave_archive.h>
#include <pulsar/bfwar/bfwar.h>
#include <pulsar/bfwar/bfwar_file.h>
#include <pulsar/bfwav/bfwav.h>
#include <pulsar/bfwav/bfwav_info.h>
#include <sys/stat.h>

// 全局 BFSAR 归档对象
static PLSR_BFSAR g_bfsar;

// 创建目录
static void createDirectory(const char* path) {
    mkdir(path, 0777);
}

// WAV file header structure
typedef struct {
    char riff[4];           // "RIFF"
    u32 fileSize;           // File size - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    u32 fmtSize;            // 16 for PCM
    u16 audioFormat;        // 1 for PCM
    u16 numChannels;        // 1=Mono, 2=Stereo
    u32 sampleRate;         // Sample rate
    u32 byteRate;           // SampleRate * NumChannels * BitsPerSample/8
    u16 blockAlign;         // NumChannels * BitsPerSample/8
    u16 bitsPerSample;      // 8 or 16
    char data[4];           // "data"
    u32 dataSize;           // Data size
} __attribute__((packed)) WAVHeader;

static int exportSound(u32 index, const char* name, PLSR_BFSARSoundInfo* soundInfo) {
    if (soundInfo->type != PLSR_BFSARSoundType_Wave) {
        return -1; // Only Wave type supported for WAV export
    }
    
    // Open wave archive 0 (qlaunch only has one)
    PLSR_BFSARWaveArchiveInfo waveArchiveInfo;
    if (PLSR_RC_FAILED(plsrBFSARWaveArchiveGet(&g_bfsar, 0, &waveArchiveInfo))) {
        printf("  Failed to get wave archive\n");
        return -1;
    }
    
    PLSR_BFWAR bfwar;
    if (PLSR_RC_FAILED(plsrBFSARWaveArchiveOpen(&g_bfsar, &waveArchiveInfo, &bfwar))) {
        printf("  Failed to open BFWAR\n");
        return -1;
    }
    
    // Get BFWAV file
    u32 waveIndex = soundInfo->wave.index;
    PLSR_BFWARFileInfo fileInfo;
    if (PLSR_RC_FAILED(plsrBFWARFileGet(&bfwar, waveIndex, &fileInfo))) {
        printf("  Failed to get BFWAV file\n");
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    // Open BFWAV
    PLSR_BFWAV bfwav;
    if (PLSR_RC_FAILED(plsrBFWAVOpenInside(&bfwar.ar, fileInfo.offset, &bfwav))) {
        printf("  Failed to open BFWAV\n");
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    // Read BFWAV info
    PLSR_BFWAVInfo info;
    if (PLSR_RC_FAILED(plsrBFWAVReadInfo(&bfwav, &info))) {
        printf("  Failed to read BFWAV info\n");
        plsrBFWAVClose(&bfwav);
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    // Only support PCM formats
    if (info.format == PLSR_BFWAVFormat_DSP_ADPCM) {
        printf("  ADPCM not supported\n");
        plsrBFWAVClose(&bfwav);
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    u16 bitsPerSample = (info.format == PLSR_BFWAVFormat_PCM_16) ? 16 : 8;
    u16 numChannels = info.channelInfoTable.info.count;
    u32 sampleCount = info.sampleCount;
    
    // Calculate sizes
    u32 bytesPerSample = bitsPerSample / 8;
    u32 dataSize = sampleCount * numChannels * bytesPerSample;
    
    // Create WAV header
    WAVHeader header;
    memcpy(header.riff, "RIFF", 4);
    header.fileSize = dataSize + sizeof(WAVHeader) - 8;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmtSize = 16;
    header.audioFormat = 1; // PCM
    header.numChannels = numChannels;
    header.sampleRate = info.sampleRate;
    header.byteRate = info.sampleRate * numChannels * bytesPerSample;
    header.blockAlign = numChannels * bytesPerSample;
    header.bitsPerSample = bitsPerSample;
    memcpy(header.data, "data", 4);
    header.dataSize = dataSize;
    
    // Open output file
    char filename[256];
    snprintf(filename, sizeof(filename), "sdmc:/wav/%s.wav", name);
    FILE* outFile = fopen(filename, "wb");
    if (!outFile) {
        printf("  Failed to create file\n");
        plsrBFWAVClose(&bfwav);
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    // Write WAV header
    fwrite(&header, 1, sizeof(header), outFile);
    
    // Read and write PCM data for each channel
    for (u32 ch = 0; ch < numChannels; ch++) {
        PLSR_BFWAVChannelInfo chInfo;
        if (PLSR_RC_FAILED(plsrBFWAVReadChannelInfo(&bfwav, &info.channelInfoTable, ch, &chInfo))) {
            continue;
        }
        
        u32 channelDataSize = sampleCount * bytesPerSample;
        void* channelData = malloc(channelDataSize);
        if (channelData) {
            if (PLSR_RC_SUCCEEDED(plsrArchiveReadAt(&bfwav.ar, chInfo.dataOffset, channelData, channelDataSize))) {
                fwrite(channelData, 1, channelDataSize, outFile);
            }
            free(channelData);
        }
    }
    
    fclose(outFile);
    plsrBFWAVClose(&bfwav);
    plsrBFWARClose(&bfwar);
    
    return 0;
}

static void detectFormat(void) {
    printf("\n=== FORMAT DETECTION ===\n");
    printf("Analyzing BFSAR wave archives...\n\n");
    
    u32 waveArchiveCount = plsrBFSARWaveArchiveCount(&g_bfsar);
    printf("Wave archives: %u\n\n", waveArchiveCount);
    
    if (waveArchiveCount == 0) {
        printf("No wave archives found!\n\n");
        return;
    }
    
    // Open first wave archive
    PLSR_BFSARWaveArchiveInfo waveArchiveInfo;
    PLSR_RC rc = plsrBFSARWaveArchiveGet(&g_bfsar, 0, &waveArchiveInfo);
    if (PLSR_RC_FAILED(rc)) {
        printf("ERROR: Failed to get wave archive info (0x%08X)\n\n", rc);
        return;
    }
    
    printf("Wave archive 0:\n");
    if (waveArchiveInfo.hasWaveCount) {
        printf("  Wave count: %u\n", waveArchiveInfo.waveCount);
    }
    
    // Open BFWAR
    PLSR_BFWAR bfwar;
    rc = plsrBFSARWaveArchiveOpen(&g_bfsar, &waveArchiveInfo, &bfwar);
    if (PLSR_RC_FAILED(rc)) {
        printf("  ERROR: Failed to open BFWAR (0x%08X)\n\n", rc);
        return;
    }
    
    printf("  Opened BFWAR successfully\n");
    
    u32 fileCount = plsrBFWARFileCount(&bfwar);
    printf("  Files in BFWAR: %u\n", fileCount);
    
    if (fileCount > 0) {
        // Get first BFWAV file
        PLSR_BFWARFileInfo fileInfo;
        rc = plsrBFWARFileGet(&bfwar, 0, &fileInfo);
        if (PLSR_RC_SUCCEEDED(rc)) {
            printf("  Opening first BFWAV...\n");
            
            // Open BFWAV
            PLSR_BFWAV bfwav;
            rc = plsrBFWAVOpenInside(&bfwar.ar, fileInfo.offset, &bfwav);
            if (PLSR_RC_SUCCEEDED(rc)) {
                // Read BFWAV info
                PLSR_BFWAVInfo info;
                rc = plsrBFWAVReadInfo(&bfwav, &info);
                if (PLSR_RC_SUCCEEDED(rc)) {
                    printf("\n  === AUDIO FORMAT ===\n");
                    printf("  Sample Rate:  %u Hz\n", info.sampleRate);
                    printf("  Channels:     %u\n", info.channelInfoTable.info.count);
                    printf("  Sample Count: %u\n", info.sampleCount);
                    printf("  Looping:      %s\n", info.looping ? "Yes" : "No");
                    
                    printf("  Format:       ");
                    switch(info.format) {
                        case PLSR_BFWAVFormat_PCM_8:
                            printf("PCM-8 (8-bit unsigned)\n");
                            printf("  STATUS: CAN CONVERT TO WAV!\n");
                            break;
                        case PLSR_BFWAVFormat_PCM_16:
                            printf("PCM-16 (16-bit signed)\n");
                            printf("  STATUS: CAN CONVERT TO WAV!\n");
                            break;
                        case PLSR_BFWAVFormat_DSP_ADPCM:
                            printf("DSP-ADPCM (4-bit compressed)\n");
                            printf("  STATUS: NEED ADPCM DECODER!\n");
                            break;
                        default:
                            printf("Unknown (%u)\n", info.format);
                            break;
                    }
                }
                plsrBFWAVClose(&bfwav);
            }
        }
    }
    
    plsrBFWARClose(&bfwar);
    
    printf("\n=== DETECTION COMPLETE ===\n");
    printf("Press + to exit\n\n");
}

// Export raw BFWAV file (for vgmstream conversion)
static int exportRawBFWAV(u32 index, const char* name, PLSR_BFSARSoundInfo* soundInfo) {
    if (soundInfo->type != PLSR_BFSARSoundType_Wave) {
        return -1; // Only Wave type supported
    }
    
    // Open wave archive 0 (qlaunch only has one)
    PLSR_BFSARWaveArchiveInfo waveArchiveInfo;
    if (PLSR_RC_FAILED(plsrBFSARWaveArchiveGet(&g_bfsar, 0, &waveArchiveInfo))) {
        printf("  Failed to get wave archive\n");
        return -1;
    }
    
    PLSR_BFWAR bfwar;
    if (PLSR_RC_FAILED(plsrBFSARWaveArchiveOpen(&g_bfsar, &waveArchiveInfo, &bfwar))) {
        printf("  Failed to open BFWAR\n");
        return -1;
    }
    
    // Get BFWAV file
    u32 waveIndex = soundInfo->wave.index;
    PLSR_BFWARFileInfo fileInfo;
    if (PLSR_RC_FAILED(plsrBFWARFileGet(&bfwar, waveIndex, &fileInfo))) {
        printf("  Failed to get BFWAV file\n");
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    // Read entire BFWAV file data
    void* bfwavData = malloc(fileInfo.size);
    if (!bfwavData) {
        printf("  Memory allocation failed\n");
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    if (PLSR_RC_FAILED(plsrArchiveReadAt(&bfwar.ar, fileInfo.offset, bfwavData, fileInfo.size))) {
        printf("  Failed to read BFWAV data\n");
        free(bfwavData);
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    // Open output file
    char filename[256];
    snprintf(filename, sizeof(filename), "sdmc:/bfwav/%s.bfwav", name);
    FILE* outFile = fopen(filename, "wb");
    if (!outFile) {
        printf("  Failed to create file\n");
        free(bfwavData);
        plsrBFWARClose(&bfwar);
        return -1;
    }
    
    // Write BFWAV file
    fwrite(bfwavData, 1, fileInfo.size, outFile);
    fclose(outFile);
    free(bfwavData);
    plsrBFWARClose(&bfwar);
    
    return 0;
}

// Export all as raw BFWAV (for vgmstream)
static void exportAllRaw(void) {
    u32 total = plsrBFSARSoundCount(&g_bfsar);
    u32 exported = 0;
    u32 skipped = 0;
    u32 failed = 0;

    printf("\nStarting raw BFWAV export...\n");
    printf("Total sounds: %u\n\n", total);

    createDirectory("/bfwav");

    for (u32 i = 0; i < total; i++) {
        PLSR_BFSARSoundInfo soundInfo;
        char name[256];

        if (PLSR_RC_FAILED(plsrBFSARSoundGet(&g_bfsar, i, &soundInfo))) {
            printf("[%u/%u] Failed to get sound info\n", i+1, total);
            failed++;
            continue;
        }

        if (soundInfo.hasStringIndex) {
            if (PLSR_RC_FAILED(plsrBFSARStringGet(&g_bfsar, soundInfo.stringIndex, name, sizeof(name)))) {
                snprintf(name, sizeof(name), "sound_%03u", i);
            }
        } else {
            snprintf(name, sizeof(name), "sound_%03u", i);
        }

        // Export based on type
        switch(soundInfo.type) {
            case PLSR_BFSARSoundType_Wave:
                printf("[%u/%u] %s (Wave)\n", i+1, total, name);
                if (exportRawBFWAV(i, name, &soundInfo) == 0) {
                    exported++;
                } else {
                    failed++;
                }
                break;

            case PLSR_BFSARSoundType_Stream:
                printf("[%u/%u] %s (Stream - skip)\n", i+1, total, name);
                skipped++;
                break;

            case PLSR_BFSARSoundType_Sequence:
                printf("[%u/%u] %s (Sequence - skip)\n", i+1, total, name);
                skipped++;
                break;

            default:
                printf("[%u/%u] %s (Unknown - skip)\n", i+1, total, name);
                skipped++;
                break;
        }
    }

    printf("\n=== Export Complete ===\n");
    printf("Exported: %u\n", exported);
    printf("Skipped:  %u\n", skipped);
    printf("Failed:   %u\n", failed);
    printf("Saved to: sdmc:/bfwav/\n\n");
}

// 批量导出所有音频文件
static void exportAll(void) {
    u32 total = plsrBFSARSoundCount(&g_bfsar);
    u32 exported = 0;
    u32 skipped = 0;
    u32 failed = 0;

    printf("\nStarting export...\n");
    printf("Total sounds: %u\n\n", total);

    createDirectory("/wav");

    for (u32 i = 0; i < total; i++) {
        PLSR_BFSARSoundInfo soundInfo;
        char name[256];

        if (PLSR_RC_FAILED(plsrBFSARSoundGet(&g_bfsar, i, &soundInfo))) {
            printf("[%u/%u] Failed to get sound info\n", i+1, total);
            failed++;
            continue;
        }

        if (soundInfo.hasStringIndex) {
            if (PLSR_RC_FAILED(plsrBFSARStringGet(&g_bfsar, soundInfo.stringIndex, name, sizeof(name)))) {
                snprintf(name, sizeof(name), "sound_%03u", i);
            }
        } else {
            snprintf(name, sizeof(name), "sound_%03u", i);
        }

        // Export based on type
        switch(soundInfo.type) {
            case PLSR_BFSARSoundType_Wave:
                printf("[%u/%u] %s (Wave)\n", i+1, total, name);
                if (exportSound(i, name, &soundInfo) == 0) {
                    exported++;
                } else {
                    failed++;
                }
                break;

            case PLSR_BFSARSoundType_Stream:
                printf("[%u/%u] %s (Stream - skip)\n", i+1, total, name);
                skipped++;
                break;

            case PLSR_BFSARSoundType_Sequence:
                printf("[%u/%u] %s (Sequence - skip)\n", i+1, total, name);
                skipped++;
                break;

            default:
                printf("[%u/%u] %s (Unknown - skip)\n", i+1, total, name);
                skipped++;
                break;
        }
    }

    printf("\n=== Export Complete ===\n");
    printf("Exported: %u\n", exported);
    printf("Skipped:  %u\n", skipped);
    printf("Failed:   %u\n", failed);
    printf("Saved to: sdmc:/wav/\n\n");
}

int main(int argc, char* argv[])
{
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);
    PLSR_RC rc = PLSR_RC_OK;
    Result result = 0;

    printf("BFSAR Audio Exporter\n");
    printf("====================\n\n");

    printf("Mounting qlaunch ROMFS...\n");
    result = romfsMountDataStorageFromProgram(0x0100000000001000, "qlaunch");
    if (R_FAILED(result)) {
        printf("Failed: 0x%x\n", result);
        printf("Press + to exit\n");
    } else {
        printf("OK!\n\n");

        printf("Opening BFSAR...\n");
        rc = plsrBFSAROpen("qlaunch:/sound/qlaunch.bfsar", &g_bfsar);
        if (PLSR_RC_FAILED(rc)) {
            printf("Failed: 0x%08X\n", rc);
            printf("Press + to exit\n");
        } else {
            printf("OK! Found %u sounds\n\n", plsrBFSARSoundCount(&g_bfsar));
            printf("Press A to export WAV files (PCM only)\n");
            printf("Press X to export raw BFWAV (all formats)\n");
            printf("Press B to detect format\n");
            printf("Press + to exit\n");
        }
    }

    while (appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_A) {
            if (R_SUCCEEDED(result) && PLSR_RC_SUCCEEDED(rc)) {
                exportAll();
            }
        }

        if (kDown & HidNpadButton_X) {
            if (R_SUCCEEDED(result) && PLSR_RC_SUCCEEDED(rc)) {
                exportAllRaw();
            }
        }

        if (kDown & HidNpadButton_B) {
            if (R_SUCCEEDED(result) && PLSR_RC_SUCCEEDED(rc)) {
                detectFormat();
            }
        }

        if (kDown & HidNpadButton_Plus) {
            break;
        }

        consoleUpdate(NULL);
    }

    if (PLSR_RC_SUCCEEDED(rc)) {
        plsrBFSARClose(&g_bfsar);
    }

    consoleExit(NULL);
    return 0;
}
