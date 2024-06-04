/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "esp_log.h"
#include "esp_check.h"
#include "keyboard_button.h"
#include "bsp/keyboard.h"
#include "bsp/lightmap.h"
#include "led_strip.h"
#include "rgb_matrix_drivers.h"
#include "rgb_matrix.h"

static const char *TAG = "kbd_kit";

esp_err_t bsp_keyboard_init(keyboard_btn_handle_t *kbd_handle, keyboard_btn_config_t *ex_cfg)
{
    ESP_RETURN_ON_FALSE(kbd_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "kbd_handle is NULL");

    if (ex_cfg != NULL) {
        keyboard_button_create(ex_cfg, kbd_handle);
    } else {
        keyboard_btn_config_t cfg = {
            .output_gpios = (int[])KBD_OUTPUT_IOS,
            .output_gpio_num = KBD_ROW_NUM,
            .input_gpios = (int[])KBD_INPUT_IOS,
            .input_gpio_num = KBD_COL_NUM,
            .active_level = KBD_ATTIVE_LEVEL,
            .debounce_ticks = 2,
            .ticks_interval = KBD_TICKS_INTERVAL_US,
            .enable_power_save = true,
        };
        keyboard_button_create(&cfg, kbd_handle);
    }

    return ESP_OK;
}

static led_strip_handle_t s_led_strip = NULL;

esp_err_t bsp_ws2812_init(led_strip_handle_t *led_strip)
{
    if (s_led_strip) {
        if (led_strip) {
            *led_strip = s_led_strip;
        }
        return ESP_OK;
    }
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = LIGHTMAP_GPIO, // The GPIO that connected to the LED strip's data line
        .max_leds = LIGHTMAP_NUM, // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812, // LED strip model
        .flags.invert_out = false, // whether to invert the output signal (useful when your hardware has a level inverter)
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
        .resolution_hz = 20 * 1000 * 1000, // 10MHz
        .flags.with_dma = false, // whether to enable the DMA feature
    };
    led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (led_strip) {
        *led_strip = s_led_strip;
    }
    return ESP_OK;
}

esp_err_t bsp_lamp_array_init(uint32_t bind)
{
    if (!s_led_strip) {
        bsp_ws2812_init(NULL);
    }
    lamp_array_matrix_cfg_t cfg = {
        .lamp_array_width = LIGHTMAP_WIDTH,
        .lamp_array_height = LIGHTMAP_HEIGHT,
        .lamp_array_depth = LIGHTMAP_DEPTH,
        .lamp_array_rotation = LampPositions,
        .pixel_cnt = LIGHTMAP_NUM,
        .update_interval = LIGHTMAP_UPDATE_INTERVAL,
        .handle = s_led_strip,
        .bind_key = bind,
    };
    lamp_array_matrix_init(cfg);
    return ESP_OK;
}

esp_err_t bsp_rgb_matrix_init(void)
{
    if (!s_led_strip) {
        bsp_ws2812_init(NULL);
    }
    rgb_matrix_driver_init(s_led_strip, LIGHTMAP_NUM);
    rgb_matrix_init();
    return ESP_OK;
}
