# 小智 AI · ESP32-S3 + 2.8" ILI9341 + XPT2046 板级包

给你的硬件配好的一套 **xiaozhi-esp32 自定义开发板**：

- ESP32-S3-DevKitC-1（44 pin，N16R8）
- INMP441（I2S 麦克风）+ MAX98357A（I2S 功放）
- 2.8" ILI9341 240×320 SPI 屏，XPT2046 电阻触摸，13 pin 排针

> 状态说明：代码是按官方 `bread-compact-wifi-lcd` 板级实现改写的（显示部分与官方一致，属于已验证路径），
> 触摸驱动和横屏参数是**新增**的，需要你在实机上校准一次。
> 我没有实机可编译，第一次 `idf.py build` 若报错，把日志发我，我按报错改。

---

## 0. 想要「一个可以刷的 bin」

可以，但**必须由 ESP-IDF 编译产生**（里面是 Xtensa 机器码 + 分区表 + 字库资源，没法手写）。
目标文件是 `build/merged-binary.bin`：bootloader、分区表、app、assets 全合并在一起，
**烧录偏移 0x0**，一个文件搞定。

两条现成路径，挑一条：

### A. 本地一键（推荐，一次装好以后都能用）

1. 装 [ESP-IDF 6.1](https://dl.espressif.com/dl/esp-idf/)（离线安装包，约 1.3 GB）。
2. 从开始菜单打开 **ESP-IDF 6.1 PowerShell**。
3. 把本目录放到 `xiaozhi-esp32` 同级，然后：

```powershell
cd <本目录>
.\build.ps1 -Repo D:\esp\xiaozhi-esp32
```

脚本会自动：拷贝板级文件 → 打 Kconfig/CMake 补丁 → 编译 → 把 bin 复制到本目录
`xiaozhi-my-s3-28tft.bin`。加 `-FlashPort COM3` 可以顺手烧进去。

### B. 云端编译（本机什么都不用装，不用 fork 大仓库）

这个目录本身就是一个「编译用仓库」：CI 会现拉上游 `xiaozhi-esp32`，合入板级文件后编译。

1. GitHub 新建一个**空仓库**（Public，不要勾选 README / .gitignore）。
2. 把本目录的**全部内容**拖到仓库根目录上传，确认这三个路径存在：
   ```
   main/boards/my-s3-28tft/       ← 5 个板级文件
   patch_repo.py
   .github/workflows/build-my-s3-28tft.yml
   ```
   （`.github` 目录在 Windows 资源管理器里能直接拖；不行就用网页
   “Add file → Create new file”，文件名填 `.github/workflows/build-my-s3-28tft.yml` 再粘内容。）
3. 打开仓库 **Actions** 页 → 第一次会问是否启用，点启用 →
   左侧 `build-firmware` → 右侧 **Run workflow** → 绿色按钮。
4. 等 10~15 分钟，出现绿色 ✓ 后点进这次 run，页面底部 **Artifacts** →
   下载 `xiaozhi-my-s3-28tft`，解压得到 `merged-binary.bin`。

> 如果编译步骤报拉不到镜像（`manifest unknown` / `not found`），
> 把 yml 里 `esp_idf_version: v6.0.2` 改成 `v6.1`（或反过来），再 Run 一次。
>
> 想锁定上游版本：把 yml 里的 `git clone --depth 1 "$XIAOZHI_REPO" xz`
> 改成 `git clone --depth 1 --branch v1.7.0 ... xz`（tag 以仓库 releases 为准）。

### 烧写（不需要 IDF）

- 网页版：<https://espressif.github.io/esptool-js/>，地址填 `0x0`，选串口，写入。
- 或命令行：`esptool.py -p COM3 -b 460800 write_flash 0x0 merged-binary.bin`

> 首次上电会进配网模式：手机连 `Xiaozhi-xxxx` 热点 → 打开 `192.168.4.1` 填 WiFi →
> 再去 xiaozhi.me 添加设备、输入屏幕上的 6 位验证码。
> 如果屏幕颜色/方向不对，那属于面板参数问题，要改 `config.h` 重编（见第 3 节），
> 不影响语音功能。

---

## 1. 接线

### 1.1 屏幕（13 pin）

| 屏幕丝印 | 接到 ESP32-S3 | 说明 |
|---|---|---|
| VCC | 3V3 | 不要接 5V |
| GND | GND | |
| RESET | GPIO45 | 官方 LCD 方案的复位脚 |
| D/C | GPIO40 | 命令/数据 |
| SDI(MOSI) | GPIO47 | |
| SCK | GPIO21 | |
| LED | 3V3 | 见下方“背光” |
| SDO(MISO) | 不接 | 显示用不到 |
| T_CLK | GPIO12 | 触摸时钟 |
| T_CS | GPIO10 | 触摸片选 |
| T_DIN | GPIO11 | 主机 → 触摸 |
| Y_OUT | GPIO13 | 触摸 → 主机（就是 T_DO，名字各家乱写） |
| T_IRQ | GPIO14 | 可选，不接也能用 |

**关于 CS**：你这块 13 pin 没有引出 LCD 的 CS，说明板上已经把它拉低了（永远选中）。
所以 `config.h` 里 `DISPLAY_CS_PIN` 设成了 `GPIO_NUM_NC`。
如果你发现屏其实有 CS 焊盘/丝印，把它接到 GPIO41，并把该宏改成 `GPIO_NUM_41`。

**关于 Y_OUT**：XPT2046 的引脚是 T_CLK / T_CS / T_DIN / T_DO / T_IRQ，
`Y_OUT` 这个丝印实际就是 T_DO（触摸的数据输出）。如果触摸完全没反应，
先把 `T_DIN` 和 `Y_OUT` 两根线对调试一次。

**背光**：2.8" 屏背光约 40~80 mA，别直接由 GPIO 供。要么把 LED 直接接 3V3（常亮），
要么加一个 S8050/MOSFET，由 GPIO42 做 PWM 调光（`DISPLAY_BACKLIGHT_PIN` 已经是 42）。
如果直接接 3V3，把 `DISPLAY_BACKLIGHT_PIN` 改成 `GPIO_NUM_NC`，否则代码会在 42 上输出 PWM 但什么也没控制。

### 1.2 麦克风 / 功放

| INMP441 | ESP32-S3 | | MAX98357A | ESP32-S3 |
|---|---|---|---|---|
| VDD | 3V3 | | Vin | 5V（或 3V3） |
| GND | GND | | GND | GND |
| L/R | GND | | SD | 3V3（拉高才工作） |
| WS | GPIO4 | | GAIN | GND（最低增益，先别接 VDD） |
| SCK | GPIO5 | | DIN | GPIO7 |
| SD | GPIO6 | | BCLK | GPIO15 |
| | | | LRC | GPIO16 |

- 功放电源脚并 **470~1000 µF 电解 + 0.1 µF**，否则大音量会掉压重启。
- 喇叭接 4Ω/8Ω 2~3W，MAX98357 是 BTL 输出，**音频- 不要接 GND**。
- 先只接屏点亮，再插麦克风/功放，出问题好定位。

---

## 2. 装进工程（3 步）

把本目录下的

```
main/boards/my-s3-28tft/
├── config.h
├── config.json
├── my_s3_28tft.cc
├── xpt2046_touch.h
└── xpt2046_touch.cc
```

整个复制到 `xiaozhi-esp32/main/boards/my-s3-28tft/`。

> 用 `build.ps1` 的话，下面 2.1 / 2.2 两步它会自动打补丁，可以跳过。

### 2.1 在 `main/Kconfig.projbuild` 里注册

在 `choice BOARD_TYPE` 块内任意位置插入（比如放在 `config BOARD_TYPE_BREAD_COMPACT_WIFI_LCD` 前面）：

```kconfig
    config BOARD_TYPE_MY_S3_28TFT
        bool "My ESP32-S3 2.8 inch TFT (ILI9341 + XPT2046)"
        depends on IDF_TARGET_ESP32S3
```

### 2.2 在 `main/CMakeLists.txt` 里注册

找到板型 `if / elseif` 选择链的结尾 `endif()`，在它**之前**插入：

```cmake
elseif(CONFIG_BOARD_TYPE_MY_S3_28TFT)
    set(BOARD_DIR "my-s3-28tft")
    set(BUILTIN_TEXT_FONT font_noto_sans_basic_20_4)
    set(BUILTIN_ICON_FONT font_material_symbols_20_4)
```

`BOARD_DIR` 下的 `*.cc` 会被自动 glob，所以 `xpt2046_touch.cc` 不用单独登记。

### 2.3 编译

```bash
idf.py set-target esp32s3
python scripts/build.py my-s3-28tft --language zh-CN --wake-word nihaoxiaozhi
idf.py -p COM3 flash monitor
```

- 一定要带 `--language zh-CN`，否则默认英文界面。
- `--wake-word nihaoxiaozhi` 是“你好小智”；不想用唤醒词就写 `disabled`，改用屏幕点击/BOOT 键说话。
- 不想用 `build.py` 的话：`idf.py menuconfig` → *Xiaozhi Assistant → Target Board* 选
  “My ESP32-S3 2.8 inch TFT”，再 `idf.py build`。

---

## 3. 屏幕调参（第一次大概率要改这里）

在 `config.h` 里：

| 现象 | 改法 |
|---|---|
| 颜色像底片 / 负片 | `DISPLAY_INVERT_COLOR` 取反 |
| 图像上下颠倒 | `DISPLAY_MIRROR_Y` 取反 |
| 左右颠倒 | `DISPLAY_MIRROR_X` 取反 |
| 想竖屏 240×320 | 宽高对调 + `DISPLAY_SWAP_XY false` |
| 花屏、雪花、闪 | `my_s3_28tft.cc` 里 `pclk_hz` 40 MHz 降到 20 MHz |
| 边缘偏移 / 有彩条 | 调 `DISPLAY_OFFSET_X/Y` |
| 颜色偏红/偏蓝（红蓝互换） | `DISPLAY_RGB_ORDER` 在 `RGB` / `BGR` 间切换 |

默认给的是 **横屏 320×240 + swap_xy + mirror_x + BGR + 不反色**（2.8" TN 屏常见组合）。
IPS 屏通常要把 `DISPLAY_INVERT_COLOR` 打开。

---

## 4. 触摸

### 4.1 现在的行为

上电后 20 ms 轮询一次，**短按屏幕 = 唤醒 / 打断当前对话**（等同 BOOT 键）。
长按超过 1.5 s 目前不处理，留给以后做菜单。

### 4.2 校准

1. `config.h` 里把 `TOUCH_DEBUG_LOG` 改成 `1`。
2. 烧进去，串口看 `Xpt2046: raw=... z=... -> x,y`。
3. 分别点屏幕四角，记下 raw_x / raw_y 的最小、最大值。
4. 把值填进 `my_s3_28tft.cc` 的 `SetCalibration(x_min, x_max, y_min, y_max)`。
5. 如果 Y 方向反了，把 y_min / y_max 对调。校准完把日志关掉。

### 4.3 排错

| 现象 | 原因 |
|---|---|
| 完全没反应 | T_DIN / Y_OUT 接反；T_CS 没上拉；T_CLK 没接 |
| 一直处于“按下” | 压力阈值太小，`SetPressureThreshold(600~1000)` 调大；或 T_IRQ 悬空被当成低 |
| 点的位置差很远 | 没校准，或 `SetScreenSize` 与 `DISPLAY_WIDTH/HEIGHT` 不一致 |
| 乱跳点 | 采样太快，把轮询 20 ms 调大；或线太长 |

> 触摸走的是 4 根独立 GPIO 的软件 SPI（约 250 kHz），**不占屏幕总线**，
> 所以不存在“屏幕 CS 常低导致触摸把屏幕刷花”的问题。

---

## 5. 关于 SD 卡槽

你这块屏的 **13 pin 排针里没有 SD 的引脚**（没有 SD_CS / SD_MOSI 之类）。
这类模块的卡座有两种情况：

1. 卡座通过板上走线并在屏幕 SPI 上，CS 另有一个**单独焊盘/排针**（仔细看板子背面）；
2. 卡座根本没引出来 —— 那就是个摆设，用不了。

而且因为这块屏的 **LCD CS 是常低的**，SD 卡几乎不可能和屏幕共用一条 SPI 总线
（SD 的时钟会被屏幕当成自己的数据）。所以：

- 想确认：用万用表量卡座的 CLK/CMD/DAT 脚是否通到排针或独立焊盘；
- 结论大概率是：**这个卡槽放弃**，音乐走网络/服务器方案（下节）。

---

## 6. 音乐播放怎么加

先跑通语音，再按投入选一条：

1. **服务器点歌（推荐先做，固件零改动）**
   自建 [xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server)，
   把 mp3 放进 `music/` 目录重启，对设备说“放首××”。
   服务器会把音频转成 16 kHz 单声道 Opus 推给设备。
2. **音乐固件（有歌词界面）**
   [Maggotxy/xiaozhi-esp32-music](https://github.com/Maggotxy/xiaozhi-esp32-music)，
   提供 `self.music.play_song`，自带歌词显示，适合 2.8" 横屏。
   它基于较早版本的 xiaozhi 和 IDF 5.4，移植时把本目录的 `config.h` 引脚表搬过去即可。
3. **本地 SD / U 盘播放**
   需要 FATFS + MP3 解码（`espressif/esp-libhelix-mp3`）+ 重采样到 24 kHz 塞进现有音频输出，
   并处理“播放时被唤醒打断”。工作量 3~7 天，且你这块屏的卡槽大概率不可用。

> 这套硬件没有 AEC 参考回采，播放音乐时麦克风会把喇叭声收进去。
> 建议：播放时把音量压到 60% 以下，或做“播放中暂停拾音、按键/触摸才恢复”。

---

## 7. 常见问题

- **上电反复重启**：GPIO45 是 strapping 脚，屏幕模块若把它上拉到 3.3V 会影响 Flash 电压选择。
  把 RESET 换到 GPIO9 之类空闲脚，同时改 `DISPLAY_RST_PIN`。
- **屏幕亮但全白**：`pclk_hz` 太高、`DISPLAY_SPI_MODE` 不对、或 RESET 没接好。
- **麦克风全是噪声**：L/R 没接地；或 VDD 接了 5V（INMP441 只能 3.3V）。
- **功放没声**：SD 脚没拉高；GAIN 悬空；喇叭接成了单端。
- **说一句话就被自己打断**：没有 AEC，降低音量或改按键/触摸说话。
- **PSRAM 没识别**：本工程 `sdkconfig.defaults.esp32s3` 已经是 `SPIRAM_MODE_OCT`，
  正好对应 N16R8，一般不用改。

---

## 8. 文件说明

| 文件 | 作用 |
|---|---|
| `config.h` | 全部引脚、屏幕参数、触摸开关 |
| `my_s3_28tft.cc` | 板级初始化：SPI、ILI9341、按键、触摸任务、音频 codec、背光 |
| `xpt2046_touch.h/.cc` | XPT2046 软件 SPI 驱动 + 坐标映射 |
| `config.json` | 上报的板型 `my-s3-28tft`、目标 `esp32s3`，给 `build.py` 用 |
