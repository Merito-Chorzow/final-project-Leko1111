#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"       // Biblioteka do obslugi pinow wejsciowych
#include "led_strip.h"

// --- KONFIGURACJA ---
#define LED_PIN     13          // Tu masz podpieta tasme (DIN)
#define RADAR_PIN   14          // Tu podpinamy OUT z radaru
#define LED_COUNT   60          // Twoja liczba diod (1 metr)
#define ANIMATION_SPEED_MS  20  // Szybkosc "pływania" (im mniej, tym szybciej)

static const char *TAG = "SMART_LIGHT";
static led_strip_handle_t led_strip;

// Funkcja wlaczajaca swiatlo (Bialy cieply)
void light_on() {
    ESP_LOGI(TAG, "Animacja START ->");
    for (int i = 0; i < LED_COUNT; i++) {
        // ZMIANA: Podajemy tylko R, G, B (bez W na koncu)
        // R=50, G=40, B=0 daje ladny cieply zolty/bialy
        led_strip_set_pixel(led_strip, i, 50, 40, 0); 
        
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(ANIMATION_SPEED_MS));
    }
    ESP_LOGI(TAG, "Swiatlo zapalone w 100%%");
}

// Funkcja gaszaca swiatlo (Animacja: Zwijanie 60 -> 0)
// Dzieki temu wyglada to profesjonalnie - swiatlo "waca" do czujnika
void light_off() {
    ESP_LOGI(TAG, "Animacja STOP <-");
    for (int i = LED_COUNT - 1; i >= 0; i--) {
        // ZMIANA: Tu tez usuwamy czwarte zero
        led_strip_set_pixel(led_strip, i, 0, 0, 0); 
        
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(ANIMATION_SPEED_MS));
    }
    led_strip_clear(led_strip);
    ESP_LOGI(TAG, "Swiatlo zgaszone");
}

void configure_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_PIN,
        .max_leds = LED_COUNT,
        .led_pixel_format = LED_PIXEL_FORMAT_GRBW, // Twoj format RGBW
        .led_model = LED_MODEL_SK6812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

void configure_radar(void)
{
    // Konfigurujemy Pin 14 jako WEJSCIE (INPUT)
    gpio_reset_pin(RADAR_PIN);
    gpio_set_direction(RADAR_PIN, GPIO_MODE_INPUT);
    // Wlaczamy rezystor sciagajacy do masy (zeby nie "plywalo" jak radar milczy)
    gpio_set_pull_mode(RADAR_PIN, GPIO_PULLDOWN_ONLY);
    ESP_LOGI(TAG, "Radar skonfigurowany na pinie %d", RADAR_PIN);
}

void app_main(void)
{
    configure_led();
    configure_radar();
    ESP_LOGI(TAG, "START SYSTEMU! Machnij reka przed czujnikiem.");

    int last_state = -1; // Zapamietujemy stan, zeby nie spamowac logami

    while (1) {
        // Odczyt stanu radaru (0 lub 1)
        int radar_state = gpio_get_level(RADAR_PIN);

        if (radar_state == 1) {
            // RUCH WYKRYTY
            if (last_state != 1) {
                ESP_LOGI(TAG, "Ruch wykryty! Wlaczam swiatlo.");
                light_on();
                last_state = 1;
            }
        } else {
            // BRAK RUCHU
            if (last_state != 0) {
                ESP_LOGI(TAG, "Brak ruchu. Gasze swiatlo.");
                light_off();
                last_state = 0;
            }
        }

        // Krotka przerwa dla stabilnosci (100ms)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}