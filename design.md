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


📅 Log Projektu: Konfiguracja Środowiska (Hello World)
Data: 26.12.2025 Cel: Uruchomienie Toolchaina ESP-IDF i wgranie pierwszego programu na mikrokontroler ESP32.

1. Część Oficjalna (Techniczna)
(To sekcja do wykorzystania w sprawozdaniu/dokumentacji końcowej)

Środowisko:

IDE: Visual Studio Code

Framework: ESP-IDF v5.4.3 (Release)

Hardware: ESP32-WROOM-32 DevKit V1

System: Windows 10/11

Przebieg prac:

Zainstalowano rozszerzenie ESP-IDF oraz sterowniki USB-UART.

Skonfigurowano projekt w oparciu o szablon hello_world / sample_project.

Dokonano kompilacji kodu (Build) – wygenerowano plik binarny .bin (rozmiar ok. 160KB).

Rozwiązane problemy techniczne:

Błąd PermissionError(13) na porcie COM:

Diagnoza: Konflikt dostępu do portu szeregowego (blokada przez procesy w tle lub system operacyjny).

Rozwiązanie: Restart hosta, manualny wybór portu w idf.py, wyczyszczenie sesji terminala.

Błąd Wrong boot mode (0x13):

Diagnoza: Układ zabezpieczający (auto-reset circuit) na płytce ewaluacyjnej nie wprowadził mikrokontrolera w stan Bootloadera automatycznie.

Rozwiązanie: Manualne wymuszenie trybu Download Mode poprzez przytrzymanie przycisku BOOT (GPIO0 -> GND) w momencie inicjacji połączenia.

Weryfikacja: Po poprawnym wgraniu wsadu (Flash), uruchomiono monitor portu szeregowego (Monitor). Potwierdzono poprawne działanie pętli głównej app_main poprzez odczyt logów systemowych ("Licznik: ...").

