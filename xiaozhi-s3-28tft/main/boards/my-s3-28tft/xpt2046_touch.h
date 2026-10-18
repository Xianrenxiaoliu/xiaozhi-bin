#pragma once

#include <driver/gpio.h>
#include <cstdint>

/**
 * XPT2046 电阻触摸，软件 SPI 实现。
 *
 * 说明：
 *  - 只依赖 4 根普通 GPIO，不走硬件 SPI，因此和 ILI9341 屏幕总线完全解耦
 *    （很多 2.8" 模块把 LCD 的 CS 直接拉低，共用总线会互相干扰）。
 *  - 坐标默认按 200..3800 的原始范围线性映射到屏幕分辨率，实际值请打开
 *    TOUCH_DEBUG_LOG 打印后自行校准。
 */
class Xpt2046Touch {
public:
    Xpt2046Touch(gpio_num_t clk, gpio_num_t cs, gpio_num_t din,
                 gpio_num_t dout, gpio_num_t irq = GPIO_NUM_NC);
    ~Xpt2046Touch() = default;

    /** 读取一次触摸：返回是否按下；x/y 为映射后的屏幕坐标（0..width-1） */
    bool Read(uint16_t* x, uint16_t* y, uint16_t* pressure);

    /** 读取原始值（12bit，0..4095），用于校准 */
    bool ReadRaw(uint16_t* raw_x, uint16_t* raw_y, uint16_t* raw_z);

    /** 设置触摸坐标的映射范围（对着屏幕四角点一下，记下原始值填进来） */
    void SetCalibration(uint16_t x_min, uint16_t x_max,
                        uint16_t y_min, uint16_t y_max);

    /** 压力阈值：越小越灵敏；默认 400，容易误触就调大 */
    void SetPressureThreshold(uint16_t threshold) { pressure_threshold_ = threshold; }

    /** 屏幕分辨率，用于把原始坐标映射成像素坐标 */
    void SetScreenSize(uint16_t width, uint16_t height) {
        screen_width_ = width;
        screen_height_ = height;
    }

    /** 额外做 2 次采样取中位数，抗抖（默认开） */
    void SetOversampling(bool enable) { oversampling_ = enable; }

private:
    uint16_t Transfer(uint8_t command);
    bool IsTouched(uint16_t raw_z);

    gpio_num_t clk_;
    gpio_num_t cs_;
    gpio_num_t din_;
    gpio_num_t dout_;
    gpio_num_t irq_;

    uint16_t x_min_ = 200;
    uint16_t x_max_ = 3800;
    uint16_t y_min_ = 200;
    uint16_t y_max_ = 3800;

    uint16_t screen_width_ = 320;
    uint16_t screen_height_ = 240;

    uint16_t pressure_threshold_ = 400;
    bool oversampling_ = true;
};
