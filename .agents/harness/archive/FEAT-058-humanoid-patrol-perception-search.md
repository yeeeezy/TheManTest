# FEAT-058 — 通用人形 Enemy 巡逻感知与丢失目标搜索

**状态：** planned

公共流程：路点巡逻 → 到点随机 Relaxed 环视 → 感知目标进入 Aim → 丢失后冲刺到 LastKnownLocation → 随机 Relaxed 环视 → 恢复最近巡逻点。不得硬编码 Phantom。
