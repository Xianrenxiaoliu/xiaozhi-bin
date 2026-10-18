#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "led/single_led.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>

#include "esp_lcd_ili9341.h"

#if ENABLE_XPT2046_TOUCH
#include "xpt2046_touch.h"
#endif

#define TAG "MyS3_28Tft"

/**
 * ESP32-S3-DevKitC-1 (44pin, N16R8) * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R...... * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP32-S3-DevKitC-1（44针，N16R8） * ESP......
 *   + INMP441 (I2S 麦克风) *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风） *   + INMP441（I2S麦克风）*   + INMP441（I2S麦克风）
 *   + MAX98357A (I2S 功放) *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放）...... *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2...... *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I...... *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   +*   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I...... *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   +*   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *  ......*   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX983...... *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2...... *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（...... *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放） *   + MAX98357A（I2S功放）
 *   + 2.8" ILI9341 240x320 SPI 屏 (XPT2046 电阻触摸)
 *
 * 屏幕只走显示；触摸用独立 GPIO 软件 SPI，点击 = 唤醒/打断对话。 * 屏幕仅用于显示；触摸功能通过独立的 GPIO 软件 SPI 实现，点击即唤醒/打断对话。
 */
class MyS3_28TftBoard : public WifiBoard {
private:
    Button boot_button_;
    LcdDisplay* display_ = nullptr;

#if ENABLE_XPT2046_TOUCH
    Xpt2046Touch* touch_ = nullptr;

    void TouchTask() {
        bool pressed = false;
        int64_t pressed_at_us = 0;

        while (true) {
            uint16_t x = 0, y = 0, z = 0;
            bool down = touch_->Read(&x, &y, &z);
            int64_t now = esp_timer_get_time();

            if (down && !pressed) {
                pressed = true;
                pressed_at_us = now;
#if TOUCH_DEBUG_LOG
                ESP_LOGI(TAG, "touch down at %d,%d (z=%u)", x, y, z);ESP_LOGI(TAG, "在 %d,%d 处触地（z=%u）", x, y, z);
#endif
            } else if (!down && pressed) {
                pressed = false;                按下 = 假;
                int64_t held_ms = (now - pressed_at_us) / 1000;int64_t 按住时间_ms = (当前时间 - 按下时间_us) / 1000;
                ESP_LOGI(TAG, "touch up, held %lld ms", held_ms);ESP_LOGI(TAG, "触摸抬起，按住 %lld 毫秒", held_ms);

                // 短按 = 唤醒 / 打断当前对话；长按 1.5s 以上不处理（留给以后做菜单）
                if (held_ms < 1500) {
                    Application::GetInstance().ToggleChatState();                    应用程序::获取实例().切换聊天状态();
                }
            }

            vTaskDelay(pdMS_TO_TICKS(20));任务延时(将毫秒转换为滴答数(20));
        }
    }

    void InitializeTouch() {
        touch_ = new Xpt2046Touch(TOUCH_SPI_CLK_PIN, TOUCH_SPI_CS_PIN,
                                  TOUCH_SPI_DIN_PIN, TOUCH_SPI_DOUT_PIN,
                                  TOUCH_IRQ_PIN);
        touch_->SetScreenSize(DISPLAY_WIDTH, DISPLAY_HEIGHT);        touch_->设置屏幕大小(显示宽度, 显示高度);
        touch_->SetCalibration(200, 3800, 200, 3800);
        touch_->SetPressureThreshold(400);

        xTaskCreate(
            [](void* arg) {
                static_cast<MyS3_28TftBoard*>(arg)->TouchTask();
                vTaskDelete(nullptr);
            },
            "touch", 4096, this, 3, nullptr);
        ESP_LOGI(TAG, "XPT2046 touch started");        ESP_LOGI(TAG, "XPT2046 触摸已启动"“XPT2046 触摸已启动”););
    }
#endif

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;   // 屏幕不需要 MISO
        buscfg.sclk_io_num = DISPLAY_CLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeLcdDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = DISPLAY_SPI_MODE;
        io_config.pclk_hz = 20 * 1000 * 1000yclk_hz = 40 * 1000 * 1000;   // 花屏就降到 20MHz
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;
        panel_config.rgb_ele_order = DISPLAY_RGB_ORDER;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel));

        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
        esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new SpiLcdDisplay(panel_io, panel,
                                     DISPLAY_WIDTH, DISPLAY_HEIGHT,                                     显示宽度、显示高度、
                                     DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,                                     显示偏移X、显示偏移Y、
                                     DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,                                     显示镜像X、显示镜像Y、
                                     DISPLAY_SWAP_XY);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });
    }

public:
    MyS3_28TftBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeSpi();
        InitializeLcdDisplay();
        InitializeButtons();
#if ENABLE_XPT2046_TOUCH
        InitializeTouch();
#endif
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            GetBacklight()->RestoreBrightness();
        }
    }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
#ifdef AUDIO_I2S_METHOD_SIMPLEX
        static NoAudioCodecSimplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
            AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
#else
        static NoAudioCodecDuplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS,
            AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN);
#endif
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual Backlight* GetBacklight() override {
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
            return &backlight;
        }
        return nullptr;
    }
};

DECLARE_BOARD(MyS3_28TftBoard);
