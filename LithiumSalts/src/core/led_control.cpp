#include "led_control.h"
#ifdef HAS_RGB_LED
#include <globals.h>
#include "core/display.h"
#include "core/utils.h"
#include <FastLED.h>
#include "driver/rmt.h"

CRGB leds[LED_COUNT];

CRGB hsvToRgb(uint16_t h, uint8_t s, uint8_t v) {
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch ((h / 60) % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }

    CRGB c;
    c.red = r;
    c.green = g;
    c.blue = b;
    return c;
}


void beginLed() {
#ifdef RGB_LED_CLK
    FastLED.addLeds<LED_TYPE, RGB_LED, RGB_LED_CLK, LED_ORDER>(leds, LED_COUNT);
#else
    FastLED.addLeds<LED_TYPE, RGB_LED, LED_ORDER>(leds, LED_COUNT); // Initialize the LED Object. Only 1 LED.
#endif


/* The default FastLED driver takes over control of the RMT interrupt
 * handler, making it hard to use the RMT device for other
 * (non-FastLED) purposes. You can change it's behavior to use the ESP
 * core driver instead, allowing other RMT applications to
 * co-exist. To switch to this mode, add the following directive
 * before you include FastLED.h:
 *
 *      #define FASTLED_RMT_BUILTIN_DRIVER 1
 *  RMT is also used for RF Spectrum (and for RF readings in the future),
 *  So it is needed to restart the driver in case it had been turned off
 *  by the RF functions, in this case, we are restarting it all the time
 */
// -- RMT configuration for transmission
for (int i = 0; i < 8; i += 2)
    {
        rmt_config_t rmt_tx;
        memset(&rmt_tx, 0, sizeof(rmt_config_t));
        rmt_tx.channel = rmt_channel_t(i);
        rmt_tx.rmt_mode = RMT_MODE_TX;
        rmt_tx.gpio_num = (gpio_num_t)RGB_LED;
        rmt_tx.mem_block_num = 2;
        rmt_tx.clk_div = 2;
        rmt_tx.tx_config.loop_en = false;
        rmt_tx.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
        rmt_tx.tx_config.carrier_en = false;
        rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
        rmt_tx.tx_config.idle_output_en = true;

        // -- Apply the configuration
        rmt_config(&rmt_tx);
        rmt_driver_uninstall(rmt_channel_t(i));
        rmt_driver_install(rmt_channel_t(i), 0, 0);
    }


    setLedColor(LithiumSaltsConfig.ledColor);
    setLedBrightness(LithiumSaltsConfig.ledBright);
}


void setLedColor(CRGB color) {
    for (int i = 0; i < LED_COUNT; i++) leds[i] = color;
    FastLED.show();
}


void setLedBrightness(int value) {
    value = max(0, min(255, value));
    int bright = 255 * value/100;
    FastLED.setBrightness(bright);
    FastLED.show();
}


void setLedColorConfig() {
    int idx;
    if (LithiumSaltsConfig.ledColor==CRGB::Black) idx=0;
    else if (LithiumSaltsConfig.ledColor==CRGB::Purple) idx=1;
    else if (LithiumSaltsConfig.ledColor==CRGB::White) idx=2;
    else if (LithiumSaltsConfig.ledColor==CRGB::Red) idx=3;
    else if (LithiumSaltsConfig.ledColor==CRGB::Green) idx=4;
    else if (LithiumSaltsConfig.ledColor==CRGB::Blue) idx=5;
    else idx=6;  // custom color

    options = {
        {"OFF",    [=]() { LithiumSaltsConfig.setLedColor(CRGB::Black); }, LithiumSaltsConfig.ledColor == CRGB::Black },
        {"Purple", [=]() { LithiumSaltsConfig.setLedColor(CRGB::Purple); }, LithiumSaltsConfig.ledColor == CRGB::Purple},
        {"White",  [=]() { LithiumSaltsConfig.setLedColor(CRGB::White); }, LithiumSaltsConfig.ledColor == CRGB::White},
        {"Red",    [=]() { LithiumSaltsConfig.setLedColor(CRGB::Red); }, LithiumSaltsConfig.ledColor == CRGB::Red},
        {"Green",  [=]() { LithiumSaltsConfig.setLedColor(CRGB::Green); }, LithiumSaltsConfig.ledColor == CRGB::Green},
        {"Blue",   [=]() { LithiumSaltsConfig.setLedColor(CRGB::Blue); }, LithiumSaltsConfig.ledColor == CRGB::Blue},
    };

    if (idx == 6) options.emplace_back("Custom Color", [=]() { backToMenu(); }, true);
    options.emplace_back("Main Menu", [=]() { backToMenu(); });

    loopOptions(options, idx);
    setLedColor(LithiumSaltsConfig.ledColor);
}


void setLedBrightnessConfig() {
    int idx;
    if (LithiumSaltsConfig.ledBright==10) idx=0;
    else if (LithiumSaltsConfig.ledBright==25) idx=1;
    else if (LithiumSaltsConfig.ledBright==50) idx=2;
    else if (LithiumSaltsConfig.ledBright==75) idx=3;
    else if (LithiumSaltsConfig.ledBright==100) idx=4;

    options = {
        {"10 %", [=]() { LithiumSaltsConfig.setLedBright(10);  }, LithiumSaltsConfig.ledBright == 10 },
        {"25 %", [=]() { LithiumSaltsConfig.setLedBright(25);  }, LithiumSaltsConfig.ledBright == 25 },
        {"50 %", [=]() { LithiumSaltsConfig.setLedBright(50);  }, LithiumSaltsConfig.ledBright == 50 },
        {"75 %", [=]() { LithiumSaltsConfig.setLedBright(75);  }, LithiumSaltsConfig.ledBright == 75 },
        {"100%", [=]() { LithiumSaltsConfig.setLedBright(100); }, LithiumSaltsConfig.ledBright == 100 },
        {"Main Menu", [=]() { backToMenu(); }},
    };

    loopOptions(options, idx);
    setLedBrightness(LithiumSaltsConfig.ledBright);
}
#endif