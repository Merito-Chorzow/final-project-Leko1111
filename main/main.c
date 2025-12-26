#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Tag identyfikujący logi
static const char *TAG = "APP_MAIN";

void app_main(void)
{
    // Ustawiam poziom logowania tylko dla mojego modułu
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "=== Start Systemu PPN_SmartLight ===");
    ESP_LOGI(TAG, "Wersja: MVP - GPIO Mode");

    int counter = 0;

    // TODO: W przyszłości przeniose to do osobnego Taska FreeRTOS (xTaskCreate)
    // Na razie testuja czy procesor żyje.
    while (1) {
        ESP_LOGI(TAG, "System dziala (Heartbeat). Licznik: %d", counter++);
        
        // Czekaj 1000ms (1 sekunda) - to jest nieblokujące dla CPU
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}