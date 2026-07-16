# 任务计划：STM32 平衡车 ESP32-S3 AI 语音拓展

## 目标
通过 ESP32-S3 联网调用 MiMo-V2.5 多模态模型，实现语音交互控制两轮自平衡车（录音→LLM理解→指令下发+TTS播报）。

## 当前阶段
阶段 3 — STM32 侧实现

## 各阶段

### 阶段 1：需求与发现 ✅ complete
- [x] 理解用户意图：语音→LLM→指令+语音反馈
- [x] 确定硬件方案：ESP32-S3-R16N8 + INMP441 + MAX98357A
- [x] 确认 API 方案：MiMo-V2.5（音频理解）+ MiMo-V2.5-TTS（语音合成）
- [x] 确定通信方式：USART2(PA2/PA3) 替代 HC-04 蓝牙
- [x] 确认录音触发：遥控器按键 → NRF → STM32 → UART → ESP32

### 阶段 2：规划与结构 ✅ complete
- [x] 确认 MiMo API 接口细节（OpenAI 兼容，Base64 WAV 输入，流式 PCM16 输出）
- [x] GPIO 引脚分配确认
- [x] 通信协议设计（CMD:VALUE\n 格式）
- [x] ESP32 任务架构设计（5 任务状态机）
- [x] 创建规划文档和项目骨架
- [x] 更新 AGENTS.md

### 阶段 3：STM32 侧实现 🔄 in_progress
- [ ] 3.1 `Hardware/USART2.h` — 替换蓝牙协议为 ESP32 命令协议
- [ ] 3.2 `Hardware/USART2.c` — 实现 ESP32 命令解析器
- [ ] 3.3 `User/PID.h` — 新增命令全局变量和运动控制变量
- [ ] 3.4 `User/main.c` — 新增 Task_Cmd，启用 USART2，扩展 NRF 数据包解析
- [ ] 3.5 编译验证 — `keil_build` 检查 0 错误
- [ ] 3.6 如编译错误则返工修改直至通过
- **状态：** in_progress

### 阶段 4：ESP32 侧实现
- [ ] 4.1 CMakeLists.txt + sdkconfig.defaults + partitions.csv 项目骨架
- [ ] 4.2 wifi_mgr.c — WiFi STA 连接 + AP 配网 fallback
- [ ] 4.3 recorder.c — I2S 录音(INMP441, 16kHz mono)
- [ ] 4.4 player.c — I2S 播放(MAX98357A, 24kHz mono, 流式)
- [ ] 4.5 mimo_api.c — MiMo HTTP API 封装（音频理解 + TTS 流式）
- [ ] 4.6 uart_bridge.c — UART 收发任务，与 STM32 通信
- [ ] 4.7 main.c — 应用入口，状态机调度
- [ ] 4.8 编译验证 — `idf.py build` 检查 0 错误
- [ ] 4.9 如编译错误则返工修改直至通过
- **状态：** pending

### 阶段 5：遥控器扩展
- [ ] 5.1 NRF24L01 数据包增加第6字节按键标志(BTN)
- [ ] 5.2 STM32 Task_Comm 适配 6 字节接收
- [ ] 5.3 编译验证
- [ ] 5.4 如编译错误则返工修改直至通过
- **状态：** pending

### 阶段 6：联调与验证
- [ ] 6.1 STM32 串口助手模拟 ESP32 命令→验证命令解析
- [ ] 6.2 ESP32 串口终端→验证录音+API调用+播放
- [ ] 6.3 STM32 ↔ ESP32 UART 联调
- [ ] 6.4 完整链路：语音输入→指令执行+TTS播放
- [ ] 6.5 每次集成后编译验证，错误即返工
- **状态：** pending

### 阶段 7：交付
- [ ] 完整工程验证
- [ ] 文档更新
- **状态：** pending

## 已做决策

| 决策 | 理由 |
|------|------|
| 使用 ESP-IDF 而非 Arduino | I2S 双工支持、流式 HTTP 稳定性、PSRAM 管理需求 |
| USART2 替代 HC-04 蓝牙 | ESP32 自带 WiFi/BLE 可完全取代蓝牙调参 |
| 两轮 API 调用而非三轮 | `mimo-v2.5` 原生支持音频输入→文本输出，不需单独 ASR |
| TTS 流式 + 指令并行下发 | command 不等 TTS 完成，减少用户感知延迟 |
| ESP32 独立 USB 供电 | 避免 STM32 电源负担，仅共地 |
| 暂不做唤醒词 | 初期降低复杂度，遥控器按键触发 |
| 波特率提高到 115200 | 提高 UART 带宽，减少大文本传输延迟 |

## 编译规范

**每次编写完代码后立即编译验证，编译失败则返工修改直至通过。**
- STM32 侧：使用 `keil_build` 工具编译 `平衡车_FreeRTOS/project.uvprojx`
- ESP32 侧：使用 `idf.py build` 编译 ESP-IDF 工程
- 遥控器侧：使用 `keil_build` 工具编译 `遥控器/Project.uvprojx`

## 关键约束

- STM32F103C8：64KB Flash / 20KB RAM，剩余空间有限，命令解析需精简
- ESP32-S3：16MB Flash / 8MB PSRAM，足够录音缓冲区(~2.5MB/10s PCM)
- 波特率 115200：STM32 USART2 需从 9600 改为 115200，ESP32 侧匹配
- NRF 数据包：32 字节容量，扩展 1 字节按键标志不影响现有逻辑
- HC-04 蓝牙移除：USART2 重定向至 ESP32，PA11/PA12 释放

## 遇到的错误
| 错误 | 解决方案 |
|------|---------|
| (暂无) | |
