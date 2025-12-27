#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_timer.h"

// --- KONFIGURACJA ---
#define LED_PIN         13
#define RADAR_PIN_L     14
#define RADAR_PIN_R     27
#define LED_COUNT       60
#define ANIM_SPEED_MS   20

static const char *TAG = "APP_CORE";
static led_strip_handle_t led_strip;

// Struktura do liczenia telemetrii
typedef struct {
    uint32_t trigger_count_l;   // Licznik wzbudzeń lewych
    uint32_t trigger_count_r;   // Licznik wzbudzeń prawych
    uint32_t errors;            // Licznik błędów
    int64_t last_trigger_time;  // Czas ostatniej akcji
} app_stats_t;

static app_stats_t g_stats = {0};

// Maszyna stanów (FSM)
typedef enum {
    STATE_INIT,     // Inicjalizacja
    STATE_IDLE,     // Czuwanie (Ciemno)
    STATE_ACTIVE_A, // Animacja L -> P
    STATE_ACTIVE_B, // Animacja P -> L
    STATE_HOLD,     // Podtrzymanie (Świeci)
    STATE_FAULT     // Błąd (np. awaria paska)
} app_state_t;

static app_state_t g_current_state = STATE_INIT;

// Funkcje sterowania LED
void led_init(void) {
    led_strip_config_t strip_conf = {
        .strip_gpio_num = LED_PIN,
        .max_leds = LED_COUNT,
        .led_pixel_format = LED_PIXEL_FORMAT_GRBW,
        .led_model = LED_MODEL_SK6812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_conf = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    if (led_strip_new_rmt_device(&strip_conf, &rmt_conf, &led_strip) != ESP_OK) {
        ESP_LOGE(TAG, "LED Driver Init Failed!");
        g_current_state = STATE_FAULT;
    }
}

void anim_wipe(int direction) { // 0: L->P, 1: P->L
    int start = (direction == 0) ? 0 : LED_COUNT - 1;
    int end   = (direction == 0) ? LED_COUNT : -1;
    int step  = (direction == 0) ? 1 : -1;

    for (int i = start; i != end; i += step) {
        led_strip_set_pixel(led_strip, i, 50, 40, 0);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(ANIM_SPEED_MS));
    }
}

void led_off() {
    led_strip_clear(led_strip);
}

// --- LOGIKA APLIKACJI ---
void print_telemetry() {
    ESP_LOGI(TAG, "--- TELEMETRIA ---");
    ESP_LOGI(TAG, "Triggers L: %lu | R: %lu", g_stats.trigger_count_l, g_stats.trigger_count_r);
    ESP_LOGI(TAG, "System State: %d | Errors: %lu", g_current_state, g_stats.errors);

    int64_t now = esp_timer_get_time();
    double seconds_ago = (now - g_stats.last_trigger_time) / 1e6; // mikrosekundy -> sekundy
    if (g_stats.last_trigger_time != 0) {
        ESP_LOGI(TAG, "Last Trigger: %.1f s ago", seconds_ago);
    } else {
        ESP_LOGI(TAG, "Last Trigger: never");
    }

    ESP_LOGI(TAG, "------------------");
}

void app_main(void) {
    // 1. INIT PHASE
    g_current_state = STATE_INIT;
    ESP_LOGI(TAG, "FSM: STATE_INIT");
    
    led_init();
    
    // Config GPIO Radars
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RADAR_PIN_L) | (1ULL << RADAR_PIN_R),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 1, // Pull-down wymagany dla LD2410
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    if (g_current_state != STATE_FAULT) {
        g_current_state = STATE_IDLE;
        ESP_LOGI(TAG, "FSM: STATE_IDLE (System Ready)");
    }

    int last_L = 0;
    int last_R = 0;
    int64_t timer_telemetry = 0;

    while (1) {
        // Telemetria co 10 sekund
        if (esp_timer_get_time() - timer_telemetry > 10000000) {
            print_telemetry();
            timer_telemetry = esp_timer_get_time();
        }

        // Odczyt wejść
        int curr_L = gpio_get_level(RADAR_PIN_L);
        int curr_R = gpio_get_level(RADAR_PIN_R);

        // Wykrycie zboczy (Rising Edge)
        int edge_L = (curr_L == 1 && last_L == 0);
        int edge_R = (curr_R == 1 && last_R == 0);

        // --- MASZYNA STANÓW (FSM) ---
        switch (g_current_state) {
            case STATE_IDLE:
                // Czekamy na sygnał
                if (edge_L && !edge_R) {
                    g_current_state = STATE_ACTIVE_A;
                    g_stats.trigger_count_l++;
					g_stats.last_trigger_time = esp_timer_get_time();
                } 
                else if (edge_R && !edge_L) {
                    g_current_state = STATE_ACTIVE_B;
                    g_stats.trigger_count_r++;
					g_stats.last_trigger_time = esp_timer_get_time();
                }
                else if (edge_L && edge_R) {
                    // Konflikt -> Priorytet A
                    g_current_state = STATE_ACTIVE_A;
                    g_stats.trigger_count_l++;
                    g_stats.errors++; // Konflikt obu wejść – liczymy jako błąd kolizji
					g_stats.last_trigger_time = esp_timer_get_time();
                }
                break;

            case STATE_ACTIVE_A:
                ESP_LOGI(TAG, "FSM: ACTIVE_A (Wipe Right)");
                anim_wipe(0);
                g_current_state = STATE_HOLD;
                break;

            case STATE_ACTIVE_B:
                ESP_LOGI(TAG, "FSM: ACTIVE_B (Wipe Left)");
                anim_wipe(1);
                g_current_state = STATE_HOLD;
                break;

            case STATE_HOLD:
                // Czekamy aż oba radary zgasną
                if (curr_L == 0 && curr_R == 0) {
                    ESP_LOGI(TAG, "FSM: HOLD -> IDLE (Wygaszanie)");
                    led_off();
                    g_current_state = STATE_IDLE;
                }
                break;

            case STATE_FAULT:
                ESP_LOGE(TAG, "CRITICAL FAULT - Check Wiring");
                vTaskDelay(pdMS_TO_TICKS(1000)); // Migaj błędem
                break;
                
            default:
                g_current_state = STATE_IDLE;
        }

        last_L = curr_L;
        last_R = curr_R;
        vTaskDelay(pdMS_TO_TICKS(50));  // Odczekaj chwilę, aby nie obciążać CPU
    }
}