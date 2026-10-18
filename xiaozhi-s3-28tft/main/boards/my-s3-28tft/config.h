#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

/*
 * 小智 AI —— ESP32-S3 DevKitC-1 (44pin, N16R8) + 2.8" ILI9341 + XPT2046
 * 接线完全兼容官方「面包板 Wi-Fi + LCD」教程，方便照图施工。
 */

// ======================= 音频 =======================
// INMP441 只支持 16k 采样，输出走 24k
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// 两个 I2S 控制器（Simplex）：麦克风一组、功放一组，互不干扰
#define AUDIO_I2S_METHOD_SIMPLEX

// INMP441：WS / SCK / SD   (L/R 必须接 GND)
#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_4
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_5
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_6

// MAX98357A：DIN / BCLK / LRC  (SD 接 3V3 使能, GAIN 接 GND)
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_7
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_15
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_16

// ======================= 按键 / 指示灯 =======================
#define BOOT_BUTTON_GPIO        GPIO_NUM_0
#define BUILTIN_LED_GPIO        GPIO_NUM_48   // DevKitC-1 v1.1 的 RGB 灯；v1.0 是 38，可自行改

// ======================= 2.8" ILI9341 (SPI) =======================
#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_42
#define DISPLAY_MOSI_PIN      GPIO_NUM_47
#define DISPLAY_CLK_PIN       GPIO_NUM_21
#define DISPLAY_DC_PIN        GPIO_NUM_40
#define DISPLAY_RST_PIN       GPIO_NUM_45

// 你这块 13 针模块没有引出 LCD 的 CS（板上直接拉低，永远选中），
// 所以这里必须给 NC，让 esp_lcd 不要去找片选脚。
// 如果你的屏其实是 14 针、带独立 CS 丝印，改成 GPIO_NUM_41 并把 CS 接到 41。
#define DISPLAY_CS_PIN        GPIO_NUM_NC

#define LCD_TYPE_ILI9341_SERIAL

// 横屏 320x240（想竖屏就把宽高对调、SWAP_XY 改 false）
#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY true
#define DISPLAY_INVERT_COLOR false          // 颜色发"底片"就改成 true
#define DISPLAY_RGB_ORDER  LCD_RGB_ELEMENT_ORDER_BGR
#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
#define DISPLAY_SPI_MODE 0

// ======================= XPT2046 电阻触摸 =======================
// 用 4 根独立 GPIO 做软件 SPI，不占用屏幕总线，也不受屏幕 CS 是否常低的影响。
// T_IRQ 可选（不接也能用，靠压力值判断按下）。
#define ENABLE_XPT2046_TOUCH 1
#define TOUCH_SPI_CLK_PIN   GPIO_NUM_12   // T_CLK
#define TOUCH_SPI_CS_PIN    GPIO_NUM_10   // T_CS
#define TOUCH_SPI_DIN_PIN   GPIO_NUM_11   // T_DIN  (主机 -> 触摸)
#define TOUCH_SPI_DOUT_PIN  GPIO_NUM_13   // Y_OUT / T_DO (触摸 -> 主机)
#define TOUCH_IRQ_PIN       GPIO_NUM_14   // T_IRQ，不用就设 GPIO_NUM_NC

// 打印原始坐标，用来校准/排错；调好后改回 0
#define TOUCH_DEBUG_LOG 0

#endif  // _BOARD_CONFIG_H_
