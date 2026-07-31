# FEAT-058 — 通用人形 Enemy 巡逻感知与丢失目标搜索

**状态：** done
**完成：** 2026-07-30

公共流程：路点巡逻 → 到点随机 Relaxed 环视 → 感知目标进入 Aim → 丢失后冲刺到 LastKnownLocation → 随机 Relaxed 环视 → 恢复最近巡逻点。不得硬编码 Phantom。

## 实现与验证

- `StartLostTargetSearch` 实现 `Aim → SearchRush → SearchScan → Patrol`；速度、到达半径和扫描时间可配置。
- 感知丢失写 LastKnown、清目标/Focus；成功后恢复最近巡逻点，无 Nav/Move 失败安全回 Patrol。
- 巡逻点与搜索点复用 Relaxed Fgt v1～v4，连续两次随机不重复。
- Development Editor 构建、Runtime 自动化及真实 PIE 通过；TestMap 无 RecastNavMesh，PIE 验证无导航时安全回巡逻。
