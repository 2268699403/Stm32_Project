# FreeRTOS 移植任务计划 —— 平衡车_裸机 → 平衡车_FreeRTOS

> 创建日期：2026-07-04
> 工程路径：`基于stm32f103的两轮自平衡车/平衡车_FreeRTOS/`
> 参考源：`基于stm32f103的两轮自平衡车/平衡车_裸机/`（完整串级 PID 控制）
> 学习目标：在自主移植过程中掌握 FreeRTOS 任务/调度/并发/中断对接原理

---

## 一、决策记录（已与 AI 讨论定稿）

| # | 决策项 | 定稿 | 原理依据 |
|---|--------|------|----------|
| 1 | 任务架构 | 按控制环一一对应，5 个任务 | RTOS 分治：每个控制环独立任务，靠抢占式调度实现准实时，易维护 |
| 2 | Timer.c 处置 | 删除，改用 `vTaskDelayUntil` 实现周期 | SysTick 已是 RTOS 节拍源，TIM1 软件分频冗余；`vTaskDelayUntil` 周期恒定不漂移 |
| 3 | OLED/Delay 范围 | 暂不迁移 OLED，先跑通控制 | 聚焦核心；裸机 `Delay.h` 走 SysTick 会与 RTOS 冲突，弃用，延时一律 `vTaskDelay` |
| 4 | 并发同步 | 不加锁 + 单写者原则 | Cortex-M3 32 位读写原子；每个变量只一个写者即无竞争；零开销、不占 RAM |
| 5 | 内存分配方式 | 全动态（关闭 configSUPPORT_STATIC_ALLOCATION）| 简化代码，删除 27 行静态分配样板；空闲/定时器任务由内核自动从 heap_4 创建，学习阶段无须管内存布局 |

---

## 二、任务架构定稿

```
优先级  任务名           周期      栈(word)  职责                                   写者归属
─────────────────────────────────────────────────────────────────────────────────
  4    Task_Balance     10ms       256      MPU6050读+互补滤波+越界保护+直立环PID  Angle/En(写)/PWM_L/R/AvePWM/PID_Angle
  3    Task_Speed       50ms       256      Encoder_GetState+速度/转向环PID        RPM_L/R/PID_Speed/PID_Turn/DifPWM/PID_Angle.Target
  2    Task_Key         20ms       128      Key_Scan+Key_Mode                      En(写，与Balance共写→见注)
  1    Task_Comm        阻塞型     256      NRF24L01_Receive+蓝牙ParseParam预留     PID_Speed.Target/PID_Turn.Target
  1    Task_LED         500ms      64       运行指示                                 —
─────────────────────────────────────────────────────────────────────────────────
```

**注（En 双写处理）**：`En` 由按键任务"切换"、由直立环"越界强制清零"。两者都只做单字节原子赋值，不存在 RMW，安全。但需约定：越界保护时直立环清自己的 PID，速度环下一周期看到 `En==0` 自清 PID，避免跨任务清他人结构体。

**内存核算**：5 用户任务栈 + TCB ≈ 4.15KB；空闲任务(128w+TCB) + 定时器任务(256w+TCB) ≈ 1.7KB 也从 heap_4 分配；合计约 5.9KB / 8KB heap，剩余 ~2.1KB 给后续队列/通知。✅

---

## 三、移植阶段

### 阶段 0：环境与冲突清障（前置，必须先做）

| 步骤 | 内容 | 验证检查点 | 状态 |
|------|------|-----------|------|
| 0.1 | 从 Keil 工程移除 `Hardware/Timer.c`，并从磁盘删除 Timer.c/h | 工程能编译过，Key.c 无 Timer.h 依赖 | ✅ |
| 0.2 | 确认 `BSP_Init()` 设 `NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4)`，且全局无其他 `PriorityGroupConfig` 调用 | 全工程搜索 `NVIC_PriorityGroupConfig` 仅 1 处且为 Group 4 | ✅ |
| 0.3 | 排查 `Delay.h` 依赖：裸机 `Delay.h`(SysTick) 不引入；裸机代码若调用 `Delay_ms` 一律改 `vTaskDelay` | 全工程仅 MPU6050.c:204 已注释的 `//Delay_ms(10)` | ✅ |
| 0.4 | 确认 `stm32f10x_it.c` 中 SVC/PendSV/SysTick 仍注释（当前已注释✅） | 编译无重复定义 | ✅ |
| 0.5 | 禁用静态分配：configSUPPORT_STATIC_ALLOCATION→0，删除 main.c 中 27 行静态分配样板 | 编译通过，无 vApplicationGetIdleTaskMemory 等调用 | ✅ |

> **原理提示**：FreeRTOS port.c 通过 `FreeRTOSConfig.h` 末尾的 `#define xPortPendSVHandler PendSV_Handler` 宏把 port.c 的 PendSV 实现接到启动文件的中断向量。若 `stm32f10x_it.c` 里也定义 `PendSV_Handler`，链接报"重复定义"。NVIC 必须全抢占分组(group 4)，否则 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 屏蔽逻辑失效，`FromISR` API 可能死机。

---

### 阶段 1：BSP 初始化迁移

| 步骤 | 内容 | 验证检查点 | 状态 |
|------|------|-----------|------|
| 1.1 | 充实 `main.c` 的 `BSP_Init()`：移植裸机 `main()` 的全部 `*_Init()`——MPU6050_Init/Config、Encoder_Init、Motor_Init、Key_Init、LED_Init、PWM_Init、NRF24L01_Init、OLED_Init、PID_Init×3 | 编译通过，外设初始化正常 | ✅ |
| 1.2 | `PID_Init(&PID_Angle/Speed/Turn)` 迁入 BSP_Init（裸机在 main 里调，结构体成员 RAM 上电随机必须显式初始化） | 调试器看 PID 结构体成员全 0 | ✅ |
| 1.3 | 全部初始化**在 `vTaskStartScheduler()` 之前完成** | 调度器启动后无任何阻塞式硬件初始化 | ✅ |

> **原理提示**：调度器启动前是"超级循环"环境，可任意阻塞初始化（I2C 轮询等）；启动后所有任务并发，若在任务里做全局硬件初始化会与其他任务竞争外设。故硬件 init 集中放 BSP_Init，调度器启动后任务只做"周期控制"。

---

### 阶段 2：任务骨架搭建

| 步骤 | 内容 | 验证检查点 | 状态 |
|------|------|-----------|------|
| 2.1 | 在 `main.c` 写 5 个任务函数空壳（while(1)+vTaskDelayUntil） | 编译通过 | ✅ |
| 2.2 | `main()` 中依次 `xTaskCreate` 创建 5 任务，传入优先级与栈大小 | 任务创建返回 pdPASS | ✅ |
| 2.3 | 末尾 `vTaskStartScheduler()`，其后加 `while(1);` 兜底 | 下载后 LED 闪烁/常亮，证明调度器跑通 | ✅ |

> **原理提示**：`xTaskCreate` 动态分配栈+TCB（来自 heap_4）。若返回 errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY 说明 heap 不够，调栈大小或 heap。`vTaskStartScheduler` 后 main 的栈被弃用，CPU 永不返回 main——故其后的 `while(1)` 只是编译器要求，正常不会执行。

---

### 阶段 3：控制环迁移（核心）

| 步骤 | 内容 | 验证检查点 | 状态 |
|------|------|-----------|------|
| 3.1 | Task_Balance：移植 `MPU6050_GetRawData` + 互补滤波算 `Angle`（互补滤波在 MPU6050_GetRawData 内部） | OLED 显示 Angle 随车身倾倒变化正确 | ✅ |
| 3.2 | Task_Balance：移植越界保护——`Angle≤-50||≥50` 时 `En=0`+清 PID_Angle+Motor_Stop | 手动倾倒>50° 电机停 | ✅ |
| 3.3 | Task_Balance：`if(En) PID_Angle_Update();` 移植直立环（含电机混合、PWM 限幅） | LED 常亮、OLED 显示 AO 值变化 | ✅ |
| 3.4 | Task_Speed：移植 `Encoder_GetState` + `if(En) PID_Move_Update();`（写 DifPWM、PID_Angle.Target） | 编码器 RPM 正确，OLED 显示 ST/SO | ✅ |
| 3.5 | Task_Speed：`else` 分支清 PID_Speed/Turn（自清，不依赖直立环） | En=0 时 SO/AO 清零，无 PWM 冲击 | ✅ |

> **原理提示**：互补滤波 `Angle = 0.01*AngleAcc + 0.99*AngleGyro` 必须在 10ms 周期稳定执行，否则积分系数失准。这就是用 `vTaskDelayUntil` 而非 `vTaskDelay` 的原因——周期必须恒定。串级：速度环输出→PID_Angle.Target，是"单写者"（速度环写、直立环读），float 原子，无需锁。

---

### 阶段 4：人机交互迁移

| 步骤 | 内容 | 验证检查点 | 状态 |
|------|------|-----------|------|
| 4.1 | Task_Key：移植 `Key_Scan()+Key_Mode()`，短按切换 `En` | 按键能启停（LED 亮/灭）| ✅ |
| 4.2 | Task_Comm：移植 `NRF24L01_Receive()`，每 10ms 轮询，成功则解析摇杆设目标值 | 遥控器能控速/转向 | ✅ |
| 4.3 | Task_Comm：预留蓝牙 `USART2_ParseParam`（先注释，后续接通） | 编译通过 | ✅ |

> **原理提示**：Task_Comm 用阻塞型——`NRF24L01_Receive` 内部轮询 SPI，自然阻塞，正好让出 CPU；周期用 `vTaskDelay(10)` 控制轮询频率即可，不必 `DelayUntil`（通信无恒周期要求）。Target 由 Comm 写、Speed 读，单写者原子，无锁。

---

### 阶段 5：并发安全与调优

| 步骤 | 内容 | 验证检查点 | 状态 |
|------|------|-----------|------|
| 5.1 | 复核单写者原则：逐变量确认写者唯一 | 已验证 En(Task_Key/Balance)/Angle(Balance)/RPM(Speed)/PID 结构体各属单一任务 | ✅ |
| 5.2 | 栈水位检查：`uxTaskGetStackHighWaterMark(NULL)` 各任务运行后打印剩余 | 各任务剩余 ≥ 30% 栈 | ⬜ |
| 5.3 | 现场联调：PID 参数复用裸机默认值，验证能立车 | 车能自平衡≥30s | ⬜ |
| 5.4 | （可选）后续接蓝牙调参时，对 PID_Angle.Kp/Ki/Kd 加临界区或 Mutex | 调参时不抖动 | ⬜ |

---

## 四、已识别的关键冲突点（详见 findings.md）

1. **NVIC 优先级分组冲突**：`Timer.c:43` Group 2 vs RTOS 要求 Group 4 → 阶段 0.2 解决 ✅
2. **SysTick 占用冲突**：裸机 `Delay.h` 走 SysTick 会破坏 RTOS 节拍 → 阶段 0.3 弃用 ✅
3. **TIM1 中断式调度冗余**：与 RTOS 任务调度设计冲突 → 阶段 0.1 删除 Timer.c/h ✅
4. **PID 状态清零跨任务一致性**：禁用时多字段清零若分散多任务会撕裂 → 阶段 3 单写者原则解决 ✅

---

## 五、遇到错误（积累记录）

| 错误现象 | 尝试次数 | 解决方案 | 状态 |
|---------|---------|---------|------|
| （移植过程中填）| | | |

---

## 六、阶段进度总览

```
阶段0 [✅✅✅✅✅] 清障         阶段3 [✅✅✅✅✅] 控制环
阶段1 [✅✅✅]     BSP初始化    阶段4 [✅✅✅]     人机交互
阶段2 [✅✅✅]    任务骨架      阶段5 [✅⬜⬜⬜]   调优
```

图例：⬜ 待办 / 🟨 进行中 / ✅ 完成
