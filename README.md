# Nintendo Switch 官方音效文件集

[English](#english-version) | 中文

本仓库收录了从 Nintendo Switch 中提取的 211 项官方音效文件。

### 文件夹结构

```
Nintendo-Switch-Sounds/
├── BFWAV/                           # Switch原始音频格式（211个文件）
├── WAV/                             # 转换后的WAV格式（211个文件）
└── Switch-Audio-Extractor/          # 音频提取工具
```

### 音频格式说明

- **BFWAV 格式**：Nintendo Switch 专有的音频文件格式，需要特定工具才能播放
- **WAV 格式**：标准的波形音频格式，可在任何播放器中直接播放

## 音效类型

本仓库包含的音效涵盖了 Switch 系统的各种交互场景：

### UI 交互音效
- **按钮音效**：焦点移动、确认、取消等
- **菜单音效**：进入、退出、滚动等
- **触摸音效**：触屏反馈音效

### 系统功能音效
- **启动音效**：各个应用的启动音
- **通知音效**：系统通知、警告提示
- **解锁音效**：屏幕解锁及相关操作音

### 应用相关音效
- **相册** (Album)
- **eShop 商店** (Shop)
- **新闻** (News)
- **设置** (Set)
- **我的页面** (Mypage)
- **控制器设置** (Controller)
- **网络相关** (Nso)
- **家长控制** (Vgc) 等

### 典型音效示例

-  `SeBtnDecide.bfwav`             按钮确认音效 
-  `SeBtnFocus.bfwav`              按钮获得焦点音效 
-  `SeGameIconDecide.bfwav`        选择游戏图标音效 
-  `SeUnlockHome.bfwav`            Home键解锁音效 
-  `StartupMenu_Game.bfwav`        游戏菜单启动音效 
-  `SeSuccess.bfwav`               操作成功音效 
-  `SeWarning.bfwav`               警告提示音效 

## 提取工具

本仓库包含了用于从 Switch 系统文件中提取音频的工具：

### Switch-Audio-Extractor

这是一个基于 [switch-libpulsar](https://github.com/p-sam/switch-libpulsar) 库开发的音频提取工具，能够：

- 从 BFSAR 归档文件中提取音频
- 导出为 BFWAV 原始格式
- 转换为 WAV 标准格式（仅支持 PCM 格式）
- 自动检测音频格式和参数

**注意** 这个工具完全由AI编写，我一个标点符号都没写。所以代码混乱，质量极差，但是既然提取完成了，这个工具的使命也就结束了，没有必要去重构代码。


## 版权声明

**重要提示**：本仓库中的所有音频文件版权归 **任天堂株式会社 (Nintendo Co., Ltd.)** 所有。

这些文件仅供：
- 个人学习和研究使用
- 非商业用途参考

**禁止**用于：
- 商业项目
- 声称所有权
- 任何侵犯任天堂版权的行为

如需商业使用，请联系任天堂公司获取正式授权。

## 致谢

- [switch-libpulsar](https://github.com/p-sam/switch-libpulsar) - 提供了优秀的音频解析库

---
---

# <a id="english-version"></a>Nintendo Switch Official Sound Effects Collection

[中文](#nintendo-switch-官方音效文件集) | English

This repository contains 211 official sound effect files extracted from the Nintendo Switch system.

### Directory Structure

```
Nintendo-Switch-Sounds/
├── BFWAV/                           # Switch native audio format (211 files)
├── WAV/                             # Converted WAV format (211 files)
└── Switch-Audio-Extractor/          # Audio extraction tool
```

### Audio Format Description

- **BFWAV Format**: Nintendo Switch proprietary audio file format, requires specific tools to play
- **WAV Format**: Standard waveform audio format, can be played directly in any player

## Sound Effect Types

This repository contains sound effects covering various Switch system interaction scenarios:

### UI Interaction Sound Effects
- **Button sounds**: Focus movement, confirm, cancel, etc.
- **Menu sounds**: Enter, exit, scroll, etc.
- **Touch sounds**: Touchscreen feedback sounds

### System Function Sound Effects
- **Startup sounds**: Startup sounds for various applications
- **Notification sounds**: System notifications, warning alerts
- **Unlock sounds**: Screen unlock and related operation sounds

### Application-related Sound Effects
- **Album**
- **eShop**
- **News**
- **Settings** (Set)
- **My Page** (Mypage)
- **Controller** settings
- **Network** related (Nso)
- **Parental Controls** (Vgc), etc.

### Typical Sound Effect Examples

  `SeBtnDecide.bfwav`             Button confirm sound
  `SeBtnFocus.bfwav`              Button focus sound
  `SeGameIconDecide.bfwav`        Game icon selection sound
  `SeUnlockHome.bfwav`            Home button unlock sound
  `StartupMenu_Game.bfwav`        Game menu startup sound
  `SeSuccess.bfwav`               Success sound
  `SeWarning.bfwav`               Warning sound

## Extraction Tool

This repository includes a tool for extracting audio from Switch system files:

### Switch-Audio-Extractor

This is an audio extraction tool developed based on the [switch-libpulsar](https://github.com/p-sam/switch-libpulsar) library, capable of:

- Extracting audio from BFSAR archive files
- Exporting to BFWAV native format
- Converting to WAV standard format (PCM format only)
- Automatically detecting audio format and parameters

**Note:** This tool was entirely written by AI - I didn't write a single punctuation mark. So the code is messy and of poor quality, but since the extraction is complete, the tool's mission is over and there's no need to refactor the code.


## Copyright Notice

**Important Notice**: All audio files in this repository are copyrighted by **Nintendo Co., Ltd.**

These files are for:
- Personal learning and research use only
- Non-commercial reference purposes

**Prohibited** uses:
- Commercial projects
- Claiming ownership
- Any infringement of Nintendo's copyright

For commercial use, please contact Nintendo for official authorization.

## Acknowledgments

- [switch-libpulsar](https://github.com/p-sam/switch-libpulsar) - Provided excellent audio parsing library





