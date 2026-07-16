# 发现与决策

## 需求
- 语音输入 → LLM 理解 → 返回 JSON {command, speech}
- command 通过 UART 控制 STM32 平衡车（前进/后退/转向/灯光/启停）
- speech 通过 TTS 转为语音由扬声器播放

## 研究发现

### MiMo API 接口（已确认）
- 兼容 OpenAI `chat/completions` 协议
- `mimo-v2.5` 音频理解：支持 WAV/MP3/FLAC/M4A/OGG，Base64 或 URL 传入
- `mimo-v2.5-tts` 语音合成：支持流式 PCM16 24kHz 输出
- 音频 Token 估算：秒数 × 6.25
- 认证方式：`api-key` HTTP Header
- TTS 音色：冰糖(默认中文女声)、茉莉、苏打(男声)、白桦(男声)
- Base URL：`https://api.xiaomimimo.com/v1`

### ESP32-S3-R16N8 硬件规格
- Xtensa LX7 双核 @ 240MHz
- 16MB Flash / 8MB PSRAM (Octal SPI)
- I2S 双通道支持全双工
- 原生 WiFi 4 + BLE 5

### 当前 GPIO 占用分析
- STM32 所有 48 引脚几乎全占
- PA2/PA3(USART2) 原用于 HC-04 蓝牙，可释放
- PA11/PA12 原 HC-04 状态/使能引脚，可释放
- 无额外空闲 UART 可用

## 技术决策

| 决策 | 理由 |
|------|------|
| ESP-IDF v5.x 框架 | I2S 双工需 i2s_chan_config_t，Arduino I2S 库不支持 |
| USART2 波特率 9600→115200 | 提高带宽，减少大文本传输延迟 |
| 16kHz 录音采样率 | MiMo API 音频理解支持，平衡质量与数据量 |
| 24kHz TTS 播放采样率 | MiMo-TTS 默认流式输出格式 |
| WAV 格式上传 | PCM + 44字节头，ESP32 原生支持无压缩 |
| NVS 存储 WiFi 凭据 | 掉电保持，免重复配网 |
| SmartConfig 配网 fallback | 用户通过手机 App 配置 WiFi |
| cJSON 解析 | 轻量级，节省 Flash |
| 每次编写完代码立即编译验证 | 编译失败则返工修改直至通过 |

## 遇到的问题

| 问题 | 解决方案 |
|------|---------|
| INMP441 和 MAX98357A 同时使用 I2S | 共用 BCLK+LRCLK，数据线分用 SD_IN/SD_OUT |
| MiMo 不支持音频文件上传(仅URL或Base64) | 使用 Base64 编码，WAV Base64 后约 50MB 上限内 |
| ESP32 单独供电 vs STM32 供电 | 开发板 USB 独立供电，仅共地 |
| 波特率不匹配需统一 | 两边统一修改为 115200（原 9600） |

## 资源
- MiMo API 文档：https://mimo.mi.com/docs/zh-CN/quick-start/summary/first-api-call
- MiMo TTS 文档：https://mimo.mi.com/docs/zh-CN/quick-start/usage-guide/audio/speech-synthesis-v2.5
- MiMo 音频理解：https://mimo.mi.com/docs/zh-CN/quick-start/usage-guide/multimodal-understanding/audio-understanding
- 现有 FreeRTOS 工程：`平衡车_FreeRTOS/`
- GPIO 引脚表：项目 README.md 表 3.1
