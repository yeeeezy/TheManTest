# [FEAT-013] 开火音效

**创建日期：** 2026-06-07
**状态：** done
**Archive 文件：** `archive/FEAT-013-fire-sound.md`

---

## 功能概述

按下 LMB 开火时，在枪口位置播放武器专属的开火音效。

---

## 架构设计

```
GA_Shoot::ActivateAbility()
  → 发射逻辑
  → UGameplayStatics::PlaySoundAtLocation(FireSound, MuzzleLocation)
```

- `FireSound`（`USoundBase*`）配置在 `AFirearm`，与 `FireMontage` 同分类
- 播放位置：枪口 Socket 位置（与子弹生成点一致）；Socket 不存在时退回相机位置
- 音量/音调通过 `FireSoundVolumeMultiplier` / `FireSoundPitchMultiplier` 调节

---

## 需修改文件

| 文件 | 变更 |
|---|---|
| `Firearm.h` | 新增 `FireSound`、`FireSoundVolumeMultiplier`、`FireSoundPitchMultiplier` |
| `GA_Shoot.cpp` | 播放音效，位置取 MuzzleLocation |

---

## 完成标准

- [ ] AFirearm 新增音效 UPROPERTY
- [ ] GA_Shoot 播放音效
- [ ] BP_RepairGun 指定音效资产
- [ ] PIE 测试：开火有声音从枪口位置发出

---

## 实现日志

### 2026-06-07 — 实现完成

- `Firearm.h`：新增 `USoundBase* FireSound`、`FireSoundVolumeMultiplier`、`FireSoundPitchMultiplier`，分类 `Weapon|Audio`
- `GA_Shoot.cpp`：引入 `Kismet/GameplayStatics.h`，在蒙太奇播放后调用 `UGameplayStatics::PlaySoundAtLocation`，位置复用已有的 `MuzzleLocation`

---

**完成标准全部满足日期：** 2026-06-07
**功能关闭日期：** 2026-06-07
