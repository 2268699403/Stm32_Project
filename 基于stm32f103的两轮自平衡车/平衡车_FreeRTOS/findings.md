# Findings —— FreeRTOS 移植冲突点与设计发现

> 创建日期：2026-07-04
> 关联计划：`task_plan.md`

---

## 1. NVIC 优先级分组冲突

**发现时间**：阶段 0 分析
**文件**：`Hardware/Timer.c:43`
**现象**：裸机 `Timer_Init()` 设置 `NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2)`（2 位抢占 + 2 位子优先级）。
**冲突**：FreeRTOS `FreeRTOSConfig.h` 和 `main.c` 的 `BSP_Init()` 使用 `NVIC_PriorityGroup_4`（4 位抢占，0 位子优先级）。若先调 `Timer_Init()` 覆盖了 group，再启动调度器，`configMAX_SYSCALL_INTERRUPT_PRIORITY` 的屏蔽逻辑会失效。
**原理**：FreeRTOS port.c 调用 `portNVIC_SET_PRIORITY_MASK()` 时写的是 `configMAX_SYSCALL_INTERRUPT_PRIORITY`(5 << 4 = 0x50)。在 Group 4 下，优先级[7:4]是抢占位，[3:0]无意义，0x50 正确屏蔽优先级 >5 的中断。若在 Group 2 下，优先级[7:6]是抢占位，[5:4]是子优先级位，0x50 会被解释为抢占 1、子优先级 1，屏蔽逻辑不可预测。
**解决**：阶段 0.2——删除 `Timer.c`（已不编译），全局确认 `NVIC_PriorityGroupConfig` 只调一次且为 Group 4。

---

## 2. SysTick 占用冲突（Delay.h）

**发现时间**：阶段 0 分析
**文件**：裸机 `User/Delay.h`（可能基于 SysTick 阻塞延时）
**冲突**：FreeRTOS port.c 占用 SysTick 作系统节拍（`xPortSysTickHandler`）。若 Delay.h 也试图用 SysTick 寄存器做阻塞延时，两者会互相干扰。
**原理**：SysTick 是 Cortex-M3 唯一系统节拍定时器。FreeRTOS 接管后自动重装 `configCPU_CLOCK_HZ / configTICK_RATE_HZ`。裸机 `Delay_ms(10)` 通常循环查询 SysTick->VAL 或 COUNTFLAG——这会读到 FreeRTOS 正在维护的计数值，产生不可预测的延迟。
**解决**：阶段 0.3——不在 FreeRTOS 任务中引入 `Delay.h`。任务内延时一律使用 `vTaskDelay()` 或 `vTaskDelayUntil()`，后者会自动挂起任务让出 CPU。

---

## 3. TIM1 中断式调度冗余

**发现时间**：阶段 0 分析
**文件**：`Hardware/Timer.c`（TIM1_UP_IRQHandler）
**现象**：裸机用 TIM1 1ms 硬件中断 + 3 个静态计数器软件分频出 10ms/20ms/50ms 周期，在 ISR 内直接调用 MPU6050 I2C 读取、PID 运算、电机控制等全部逻辑。
**问题**：
- ISR 内跑 I2C（阻塞）本身不规范——I2C 等待应答时会空转等待
- PID 运算在 ISR 中增加了中断延迟不确定性
- 与 FreeRTOS 任务调度架构冲突——RTOS 期望控制逻辑在任务中执行，而非 ISR
**解决**：阶段 0.1——从 Keil 工程移除 `Timer.c`（或删除 TIM1_UP_IRQHandler），改用 `vTaskDelayUntil()` 实现每个任务独立周期。

---

## 4. PID 状态清零跨任务一致性

**发现时间**：并发策略分析（阶段 3-5）
**文件**：裸机 `Timer.c:86-115`
**现象**：角度超 ±50° 或 `En==0` 时，裸机在 ISR 中连续清零 3 个 PID 结构体的 `ErrorInt` 和 `Target`（6 个 float 赋值）。若这些清零分散到不同任务执行（如直立环清 `PID_Angle`、速度环清 `PID_Speed`），中间可能被抢占导致速度环读到半清的 `PID_Speed` 结构体，计算出异常输出冲击电机。
**解决**：阶段 3.2/3.5——越界保护时直立环任务只清零自己的 `PID_Angle`，然后置 `En=0`；速度环任务下一周期检测到 `En==0` 后自行清零 `PID_Speed` 和 `PID_Turn`。每个 PID 结构体仅由一个任务写，消除竞争。

---

## 5. 并发策略决策

**架构**：不加锁 + 单写者原则
**依据**：逐变量分析裸机所有跨任务共享变量（`Angle`/`RPM_L/R`/`En`/`AvePWM`/`DifPWM`/`PWM_L/R`/各 PID 结构体），发现通过合理的写者归属可保证每变量最多一个写者。Cortex-M3 32 位 `float`/`int` 单次读写天然原子，无需锁。
**例外**：未来若引入蓝牙调参（USART2 中断 + 任务写 PID 参数），需评估给 PID 参数加 `taskENTER_CRITICAL` 或用 Mutex。

---

## 6. 互补滤波在 MPU6050_GetRawData 内部（阶段 3 发现）

**发现时间**：阶段 3 分析
**关键事实**：`MPU6050.c:259-279` 在 `MPU6050_GetRawData()` 函数内部自动完成：
- `AngleAcc = atan2(AX,AZ) * 180/π - 1.5`（加速度计角度校准）
- `AngleGyro = Angle + (-Gyro_Y/32768) * 2000 * 0.01`（陀螺仪积分）
- `Angle = 0.01*AngleAcc + 0.99*AngleGyro`（互补滤波）

任务 `Task_Balance` 只需调用 `MPU6050_GetRawData(&Data)`，`Angle` 自动更新。

## 7. 内存预算（更新：2026-07-06，全动态分配 + OLED 显存）

| 项目 | 大小 |
|------|------|
| heap_4 总堆 | 8KB |
| 5 个用户任务栈 | 256+256+128+256+64 = 960 words = 3.75KB |
| 5 个任务 TCB | ~5 × 80B ≈ 0.4KB |
| 空闲任务（内核自动创建）| 128 words × 4 + TCB ≈ 0.6KB |
| 定时器任务（内核自动创建）| 256 words × 4 + TCB ≈ 1.1KB |
| 用户任务合计 | ~4.15KB |
| 内核任务合计 | ~1.7KB |
| 剩余 heap | ~2.1KB（留给后续队列/通知）|

结论：全动态分配后，heap 消耗约 5.9KB / 8KB，剩余约 2.1KB。OLED 显存 `OLED_DisplayBuf[8][128]`（1KB）作为全局数组占 ZI-data 段（BSS），不额外消耗 heap。后续若加消息队列需重新核算。

---

## 8. 静态 vs 动态分配决策

**决策时间**：2026-07-04，主要反复讨论后定稿
**最终选择**：全动态（`configSUPPORT_STATIC_ALLOCATION = 0`）

### 两种方式对比

| 对比项 | 全静态（原设计） | 全动态（定稿） |
|--------|----------------|---------------|
| 样板代码 | 27 行回调 + 4 个静态数组/TCB | 0 行 |
| 空闲/定时器任务内存 | 静态 BSS | heap_4 自动分配 |
| 碎片风险 | 无 | 极低（任务不删除）|
| 适合场景 | 安全认证/硬实时 | 学习/一般嵌入式 ✅ |

### 用户任务 API 选择

动态模式下用户任务使用 `xTaskCreate`，只需"写任务函数 + 一行调用"两步完成。静态模式下每多一个任务就要额外声明栈数组和 TCB 结构体，对学习阶段是噪音。

### 后续可切换

`FreeRTOSConfig.h` 中 `configSUPPORT_STATIC_ALLOCATION` 随时可改回 1。若后期发现 heap_4 碎片问题（当前场景不会），可切换回静态分配——main.c 的结构不变，只需加回调函数和静态数组。

---

## 9. 空闲任务/定时器任务的自动创建机制

**发现时间**：2026-07-04，动态改造时从源码理解

### 核心原理

很多初学者误以为"动态模式下没有空闲任务"或"空闲任务需要手动创建"。实际上**无论静态还是动态模式，空闲任务和定时器任务都会被创建**，区别只在创建方式：

- **静态模式**（`configSUPPORT_STATIC_ALLOCATION = 1`）：`vTaskStartScheduler()` 调用 `vApplicationGetIdleTaskMemory()` 回调问你要内存，然后用 `xTaskCreateStatic()` 创建
- **动态模式**（`configSUPPORT_STATIC_ALLOCATION = 0`）：内核直接调 `xTaskCreate(prvIdleTask, ...)`，跟用户调 `xTaskCreate` 创建自己的任务**用的是同一个 API**，只是调用者不同

### 源码依据

`FreeRTOS/src/tasks.c:1826-1866`：
```c
void vTaskStartScheduler( void )
{
    #if( configSUPPORT_STATIC_ALLOCATION == 1 )
        // 静态：通过回调问你要内存
        vApplicationGetIdleTaskMemory( ... );
        xIdleTaskHandle = xTaskCreateStatic( prvIdleTask, ... );
    #else
        // 动态：自己从 heap 分配
        xReturn = xTaskCreate( prvIdleTask, "IDLE", configMINIMAL_STACK_SIZE, ... );
    #endif

    #if ( configUSE_TIMERS == 1 )
        xReturn = xTimerCreateTimerTask();  // 定时器任务同理
    #endif
}
```

### 创建流程

```
用户程序
  │
  ├─ BSP_Init()               // 硬件初始化
  ├─ xTaskCreate(任务A, ...)   // 用户手动创建 5 个任务
  ├─ xTaskCreate(任务B, ...)
  │
  └─ vTaskStartScheduler()    // 启动调度器
         │
         ├─ xTaskCreate(prvIdleTask, "IDLE", ...)     ← 内核自动创建空闲任务
         │     └─ pvPortMalloc → 栈 + TCB 从 heap_4 分配
         │
         ├─ xTimerCreateTimerTask()                   ← 内核自动创建定时器任务
         │     └─ xTaskCreate(prvTimerTask, "Tmr Svc", ...)
         │
         ├─ 配置 SysTick（configTICK_RATE_HZ）
         ├─ 配置 PendSV（最低优先级异常）
         └─ 首次任务切换（永不返回）
```

所以用户只需要关注自己的业务任务，内核底座（空闲 + 定时器）由调度器自己管理。

---

## 10. LED 控制权由 Task_Balance 独占（阶段 3）

**决策**：删除独立 LED 闪烁任务，LED 由 Task_Balance 按 En 状态控制（ON=运行, OFF=停止）。原因是 LED 任务与 Task_Balance 同时操控 LED 会产生竞态。改为 Task_Balance 独占后，LED 状态直接反映系统运行状态，与裸机行为一致。

## 11. En 按键切换由 Task_Key 实现（阶段 4）

**实现**：Task_Key 每 20ms 调用 `Key_Scan()` + `Key_Mode()`，检测到短按（`Key1_Mode==1`）时切换 `En = !En` 并清零标志。En 双写者（Task_Key 切换、Task_Balance 越界清零）均为单字节原子赋值，无 RMW 竞争。

## 12. NRF24L01 遥控与 OLED 显示共存于 Task_Comm（阶段 4）

**实现**：Task_Comm 以 10ms 为基准周期，每周期调用 `NRF24L01_Receive()` 轮询遥控数据（非阻塞），每 10 个周期（100ms）刷新一次 OLED。OLED 刷新用 `continue` 跳过帧间无意义的清屏操作，节省 CPU 时间。
