# init.ps1 — TheManTest Harness 启动验证脚本
# 在每次会话开始时运行，确认环境和 harness 状态健康

$HarnessDir  = $PSScriptRoot
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $HarnessDir)
$SourceDir   = Join-Path $ProjectRoot "Source\TheManTest"
$UProject    = Join-Path $ProjectRoot "TheManTest.uproject"
$FeatureList = Join-Path $HarnessDir "feature_list.json"
$ProgressFile = Join-Path $HarnessDir "progress.md"

$errorCount = 0
$warnCount  = 0

function Ok   { param($msg) Write-Host "  [OK]   $msg" -ForegroundColor Green }
function Fail { param($msg) Write-Host "  [错误] $msg" -ForegroundColor Red;    $script:errorCount++ }
function Warn { param($msg) Write-Host "  [警告] $msg" -ForegroundColor Yellow; $script:warnCount++ }
function Info { param($msg) Write-Host "  [信息] $msg" -ForegroundColor Cyan }

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  TheManTest Harness 启动检查" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# ── 1. 项目根目录检查 ──────────────────────────
Write-Host ""
Write-Host "[ 1/5 ] 项目文件" -ForegroundColor White

if (Test-Path $UProject) {
    Ok ".uproject 文件存在：$UProject"
} else {
    Fail ".uproject 文件不存在，路径：$UProject"
}

# ── 2. Source 目录检查 ─────────────────────────
Write-Host ""
Write-Host "[ 2/5 ] 源码目录" -ForegroundColor White

if (Test-Path $SourceDir) {
    $cppFiles = @(Get-ChildItem -Path $SourceDir -Recurse -Filter "*.cpp" -ErrorAction SilentlyContinue)
    $hFiles   = @(Get-ChildItem -Path $SourceDir -Recurse -Filter "*.h"   -ErrorAction SilentlyContinue)
    Ok "Source\TheManTest 存在：$($cppFiles.Count) 个 .cpp，$($hFiles.Count) 个 .h"
} else {
    Fail "Source\TheManTest 目录不存在"
}

# ── 3. Harness 文件完整性 ──────────────────────
Write-Host ""
Write-Host "[ 3/5 ] Harness 文件" -ForegroundColor White

$requiredFiles = @(
    @{ Path = (Join-Path $HarnessDir "AGENTS.md");        Label = "AGENTS.md" },
    @{ Path = $FeatureList;                                Label = "feature_list.json" },
    @{ Path = $ProgressFile;                               Label = "progress.md" }
)

foreach ($f in $requiredFiles) {
    if (Test-Path $f.Path) { Ok "$($f.Label) 存在" }
    else                   { Fail "$($f.Label) 缺失" }
}

$archiveDir = Join-Path $HarnessDir "archive"
if (Test-Path $archiveDir) { Ok "archive/ 目录存在" }
else                        { Warn "archive/ 目录不存在，请手动创建" }

# ── 4. 活跃功能状态 ────────────────────────────
Write-Host ""
Write-Host "[ 4/5 ] 活跃功能" -ForegroundColor White

if (Test-Path $FeatureList) {
    try {
        $fl = Get-Content $FeatureList -Raw | ConvertFrom-Json
        $active = $fl.active_feature

        if ($null -eq $active -or $active -eq "") {
            Warn "当前没有活跃功能。请在 feature_list.json 中设置 active_feature 后开始工作。"
        } else {
            $feat = $fl.features | Where-Object { $_.id -eq $active } | Select-Object -First 1
            if ($feat) {
                Ok "活跃功能：$($feat.id) — $($feat.name) [状态: $($feat.status)]"

                # 检查 archive 文件是否存在
                $archiveFile = Join-Path $HarnessDir $feat.archive_file
                if (Test-Path $archiveFile) {
                    Ok "Archive 文件存在：$($feat.archive_file)"
                } else {
                    Warn "Archive 文件缺失：$($feat.archive_file)，请按模板创建"
                }
            } else {
                Fail "active_feature '$active' 在 features 数组中找不到对应条目"
            }
        }
    } catch {
        Fail "feature_list.json 解析失败：$_"
    }
}

# ── 5. 提醒事项 ────────────────────────────────
Write-Host ""
Write-Host "[ 5/5 ] 编译 & 编辑器提醒" -ForegroundColor White
Info "C++ 验证须在 Visual Studio（Development Editor/Win64）或 UE 编辑器内完成"
Info "蓝图验证须在 UE 编辑器内完成，Claude Code 无法读写 .uasset 文件"
Info "命令行构建参考 AGENTS.md 中的验证命令部分"

# ── 汇总 ───────────────────────────────────────
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
if ($errorCount -eq 0 -and $warnCount -eq 0) {
    Write-Host "  结果：全部通过 ✓  继续查看 progress.md 了解上次交接内容。" -ForegroundColor Green
} elseif ($errorCount -eq 0) {
    Write-Host "  结果：通过（$warnCount 条警告）  请关注上方警告后继续。" -ForegroundColor Yellow
} else {
    Write-Host "  结果：失败（$errorCount 个错误，$warnCount 条警告）  修复错误后再开始开发。" -ForegroundColor Red
    exit 1
}
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
