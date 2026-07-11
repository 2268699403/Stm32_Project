# Progress —— FreeRTOS 移植会话日志

> 起始日期：2026-07-04

---

## 会话 1：规划与决策（2026-07-04）

### 已完成
- 读取裸机版和 FreeRTOS 版两个工程的 `main.c`、`Timer.c`、`PID.c/h`、`FreeRTOSConfig.h`、`stm32f10x_it.c`
- 分析两个工程的目录差异和项目文件结构（uvprojx）
- 创建 `task_plan.md`（5 阶段移植计划）
- 更新 `AGENTS.md`（补充任务架构、并发策略、决策记录）
- 创建 `findings.md`（4 个冲突点 + 并发策略依据 + 内存预算）
- 创建 `progress.md`（本日志）

### 决策总结

| 决策 | 结论 |
|------|------|
| 任务划分 | 5 个独立任务（直立环/速度环/按键/通信/LED）|
| Timer.c 处理 | 从工程移除，用 `vTaskDelayUntil` |
| OLED/Delay | 暂不迁移 |
| 并发策略 | 不加锁 + 单写者原则 |

### 待执行
用户按 `task_plan.md` 阶段 0~5 自行移植，AI 协助代码检查和答疑。

---

## 会话 3：阶段 3 控制环迁移（2026-07-06）

### 已完成
- **Task_Balance**（10ms）：`MPU6050_GetRawData` + 互补滤波 + 越界保护(±50°) + `PID_Angle_Update`
- **Task_Speed**（50ms）：`Encoder_GetState` + `PID_Move_Update` + En=0 自清 PID
- 删除 `led` 任务——LED 由 Task_Balance 控制（ON=运行, OFF=停止）
- 删除 `Timer.c`/`Timer.h` 从磁盘；修复 `Key.c` 的 `#include "Timer.h"` 依赖
- 编译通过，OLED 显示 Angle/En/ST/SO/AO，控制功能正常

### 发现
- 互补滤波在 `MPU6050_GetRawData()` 函数内部，`Task_Balance` 只需调用该函数即可
- `Encoder_GetState()` 无中断（纯轮询 TIM3/TIM4 计数器），RPM 公式的 1200 常数对应 50ms 采样周期

---

## 会话 4：阶段 4 人机交互迁移（2026-07-06）

### 已完成
- **Task_Key**（20ms）：`Key_Scan` + `Key_Mode` → 短按切换 `En=!En`
- **Task_Comm**（10ms 基准）：`NRF24L01_Receive` 每 10ms 轮询 + OLED 每 100ms 刷新
- 下载验证：按键启停正常、OLED 正常、遥控正常、控制功能正常

### 当前进度
- 阶段 0-4 全部完成，所有功能已验证
- 剩余：阶段 5 栈水位检查 + 现场联调 + 蓝牙调参预留
