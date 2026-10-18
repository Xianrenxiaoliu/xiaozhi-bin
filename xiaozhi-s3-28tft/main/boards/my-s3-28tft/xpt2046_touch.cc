#include "xpt2046_touch.h"
#include "config.h"

#include <esp_rom_sys.h>
#include <esp_log.h>
#include <algorithm>

namespace {
constexpr char kTag[] = "Xpt2046";

// XPT2046 命令字：起始位 + 通道 + 12bit + 差分 + 掉电模式
constexpr uint8_t kCmdX  = 0xD0;
constexpr uint8_t kCmdY  = 0x90;
constexpr uint8_t kCmdZ1 = 0xB0;
constexpr uint8_t kCmdZ2 = 0xC0;

// 软件 SPI 半周期延时（us）。2us ≈ 250kHz，足够稳，20Hz 轮询毫无压力。
constexpr uint32_t kHalfPeriodUs = 2;
}  // namespace

Xpt2046Touch::Xpt2046Touch(gpio_num_t clk, gpio_num_t cs, gpio_num_t din,
                           gpio_num_t dout, gpio_num_t irq)
    : clk_(clk), cs_(cs), din_(din), dout_(dout), irq_(irq) {
    gpio_config_t out_cfg = {};
    out_cfg.pin_bit_mask = (1ULL << clk_) | (1ULL << cs_) | (1ULL << din_);
    out_cfg.mode = GPIO_MODE_OUTPUT;
    out_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    out_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    out_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&out_cfg);

    gpio_config_t in_cfg = {};
    in_cfg.pin_bit_mask = (1ULL << dout_);
    if (irq_ != GPIO_NUM_NC) {
        in_cfg.pin_bit_mask |= (1ULL << irq_);
    }
    in_cfg.mode = GPIO_MODE_INPUT;
    in_cfg.pull_up_en = GPIO_PULLUP_ENABLE;  // T_DO / PENIRQ 都需要上拉
    in_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    in_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&in_cfg);

    gpio_set_level(cs_, 1);
    gpio_set_level(clk_, 0);
}

uint16_t Xpt2046Touch::Transfer(uint8_t command) {
    gpio_set_level(cs_, 0);
    esp_rom_delay_us(kHalfPeriodUs);

    // 命令：MSB first，上升沿采样
    for (int i = 7; i >= 0; --i) {
        gpio_set_level(din_, (command >> i) & 0x01);
        esp_rom_delay_us(kHalfPeriodUs);
        gpio_set_level(clk_, 1);
        esp_rom_delay_us(kHalfPeriodUs);
        gpio_set_level(clk_, 0);
    }

    // 读 16 bit：前 1 bit busy + 12 bit 数据 + 3 bit 0
    uint16_t value = 0;
    for (int i = 0; i < 16; ++i) {
        gpio_set_level(clk_, 1);
        esp_rom_delay_us(kHalfPeriodUs);
        value = static_cast<uint16_t>((value << 1) | (gpio_get_level(dout_) ? 1 : 0));
        gpio_set_level(clk_, 0);
        esp_rom_delay_us(kHalfPeriodUs);
    }

    gpio_set_level(cs_, 1);
    esp_rom_delay_us(kHalfPeriodUs);
    return value;
}

bool Xpt2046Touch::IsTouched(uint16_t raw_z) {
    if (irq_ != GPIO_NUM_NC) {
        // PENIRQ 低有效，模块上一般有上拉
        if (gpio_get_level(irq_) == 0) {
            return true;
        }
        // IRQ 没接/没上拉时也不能完全依赖它，继续看压力值
    }
    return raw_z > pressure_threshold_;
}

bool Xpt2046Touch::ReadRaw(uint16_t* raw_x, uint16_t* raw_y, uint16_t* raw_z) {
    uint16_t x = static_cast<uint16_t>((Transfer(kCmdX) >> 3) & 0x0FFF);
    uint16_t y = static_cast<uint16_t>((Transfer(kCmdY) >> 3) & 0x0FFF);
    uint16_t z1 = static_cast<uint16_t>((Transfer(kCmdZ1) >> 3) & 0x0FFF);

    if (oversampling_) {
        uint16_t x2 = static_cast<uint16_t>((Transfer(kCmdX) >> 3) & 0x0FFF);
        uint16_t y2 = static_cast<uint16_t>((Transfer(kCmdY) >> 3) & 0x0FFF);
        // 取中位数：两个值取平均已经够用，避免引入大数组
        x = static_cast<uint16_t>((x + x2) / 2);
        y = static_cast<uint16_t>((y + y2) / 2);
    }

    if (raw_x) *raw_x = x;
    if (raw_y) *raw_y = y;
    if (raw_z) *raw_z = z1;
    return IsTouched(z1);
}

bool Xpt2046Touch::Read(uint16_t* x, uint16_t* y, uint16_t* pressure) {
    uint16_t raw_x = 0, raw_y = 0, raw_z = 0;
    bool touched = ReadRaw(&raw_x, &raw_y, &raw_z);

    if (pressure) *pressure = raw_z;

    if (!touched) {
        return false;
    }

    if (x_min_ == x_max_ || y_min_ == y_max_) {
        return false;
    }

    int32_t px = (static_cast<int32_t>(raw_x) - x_min_) * (screen_width_ - 1) /
                 (static_cast<int32_t>(x_max_) - x_min_);
    int32_t py = (static_cast<int32_t>(raw_y) - y_min_) * (screen_height_ - 1) /
                 (static_cast<int32_t>(y_max_) - y_min_);

    px = std::clamp<int32_t>(px, 0, screen_width_ - 1);
    py = std::clamp<int32_t>(py, 0, screen_height_ - 1);

    if (x) *x = static_cast<uint16_t>(px);
    if (y) *y = static_cast<uint16_t>(py);

#if TOUCH_DEBUG_LOG
    ESP_LOGI(kTag, "raw=%u,%u z=%u -> %d,%d", raw_x, raw_y, raw_z, px, py);
#endif
    return true;
}

void Xpt2046Touch::SetCalibration(uint16_t x_min, uint16_t x_max,
                                  uint16_t y_min, uint16_t y_max) {
    x_min_ = x_min;
    x_max_ = x_max;
    y_min_ = y_min;
    y_max_ = y_max;
}
