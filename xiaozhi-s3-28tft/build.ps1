# =====================================================================
#  一键把小智固件编成 merged-binary.bin（可直接刷入的单个 bin）
#
#  前置：装好 ESP-IDF 6.1，并从「ESP-IDF PowerShell」或「ESP-IDF CMD」
#        里运行本脚本（那样 IDF_PATH / python 才在环境里）。
#
#  用法：
#    .\build.ps1                                  # 仓库在 ..\xiaozhi-esp32
#    .\build.ps1 -Repo D:\esp\xiaozhi-esp32       # 指定仓库位置
#    .\build.ps1 -FlashPort COM3                  # 编完顺便烧录
#
#  产物：本目录下 xiaozhi-my-s3-28tft.bin
# =====================================================================

param(
    [string]$Repo = "",
    [string]$Language = "zh-CN",
    [string]$WakeWord = "nihaoxiaozhi",
    [string]$FlashPort = ""
)

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$boardSrc = Join-Path $here "main\boards\my-s3-28tft"

if (-not (Test-Path $boardSrc)) {
    throw "找不到板级目录: $boardSrc"
}

if ([string]::IsNullOrWhiteSpace($Repo)) {
    $Repo = Join-Path (Split-Path -Parent $here) "xiaozhi-esp32"
}
if (-not (Test-Path (Join-Path $Repo "main\CMakeLists.txt"))) {
    throw "这不是 xiaozhi-esp32 仓库: $Repo（请用 -Repo 指定）"
}
$Repo = (Resolve-Path $Repo).Path

# ---------- 1. 拷贝板级目录 ----------
$dst = Join-Path $Repo "main\boards\my-s3-28tft"
New-Item -ItemType Directory -Force -Path $dst | Out-Null
Copy-Item (Join-Path $boardSrc "*") $dst -Force -Recurse
Write-Host "[1/4] 板级文件已复制 -> $dst" -ForegroundColor Green

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $enc = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $enc)
}

# ---------- 2. 注册 Kconfig ----------
$kconfigPath = Join-Path $Repo "main\Kconfig.projbuild"
$k = [System.IO.File]::ReadAllText($kconfigPath)
if ($k -notmatch "BOARD_TYPE_MY_S3_28TFT") {
    $kblock = @"
    config BOARD_TYPE_MY_S3_28TFT
        bool "My ESP32-S3 2.8 inch TFT (ILI9341 + XPT2046)"
        depends on IDF_TARGET_ESP32S3

"@
    $anchor = "    config BOARD_TYPE_BREAD_COMPACT_WIFI_LCD"
    if ($k.Contains($anchor)) {
        $k = $k.Replace($anchor, $kblock + $anchor)
    }
    else {
        $ci = $k.IndexOf("choice BOARD_TYPE")
        $ei = -1
        if ($ci -ge 0) { $ei = $k.IndexOf("endchoice", $ci) }
        if ($ei -lt 0) { throw "Kconfig 里找不到 BOARD_TYPE 选择块，请手工添加。" }
        $k = $k.Insert($ei, $kblock)
    }
    Write-Utf8NoBom $kconfigPath $k
    Write-Host "[2/4] Kconfig 已注册" -ForegroundColor Green
}
else {
    Write-Host "[2/4] Kconfig 已有该板型，跳过" -ForegroundColor DarkGray
}

# ---------- 3. 注册 CMakeLists ----------
$cmakePath = Join-Path $Repo "main\CMakeLists.txt"
$c = [System.IO.File]::ReadAllText($cmakePath)
if ($c -notmatch "BOARD_TYPE_MY_S3_28TFT") {
    $cblock = @"
elseif(CONFIG_BOARD_TYPE_MY_S3_28TFT)
    set(BOARD_DIR "my-s3-28tft")
    set(BUILTIN_TEXT_FONT font_noto_sans_basic_20_4)
    set(BUILTIN_ICON_FONT font_material_symbols_20_4)
"@
    $anchor1 = "elseif(CONFIG_BOARD_TYPE_BREAD_COMPACT_WIFI_LCD)"
    $anchor2 = "elseif(CONFIG_BOARD_TYPE_BREAD_COMPACT_WIFI)"
    if ($c.Contains($anchor1)) {
        $c = $c.Replace($anchor1, $cblock + $anchor1)
    }
    elseif ($c.Contains($anchor2)) {
        $c = $c.Replace($anchor2, $cblock + $anchor2)
    }
    else {
        $marker = "if(NOT BOARD_DIR AND NOT CMAKE_BUILD_EARLY_EXPANSION)"
        $mi = $c.IndexOf($marker)
        $ei = -1
        if ($mi -gt 0) { $ei = $c.LastIndexOf("endif()", $mi) }
        if ($ei -lt 0) { throw "CMakeLists.txt 里找不到板型分支结尾，请手工添加。" }
        $c = $c.Insert($ei, $cblock + "`n")
    }
    Write-Utf8NoBom $cmakePath $c
    Write-Host "[3/4] CMakeLists 已注册" -ForegroundColor Green
}
else {
    Write-Host "[3/4] CMakeLists 已有该板型，跳过" -ForegroundColor DarkGray
}

# ---------- 4. 编译 ----------
if (-not $env:IDF_PATH) {
    Write-Host ""
    Write-Host "没检测到 ESP-IDF 环境变量 IDF_PATH。" -ForegroundColor Yellow
    Write-Host "请从开始菜单打开 “ESP-IDF 6.1 PowerShell”，再运行本脚本；" -ForegroundColor Yellow
    Write-Host "或在当前窗口执行:  C:\Espressif\frameworks\esp-idf-v6.1\export.ps1" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "板级文件已经装好，重跑本脚本即可继续。" -ForegroundColor Yellow
    exit 1
}

Push-Location $Repo
try {
    Write-Host "[4/4] 开始编译（首次会下载组件，约 5~15 分钟）..." -ForegroundColor Green
    & python scripts/build.py my-s3-28tft --language $Language --wake-word $WakeWord
    if ($LASTEXITCODE -ne 0) { throw "编译失败，退出码 $LASTEXITCODE" }

    $binPath = Join-Path $Repo "build\merged-binary.bin"
    if (-not (Test-Path $binPath)) { throw "没找到 $binPath，编译可能没走 merge-bin" }

    $outBin = Join-Path $here "xiaozhi-my-s3-28tft.bin"
    Copy-Item $binPath $outBin -Force
    Write-Host ""
    Write-Host "OK -> $outBin" -ForegroundColor Green
    Write-Host ("大小: {0:N2} MB" -f ((Get-Item $outBin).Length / 1MB)) -ForegroundColor Green

    if ($FlashPort) {
        Write-Host "烧录到 $FlashPort ..." -ForegroundColor Green
        & idf.py -p $FlashPort flash
    }
}
finally {
    Pop-Location
}
