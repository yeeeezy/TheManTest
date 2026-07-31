# FEAT-058 — 通用人形 Enemy 巡逻感知与丢失目标搜索

**状态：** done
**完成：** 2026-07-30

公共流程：路点巡逻 → 到点随机 Relaxed 环视 → 感知目标进入 Aim → 丢失后冲刺到 LastKnownLocation → 随机 Relaxed 环视 → 恢复最近巡逻点。不得硬编码 Phantom。

## 实现与验证

- `StartLostTargetSearch` 实现 `Aim → SearchRush → SearchScan → Patrol`；速度、到达半径和扫描时间可配置。
- 感知丢失写 LastKnown、清目标/Focus；成功后恢复最近巡逻点，无 Nav/Move 失败安全回 Patrol。
- 巡逻点与搜索点复用 Relaxed Fgt v1～v4，连续两次随机不重复。
- Development Editor 构建、Runtime 自动化及真实 PIE 通过；TestMap 无 RecastNavMesh，PIE 验证无导航时安全回巡逻。

## Session126 完善

- 完成审计发现“无导航时安全留在 Patrol”仍不能满足拖入 TestMap 后实际巡逻的要求，因此为 Patrol 与 SearchRush 补充 CharacterMovement 直移回退；有 NavMesh 时仍优先使用寻路。
- 路径请求立即失败或异步失败都会切换回退；到达后复用原有 Patrol 等待/随机 Relaxed 环视/下一路点与 SearchScan/恢复最近巡逻点流程。
- 新增 `SetPatrolPoints`，支持关卡实例和运行时生成敌人复用同一公共巡逻逻辑。
- `PIENoNavPatrol`：位移 94.3cm、Walking、Patrol、scanning=true；`PIENoNavSearch`：位移 189.6cm、SearchScan、scanning=true。最终 Phantom 前缀 7/7 Success。
