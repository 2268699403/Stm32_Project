# STM32 两轮自平衡车

## 项目概览

三个独立 Keil MDK 工程，共享 `Hardware/` `Library/` `Start/` 目录：

| 工程 | 路径 | 状态 |
|------|------|------|
| 平衡车 (FreeRTOS) | `基于stm32f103的两轮自平衡车/平衡车_FreeRTOS/project.uvprojx` | 正在从裸机版移植——已有 FreeRTOS 内核骨架和硬件驱动，控制逻辑迁移进行中 |
| 平衡车 (裸机) | `基于stm32f103的两轮自平衡车/平衡车_裸机/project.uvprojx` | 完整的串级 PID 控制逻辑（迁移参考源） |
| 遥控器 | `遥控器/Project.uvprojx` | NRF24L01 遥控发送端 |

**移植状态**：按 `平衡车_FreeRTOS/task_plan.md` 分阶段执行。已完成阶段 0-4（清障→BSP→骨架→控制环→人机交互），4 个 FreeRTOS 任务（直立环/速度环/按键/通信）全部验证通过。

## 开发环境

- **IDE**：Keil MDK μVision V5.24（Armcc v5.06）
- **芯片**：STM32F103C8（Cortex-M3，64KB Flash，20KB RAM）
- **工程文件**：`project.uvprojx`（XML 格式，非 Makefile）
- **启动文件**：`Start/startup_stm32f10x_md.s`（中等容量）
- **唯一预定义宏**：`USE_STDPERIPH_DRIVER`

## 构建

1. 用 Keil MDK 打开对应 `.uvprojx`
2. 选择 Target 1 → **Build (F7)** 生成 `Objects/project.axf`
3. **Download (F8)** 通过 SWD 烧录
4. 清理构建产物：`keilkill.bat`（删除 .o/.d/.axf/.map 等）

## FreeRTOS 配置

`User/FreeRTOSConfig.h`：
- V9.0.0，仅动态分配（configSUPPORT_STATIC_ALLOCATION=0，`configSUPPORT_STATIC_ALLOCATION` 已关闭）
- 堆大小 8KB（`configTOTAL_HEAP_SIZE 8192`）
- 系统节拍 1000Hz
- 内存管理：`heap_4.c`
- 移植层：`FreeRTOS/port/RVDS/ARM_CM3/port.c`
- 中断优先级分组 4，可管理的最高中断优先级 5

## FreeRTOS 任务架构（迁移设计）

`task_plan.md` 中定义并已实现的 4 个任务（按优先级从高到低）：

| 优先级 | 任务名 | 周期 | 栈(word) | 职责 |
|--------|--------|------|----------|------|
| 4 | Task_Balance | 10ms | 256 | MPU6050 读取 + 互补滤波 + 越界保护 + 直立环 PID |
| 3 | Task_Speed | 50ms | 256 | Encoder 读取 + 速度环/转向环串级 PID |
| 2 | Task_Key | 20ms | 128 | 按键扫描，短按切换系统启停(En) |
| 1 | Task_Comm | 10ms | 256 | NRF24L01 遥控接收 + OLED 显示(100ms) |

周期通过 `vTaskDelayUntil` 实现恒定周期。LED 由 Task_Balance 控制（ON=运行, OFF=停止）。

**关键架构决策**：
- 移除 `Hardware/Timer.c`（TIM1 中断式调度），改用 FreeRTOS 任务的 `vTaskDelayUntil`
- 并发策略：**不加锁 + 单写者原则**——每个共享变量只允许一个任务写入，利用 Cortex-M3 32 位读写原子性避免竞争
- 角度越界保护时，清零操作仅由直立环任务执行自己的 PID，通过 `En` 标志单向通知速度环自清

## PID 控制架构

`User/PID.c/h` 中三个控制器均使用同一 `PID_Update` 增量式算法：

| 控制器 | 周期 | 反馈 | 默认 Kp/Ki/Kd | 限幅 |
|--------|------|------|---------------|------|
| 直立环 Angle | 10ms | MPU6050 `Angle` | 3.5 / 0.3 / 6.5 | ±100 |
| 速度环 Speed | 50ms | `(RPM_L+RPM_R)/2` | 0.065 / 0.003 / 0 | ±20 |
| 转向环 Turn | 50ms | `RPM_L-RPM_R` | 0.05 / 0 / 0 | ±50 |

电机混合公式：`PWM_L = AvePWM + DifPWM/2`, `PWM_R = AvePWM - DifPWM/2`

**关键事实**：所有控制模块通过全局变量耦合（`Angle`, `RPM_L/R`, `En`, `AvePWM`, `DifPWM` 均在头文件中 `extern` 声明），修改时需同步更新所有引用。

## 蓝牙调参协议

USART2 接受 `参数名:值\n` 格式字符串（如 `Kp:3.5`），解析后直接写入对应的 PID 结构体成员。

## 硬件设计文件

`原理图/` 目录包含 PCB 原理图 PDF、Gerber 文件、PCB 布局图。
GPIO 引脚分配详见 `README.md` 的表 3.1。

## ESP32 AI 语音拓展工程

**目录**：`基于stm32f103的两轮自平衡车/平衡车_ESP拓展/`

### 功能
通过 ESP32-S3 联网调用 MiMo-V2.5 多模态 API，实现：
1. 语音录音 → MiMo 音频理解 → 返回指令+回复文本
2. 文本指令通过 UART 下发 STM32 控制平衡车
3. 回复文本通过 MiMo-TTS 转为语音播放

### 开发环境
- **框架**：ESP-IDF v5.x
- **芯片**：ESP32-S3-R16N8（16MB Flash, 8MB PSRAM）
- **编译**：`idf.py build`
- **烧录**：`idf.py -p PORT flash`
- **监控**：`idf.py -p PORT monitor`

### 硬件连接

**ESP32-S3 ↔ STM32 (UART)**

| ESP32-S3 | STM32 | 说明 |
|----------|-------|------|
| GPIO17 (UART RX) | PA2 (USART2 TX) | 交叉连接 |
| GPIO18 (UART TX) | PA3 (USART2 RX) | 交叉连接 |
| GND | GND | 共地 |

**ESP32-S3 ↔ 音频外设 (I2S)**

| ESP32-S3 引脚 | 连接 | 说明 |
|-------------|------|------|
| GPIO4 (I2S BCLK) | INMP441 SCK + MAX98357A BCLK | 共用时钟 |
| GPIO5 (I2S LRCLK) | INMP441 WS + MAX98357A LRCLK | 共用帧时钟 |
| GPIO6 (I2S SD_IN) | INMP441 SD | 麦克风数据输入 |
| GPIO7 (I2S SD_OUT) | MAX98357A DIN | 扬声器数据输出 |

### ESP32 任务架构

| 任务 | 优先级 | 职责 |
|------|--------|------|
| recorder_task | 5 | I2S 录音(16kHz)→PSRAM 缓冲→WAV 封装 |
| llm_task | 4 | HTTP POST → MiMo 音频理解 → 解析 JSON → 分离 command/speech |
| tts_player_task | 3 | HTTP 流式 → MiMo-TTS → 边收边解码 → I2S 播放 |
| uart_bridge_task | 5 | UART 收发：接收 REC → 发送 command → 收 OK/ERR |
| app_main | 1 | 初始化 + 状态机调度 |

### 通信协议

**ESP32 → STM32（命令下发）**：`CMD:VALUE\n`
- `forward:N` / `back:N` — 前进/后退 N 厘米
- `left:N` / `right:N` — 左转/右转 N 度
- `stop` — 紧急停止
- `light:on` / `light:off` — 车灯开关（映射到板载 LED PC13）
- `balance:on` / `balance:off` — 平衡启停

多条指令用 `;` 分隔：`forward:100;light:on\n`

**STM32 → ESP32（触发/反馈）**：
- `REC\n` — 触发录音（遥控器语音按键按下）
- `OK\n` — 命令执行成功
- `ERR:原因\n` — 执行失败

### 数据流
```
用户语音 → INMP441 → I2S → ESP32 录音 → Base64 WAV
  → POST mimo-v2.5 (音频理解)
  → 返回 {command:"forward:100", speech:"好的，正在前进1米"}
    ├─ command → UART TX → STM32 立即执行
    └─ speech → POST mimo-v2.5-tts (流式PCM16, 音色:冰糖)
                → 边收边播放 → I2S → MAX98357A → 扬声器
```

### API 端点
- Base URL：`https://api.xiaomimimo.com/v1`
- 音频理解模型：`mimo-v2.5`
- TTS 模型：`mimo-v2.5-tts`，音色：`冰糖`
- 认证：Header `api-key: sk-xxxxx`

### NRF24L01 数据包扩展
- 旧格式：`[0x00, LH, LV, RH, RV]`（5字节）
- 新格式：`[0x00, LH, LV, RH, RV, BTN]`（6字节，BTN bit0 = 语音按键）

### 注意事项
- ESP32 独立 USB 供电，不与 STM32 共享电源
- 首次 WiFi 配置通过 AP 模式（SSID: ESP32-Car-Config）
- 录音最长 10 秒，静音 1.5 秒自动截止
- USART2 波特率从 9600 改为 **115200**
- **每次代码编写后立即编译验证，错误则返工修改直至通过**
- 详细实施计划见 `平衡车_ESP拓展/task_plan.md`

## 注意事项

- 不要在非 Keil 环境中尝试编译——这是 ARMCC v5 专有工程
- 裸机版的中断服务（10ms/50ms 任务调度）在 `Hardware/Timer.c` 的 `TIM1_UP_IRQHandler` 中，FreeRTOS 版**计划移除** `Timer.c`（阶段 0.1 待办），改用 FreeRTOS 任务的 `vTaskDelayUntil`
- RAM 上电为随机值，所有全局变量必须在 `main()` 中显式初始化——尤其注意 `PID_Speed.Out` 等结构体成员
- 互补滤波：`Angle = 0.01*AngleAcc + 0.99*AngleGyro`（加速度计权重 0.01，在裸机 `main.c` 中实现）
- 编码器：11 线四倍频（44 PPR），减速比 9.6，RPM 换算公式见 `Encoder.c`
