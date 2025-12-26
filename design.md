# Projekt: Inteligentny Sterownik Oświetlenia Przejścia

## 1. Cel projektu
Stworzenie sterownika oświetlenia liniowego LED, który uruchamia animację świetlną zgodnie z kierunkiem poruszania się osoby (L->P lub P->L).

## 2. Założenia architektury
Projekt realizowany na platformie ESP32 z wykorzystaniem frameworka ESP-IDF (FreeRTOS).

### Decyzje sprzętowe:
- **Sensory:** LD2410b w trybie GPIO. Wybrano ten tryb ze względu na deterministyczność sygnału i minimalizację opóźnień w pętli sterowania.
- **Aktuator:** Adresowalne diody LED sterowane przez peryferium RMT (Remote Control Transceiver) w celu odciążenia CPU.

### Warstwy oprogramowania:
1. **HAL (Hardware Abstraction Layer):** Izolacja sprzętu (odczyt GPIO, wysyłka RMT).
2. **Logic/FSM:** Maszyna stanów zarządzająca logiką aplikacji.
3. **Main:** Inicjalizacja i pętla główna.