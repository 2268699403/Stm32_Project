# STM32两轮自平衡小车

[![GitHub](https://img.shields.io/badge/GitHub-Repository-blue)](https://github.com/2268699403/Stm32_Project)

基于 **STM32F103C8T6** 主控芯片的双轮自平衡小车项目，采用 **PID 串级控制算法**（直立环 + 速度环 + 转向环），搭配 **MPU6050** 姿态传感器进行角度检测，**TB6612** 驱动直流减速编码电机，支持 **NRF24L01** 无线遥控。新增 **ESP32-S3** 拓展模块，通过 **MiMo-V2.5** 大模型实现语音交互控制与语音反馈。

---

## 1. 项目简介

本项目实现了一款双轮自平衡小车，核心控制策略为 **直立环 + 速度环 + 转向环三环串级 PID**：

- **直立环**：以 MPU6050 解算的车身角度为输入，控制电机使车身保持直立平衡。
- **速度环**：以左右轮编码器测速的均值为输入，控制小车的行进速度。
- **转向环**：以左右轮差速为输入，控制小车的转向。

小车可通过 **NRF24L01 无线遥控器** 远程控制前进/后退与转向，同时支持通过 **HC-04 蓝牙模块** 在线调节 PID 参数，方便调试。

---

## 2. 硬件清单

| 组件 | 型号/规格 | 数量 |
|------|-----------|:----:|
| 主控芯片 | STM32F103C8T6（蓝色开发板） | 1 |
| 姿态传感器 | MPU6050（六轴陀螺仪+加速度计） | 1 |
| 电机驱动 | TB6612 | 1 |
| 直流减速电机 | 带霍尔编码器 | 2 |
| 无线通信模块 | NRF24L01（2.4GHz） | 2 |
| 显示模块 | 0.96寸 OLED（I2C接口，SSD1306） | 1 |
| AI 拓展模块 | ESP32-S3-R16N8（16MB Flash, 8MB PSRAM） | 1 |
| 麦克风 | INMP441（I2S MEMS） | 1 |
| 扬声器驱动 | MAX98357AETE + 3525 腔体喇叭 | 1 |
| 电池 | 3.7V 18650 锂电池 | 2 |
| 按键 | 轻触按键 | 4 |
| LED | 指示灯 | 若干 |

---

## 3. 硬件连接

### 3.1 STM32F103C8T6 引脚分配表

| GPIO | 网络标号 | 连接模块 | 功能说明 |
|:----:|:--------:|---------|---------|
| PA0 | PWMA | TB6612 | 电机A PWM 调速 |
| PA1 | PWMB | TB6612 | 电机B PWM 调速 |
| PA2 | USART2_TX | ESP32-S3 | ESP32 通信（USART2 TX → ESP32 RX） |
| PA3 | USART2_RX | ESP32-S3 | ESP32 通信（USART2 RX → ESP32 TX） |
| PA4 | K4 | 按键4 | GPIO 输入（短按/长按） |
| PA5 | K3 | 按键3 | GPIO 输入（短按/长按） |
| PA6 | E1A | 编码电机1 | 编码器 A 相 |
| PA7 | E1B | 编码电机1 | 编码器 B 相 |
| PA8 | CE | NRF24L01 | SPI 片选（CE） |
| PA9 | TX | UART排针 | 串口1 发送（USART1 TX） |
| PA10 | RX | UART排针 | 串口1 接收（USART1 RX） |
| PA11 | (已释放) | 原 HC-04 STATE | 可用于车灯等扩展功能 |
| PA12 | (已释放) | 原 HC-04 EN | 可用于车灯等扩展功能 |
| PA15 | CSN | NRF24L01 | SPI 从机选择（CSN） |
| PB0 | K2 | 按键2 | GPIO 输入（短按/长按） |
| PB1 | K1 | 按键1 | GPIO 输入（短按/长按） |
| PB3 | SCK | NRF24L01 | SPI 时钟 |
| PB4 | MISO | NRF24L01 | SPI 主机输入/从机输出 |
| PB5 | MOSI | NRF24L01 | SPI 主机输出/从机输入 |
| PB6 | E2A | 编码电机2 | 编码器 A 相 |
| PB7 | E2B | 编码电机2 | 编码器 B 相 |
| PB8 | O-SCL | OLED 模块 | I2C 时钟线 |
| PB9 | O-SDA | OLED 模块 | I2C 数据线 |
| PB10 | M-SCL | MPU6050 | I2C 时钟线 |
| PB11 | M-SDA | MPU6050 | I2C 数据线 |
| PB12 | AIN1 | TB6612 | 电机A 方向控制 1 |
| PB13 | AIN2 | TB6612 | 电机A 方向控制 2 |
| PB14 | BIN1 | TB6612 | 电机B 方向控制 1 |
| PB15 | BIN2 | TB6612 | 电机B 方向控制 2 |

### 3.2 TB6612 驱动板接线

| TB6612 引脚 | 连接目标 | 说明 |
|:----------:|---------|------|
| VM | VIN（电池正极） | 电机电源（6V~12V） |
| VCC | VCC_3.3V | 逻辑电源 3.3V |
| GND | GND | 电源地 |
| AO1 | 电机1 M1- | 电机A 输出 |
| AO2 | 电机1 M1+ | 电机A 输出 |
| BO1 | 电机2 M2- | 电机B 输出 |
| BO2 | 电机2 M2+ | 电机B 输出 |
| PWMA | PA0 | 电机A PWM 调速 |
| PWMB | PA1 | 电机B PWM 调速 |
| AIN1 | PB12 | 电机A 方向 |
| AIN2 | PB13 | 电机A 方向 |
| BIN1 | PB14 | 电机B 方向 |
| BIN2 | PB15 | 电机B 方向 |
| STBY | VCC_3.3V | 待机控制（高电平使能） |

### 3.3 编码电机接线

| 编码电机 | A相 | B相 | 电机正极 | 电机负极 |
|---------|:---:|:---:|:--------:|:--------:|
| 电机1（左） | PA6 (E1A) | PA7 (E1B) | TB6612 AO2 (M1+) | TB6612 AO1 (M1-) |
| 电机2（右） | PB6 (E2A) | PB7 (E2B) | TB6612 BO2 (M2+) | TB6612 BO1 (M2-) |

### 3.4 模块接线示意

- **MPU6050** → PB10 (SCL)、PB11 (SDA)，I2C 通信
- **OLED** → PB8 (SCL)、PB9 (SDA)，I2C 通信
- **NRF24L01** → PA8 (CE)、PA15 (CSN)、PB3 (SCK)、PB5 (MOSI)、PB4 (MISO)，SPI 通信
- **ESP32-S3** → PA2 (TX)、PA3 (RX)，USART2 115200 通信
- **UART排针** → PA9 (TX)、PA10 (RX)，USART1 调试通信

### 3.5 ESP32-S3 音频外设接线

| ESP32-S3 引脚 | 连接 | 说明 |
|-------------|------|------|
| GPIO17 (UART RX) | STM32 PA2 (USART2 TX) | 交叉连接 |
| GPIO18 (UART TX) | STM32 PA3 (USART2 RX) | 交叉连接 |
| GPIO4 (I2S BCLK) | INMP441 SCK + MAX98357A BCLK | 共用时钟 |
| GPIO5 (I2S LRCLK) | INMP441 WS + MAX98357A LRCLK | 共用帧时钟 |
| GPIO6 (I2S SD_IN) | INMP441 SD | 麦克风数据输入 |
| GPIO7 (I2S SD_OUT) | MAX98357A DIN | 扬声器数据输出 |

> ESP32-S3 开发板通过 **USB 独立供电**，不与 STM32 共享电源，仅共地。

---

## 4. 项目架构

### 4.1 目录结构

```
├── User/                    # 用户层，主逻辑与控制算法
│   ├── main.c               # 主函数，系统初始化与主循环
│   ├── PID.c                # PID 控制器实现
│   ├── PID.h                # PID 结构体定义与函数声明
│   ├── stm32f10x_conf.h     # 外设头文件配置
│   ├── stm32f10x_it.c       # 中断服务函数
│   └── stm32f10x_it.h       # 中断声明
│
├── System/                  # 系统层，基础延时
│   ├── Delay.c              # 延时函数实现
│   └── Delay.h              # 延时函数头文件
│
├── Hardware/                # 硬件层，各模块驱动
│   ├── Encoder.c/h          # 正交编码器读数
│   ├── Key.c/h              # 按键扫描（短按/长按）
│   ├── LED.c/h              # LED 指示灯
│   ├── Motor.c/h            # 电机控制（方向+速度）
│   ├── MPU6050.c/h          # MPU6050 初始化、数据读取
│   ├── MPU6050_Reg.h        # MPU6050 寄存器地址定义
│   ├── NRF24L01.c/h         # NRF24L01 无线收发
│   ├── NRF24L01_Define.h    # NRF24L01 寄存器与指令定义
│   ├── OLED.c/h             # OLED 显示驱动
│   ├── OLED_Data.c/h        # OLED 字库与图像数据
│   ├── PWM.c/h              # PWM 输出初始化
│   ├── Timer.c/h            # 定时器中断（控制周期）
│   ├── USART1.c/h           # 串口1（调试用）
│   └── USART2.c/h           # 串口2（ESP32 通信，115200）
│
├── Library/                 # 标准外设库（STM32F10x StdPeriph）
│   ├── misc.c/h
│   ├── stm32f10x_*.c/h      # 各外设库文件
│   └── ...
│
├── Start/                   # 启动文件与CMSIS
│   ├── startup_stm32f10x_md.s  # 启动文件（中等容量）
│   ├── core_cm3.c/h         # Cortex-M3 内核文件
│   ├── stm32f10x.h          # STM32F10x 寄存器定义
│   └── system_stm32f10x.c/h # 系统时钟配置
│
├── DebugConfig/             # Keil 调试配置
├── Listings/                # 编译列表文件
├── Objects/                 # 编译目标文件
├── project.uvprojx          # Keil 工程文件
├── project.uvoptx           # Keil 工程选项
└── project.uvguix.22686     # Keil 工程用户配置
│
├── 平衡车_ESP拓展/            # ESP32-S3 AI 语音拓展工程
│   ├── task_plan.md             # 实施计划
│   ├── findings.md              # 技术发现与决策
│   ├── progress.md              # 进度日志
│   ├── CMakeLists.txt           # ESP-IDF 顶层 cmake
│   ├── sdkconfig.defaults       # 默认 Kconfig 配置
│   ├── partitions.csv           # Flash 分区表
│   └── main/                    # 应用主代码
│       ├── CMakeLists.txt       # 组件注册
│       ├── main.c               # 入口 + 状态机
│       ├── wifi_mgr.c/h         # WiFi STA 连接 + AP 配网
│       ├── recorder.c/h         # I2S 录音 (INMP441)
│       ├── player.c/h           # I2S 播放 (MAX98357A)
│       ├── mimo_api.c/h         # MiMo HTTP API 封装
│       ├── uart_bridge.c/h      # UART 与 STM32 通信
│       └── app_state.h          # 应用状态机定义
│
└── 遥控器/                     # NRF24L01 遥控器工程
```

### 4.2 软件层次说明

| 层次 | 说明 |
|------|------|
| **User（用户层）** | 包含主函数 `main.c` 和控制算法 `PID.c`，负责系统初始化、FreeRTOS 任务调度与 PID 运算 |
| **System（系统层）** | 基础的延时函数，为上层提供时间基准 |
| **Hardware（硬件层）** | 各硬件模块的底层驱动，向上层提供标准化接口 |
| **Library（库层）** | STM32 标准外设库，Hardware 层基于此库进行寄存器与外设操作 |
| **Start（启动层）** | 启动文件、CMSIS 内核文件与系统时钟配置 |
| **ESP32（AI 层）** | ESP-IDF 工程，独立于 STM32 项目，负责录音/API 调用/TTS 播放，通过 UART 下发指令 |

### 4.3 FreeRTOS 任务架构

| 优先级 | 任务名 | 周期 | 栈(word) | 职责 |
|--------|--------|------|----------|------|
| 4 | Task_Balance | 10ms | 256 | MPU6050 读取 + 互补滤波 + 越界保护 + 直立环 PID |
| 3 | Task_Speed | 50ms | 256 | Encoder 读取 + 速度环/转向环串级 PID + 距离闭环 |
| 2 | Task_Cmd | 20ms | 128 | USART2 ESP32 命令解析与执行 |
| 2 | Task_Key | 20ms | 128 | 按键扫描，短按切换系统启停(En) |
| 1 | Task_Comm | 10ms | 256 | NRF24L01 遥控接收(6字节) + OLED 显示(100ms) + REC 触发 |

周期通过 `vTaskDelayUntil` 实现恒定周期。LED 由 Task_Balance 控制（ON=运行, OFF=停止）。

**并发策略**：不加锁 + 单写者原则——每个共享变量只允许一个任务写入，利用 Cortex-M3 32 位读写原子性避免竞争。

---

## 5. 控制算法

本项目采用 **串级 PID 控制结构**，包含三个 PID 控制器：

### 5.1 控制流程图

```
MPU6050 角度 ──────┬──▶ [直立环 PID] ──▶ （取反） ──▶ AvePWM ──▶ 电机混合 ──▶ PWM输出
                         ▲                           │
编码器测速 ──┬──▶ 平均速度 ──▶ [速度环 PID] ──────────┘
             │                                       
             └──▶ 左右差速 ──▶ [转向环 PID] ──▶ DifPWM ──▶ 电机混合
```

### 5.2 直立环（PID_Angle）

- **输入**：MPU6050 解算的当前车身角度 `Angle`
- **目标值**：0°（直立平衡位置）
- **默认参数**：Kp = 3.5, Ki = 0.3, Kd = 6.5
- **输出限幅**：±100
- **说明**：直立环输出取反后作为 `AvePWM`，即车身前倾 → 电机向前加速 → 使车身回到直立位置

### 5.3 速度环（PID_Speed）

- **输入**：左右轮编码器转速的均值 `AveSpeed = (RPM_L + RPM_R) / 2`
- **输出**：作为直立环的目标角度 `PID_Angle.Target`
- **默认参数**：Kp = 0.065, Ki = 0.003, Kd = 0
- **输出限幅**：±20
- **说明**：速度环是外环，输出修正直立环的目标角度，使小车在保持平衡的同时实现速度控制

### 5.4 转向环（PID_Turn）

- **输入**：左右轮编码器转速的差值 `DifSpeed = RPM_L - RPM_R`
- **输出**：差速 PWM `DifPWM`，参与电机混合
- **默认参数**：Kp = 0.05, Ki = 0, Kd = 0
- **输出限幅**：±50
- **说明**：转向环控制两轮差速实现转向

### 5.5 电机混合输出

```
PWM_L = AvePWM + DifPWM / 2    // 左电机
PWM_R = AvePWM - DifPWM / 2    // 右电机
```

输出限幅到 ±100 后，调用 `Motor_Direction()` 输出 PWM。

---

## 6. 编译与烧录

### 6.1 开发环境

| 工具 | 版本 |
|------|:----:|
| IDE | μVision V5.24.2.0 |
| C 编译器 | Armcc.exe V5.06 update 5 (build 528) |
| 芯片 | STM32F103C8 (中等容量, 72MHz) |

### 6.2 编译步骤

1. 使用 Keil MDK 打开工程文件 `project.uvprojx`
2. 在工具栏中选择 Target 为 `Target 1`
3. 点击 **Build (F7)** 编译工程
4. 编译完成后在 `Objects/` 目录下生成 `project.axf` 文件

### 6.3 烧录步骤

- 使用 **ST-Link / J-Link / DAP-Link** 等调试烧录器连接至开发板的 SWD 接口（PA13 SWDIO、PA14 SWCLK）
- 在 Keil 中点击 **Download (F8)** 将程序烧录至芯片
- 烧录成功后，复位开发板即可运行

---

## 7. 使用方法

### 7.1 开机流程

1. 安装两节 18650 锂电池，拨动电源开关上电
2. 系统自动完成各模块初始化（OLED、MPU6050、编码器、电机、蓝牙、NRF24L01 等）
3. OLED 显示屏亮起，显示当前角度和速度数据
4. **将小车车身放置水平，按下 Key1 启动自平衡**

### 7.2 按键功能

| 按键 | GPIO | 功能说明 |
|:---:|:----:|---------|
| Key1 | PB1 | **短按**：切换系统启停状态（自平衡开启/关闭） |
| Key2 | PB0 | **短按**：目标值 +1　/　**长按**：目标值 -1 |
| Key3 | PA5 | 预留（可自定义） |
| Key4 | PA4 | 预留（可自定义） |

### 7.3 OLED 显示界面

OLED 显示屏实时显示以下数据：

- **第1行**：当前车身角度（`Angle`，浮点数）
- **第2行**：当前平均速度（`AveSpeed`，浮点数）

> 可通过修改 `main.c` 中的 `OLED_Show*` 函数自定义显示内容。

### 7.4 ESP32 AI 语音控制

ESP32-S3 通过 **USART2（115200 波特率）** 下发控制指令：

| 命令格式 | 说明 | 示例 |
|---------|------|------|
| `forward:N\n` | 前进 N 厘米 | `forward:100\n` → 前进 1 米 |
| `back:N\n` | 后退 N 厘米 | `back:50\n` → 后退 50 厘米 |
| `left:N\n` | 左转 N 度 | `left:45\n` → 左转 45° |
| `right:N\n` | 右转 N 度 | `right:90\n` → 右转 90° |
| `stop\n` | 紧急停止 | `stop\n` |
| `light:on\n` | 开灯（板载 LED PC13） | `light:on\n` |
| `light:off\n` | 关灯 | `light:off\n` |
| `balance:on\n` | 开启自平衡 | `balance:on\n` |
| `balance:off\n` | 关闭自平衡 | `balance:off\n` |

多条指令用 `;` 分隔：`forward:100;light:on\n`

STM32 执行完成后回复 `OK\n`，失败回复 `ERR:原因\n`。

**录音触发**：遥控器按下语音按键 → STM32 收到 NRF 数据包的第 6 字节按键标志 → USART2 发送 `REC\n` → ESP32 开始录音（最长 10 秒，静音 1.5 秒自动截止）。

### 7.5 ESP32 数据流

```
用户语音 → INMP441 → I2S → ESP32 录音 → Base64 WAV
  → POST mimo-v2.5 (音频理解)
  → 返回 JSON: {"command":"forward:100", "speech":"好的，正在前进1米"}
    ├─ command → UART TX → STM32 立即执行
    └─ speech → POST mimo-v2.5-tts (流式 PCM16, 24kHz, 音色:冰糖)
                → 边收边解码 → I2S → MAX98357A → 扬声器
```

### 7.6 NRF24L01 无线遥控

- 小车作为 NRF24L01 接收端，接收遥控器发送的数据包
- 数据包格式：`[0x00, LH, LV, RH, RV, BTN]`（6 字节）
  - `LH`：左摇杆水平方向 → 控制目标速度（`PID_Speed.Target = LH * 2`）
  - `RV`：右摇杆垂直方向 → 控制目标转向（`PID_Turn.Target = RV * 3`）
  - `BTN`：按键标志（bit0 = 语音录音按键）

---

## 8. PID 参数整定指南

### 8.1 默认参数

| 控制器 | Kp | Ki | Kd | 输出限幅 |
|:-----:|:--:|:--:|:--:|:--------:|
| 直立环 | 3.5 | 0.3 | 6.5 | ±100 |
| 速度环 | 0.065 | 0.003 | 0 | ±20 |
| 转向环 | 0.05 | 0 | 0 | ±50 |

### 8.2 调参建议

1. **先调直立环**：将速度环和转向环的输出限幅设为 0，只保留直立环。调整 Kp 使车身能快速回正但不剧烈振荡，再调 Kd 抑制振荡，最后加少量 Ki 消除静差。
2. **再加速度环**：从很小的 Kp（如 0.01）开始逐步增大，使小车能平稳前进/后退而不抖动。速度环 Ki 通常取 Kp 的 1/10 ~ 1/20。
3. **最后调转向环**：在不移动时调试转向，Kp 从 0.01 开始，使差速控制平滑即可，转向环通常不需要积分项。
4. **注意**：不同机械结构（重心高度、轮径、电机减速比等）的参数可能差异较大，以上默认参数仅供参考。

---

## 9. 常见问题

**Q：小车无法保持平衡，一直往一个方向加速？**
> 检查 MPU6050 安装方向是否正确，以及 `Angle` 角度的正负方向是否与电机运动方向匹配。可尝试给 `AvePWM` 取反（在 `PID.c` 中已实现）。

**Q：电机不转或单侧不转？**
> 检查 TB6612 接线（AIN1/AIN2/BIN1/BIN2）、PWM 信号是否正常，以及电池电压是否充足。

**Q：ESP32 无法通信？**
> 检查 USART2 交叉接线（STM32 PA2→ESP32 RX, STM32 PA3→ESP32 TX），波特率均为 115200。确保共地。

**Q：NRF24L01 通信失败？**
> 检查 SPI 接线（CE、CSN、SCK、MOSI、MISO），确保收发双方的地址和通道配置一致。

---

## 10. 项目介绍与演示

项目演示视频（Bilibili）：[点击观看](https://www.bilibili.com/video/BV1VKGo6UEJ6/?spm_id_from=333.1365.list.card_archive.click&vd_source=27961dae681973fda6881af042d19fe7)

---

## 11. 参考与致谢

- [STM32F10x 标准外设库](https://www.st.com/en/embedded-software/stsw-stm32054.html) — STMicroelectronics
- 江协科技 — OLED、NRF24L01 等驱动代码参考框架
- [GitHub 仓库](https://github.com/2268699403/Stm32_Project) — 项目源码与更新

---

© 2026 STM32 两轮自平衡小车项目