#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"

// KONFIGURACJA
#define BLINK_GPIO  13       // Pin danych (DIN)
#define LED_COUNT   60        // Ile diod wlaczamy (bezpiecznie dla USB!)

static const char *TAG = "SMART_LIGHT";
static led_strip_handle_t led_strip; // Uchwyt do naszej tasmy

// Funkcja konfigurujaca tasme
void configure_led(void)
{
    // 1. Konfiguracja pinu RMT (to ten mechanizm sprzetowy)
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = LED_COUNT, // Max liczba diod
        .led_pixel_format = LED_PIXEL_FORMAT_GRBW, // SK6812 zazwyczaj ma GRB
        .led_model = LED_MODEL_SK6812, // Dedykowany model
        .flags.invert_out = false,
    };

    // 2. Konfiguracja RMT (Remote Control Transceiver)
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };

    // 3. Instalacja sterownika
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Tasma LED skonfigurowana na pinie %d", BLINK_GPIO);
}

void app_main(void)
{
    configure_led();
    ESP_LOGI(TAG, "Startujemy test LED!");

    while (1) {
        // --- CZERWONY ---
        ESP_LOGI(TAG, "Kolor: CZERWONY");
        for (int i = 0; i < LED_COUNT; i++) {
            // Ustaw diode i: Red=20, Green=0, Blue=0, White=0 (oszczedzamy prad, jasnosc 20/255)
            led_strip_set_pixel(led_strip, i, 20, 0, 0); 
        }
        led_strip_refresh(led_strip); // Wyslij dane do paska
        vTaskDelay(pdMS_TO_TICKS(1000));

        // --- ZIELONY ---
        ESP_LOGI(TAG, "Kolor: ZIELONY");
        for (int i = 0; i < LED_COUNT; i++) {
            led_strip_set_pixel(led_strip, i, 0, 20, 0);
        }
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // --- NIEBIESKI ---
        ESP_LOGI(TAG, "Kolor: NIEBIESKI");
        for (int i = 0; i < LED_COUNT; i++) {
            led_strip_set_pixel(led_strip, i, 0, 0, 20);
        }
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // --- WYLACZ ---
        led_strip_clear(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}