# Projekt: Inteligentny Sterownik Oświetlenia Przejścia

## 1. Cel projektu
Celem projektu jest stworzenie sterownika oświetlenia LED, który włącza animację świetlną w zależności od kierunku ruchu osoby (Lewo -> Prawo lub Prawo -> Lewo).

## 2. Założenia architektury
Projekt działa na ESP32 z użyciem frameworka ESP-IDF (FreeRTOS).

### Sprzęt
- **Sensory:** LD2410B w trybie GPIO – wybrano ten tryb, bo jest prosty w obsłudze i szybki w działaniu.  
- **LED:** Taśma adresowalna SK6812 sterowana modułem RMT w ESP32 – odciąża procesor przy sterowaniu dużą liczbą diod.

### Warstwy oprogramowania
1. **HAL (Hardware Abstraction Layer):** Obsługa sprzętu – odczyt GPIO, wysyłanie danych do LED.  
2. **Logic/FSM:** Maszyna stanów, która decyduje, co ma się dziać w danym momencie.  
3. **Main:** Inicjalizacja i główna pętla programu.

## 3. Log Projektu: Konfiguracja środowiska i pierwsze uruchomienie
**Data:** 26.12.2025  
**Cel:** Uruchomienie ESP32 i pierwszego programu testowego.

- Środowisko: Visual Studio Code + ESP-IDF v5.4.3  
- Płytka: ESP32-WROOM-32 DevKit V1  
- System: Windows 10  

### Przebieg prac
1. Zainstalowano rozszerzenie ESP-IDF i sterowniki USB-UART.  
2. Skonfigurowano projekt w oparciu o szablon `hello_world`.  
3. Skompilowano program – wygenerowano plik `.bin` (~160 KB).  

### Napotkane problemy i rozwiązania
- **PermissionError(13)** przy otwieraniu portu COM → zablokowany przez inny proces; rozwiązanie: restart systemu i ręczne wybranie portu w `idf.py`.  
- **Wrong boot mode (0x13)** → ESP32 nie wszedł w tryb bootloadera automatycznie; rozwiązanie: przytrzymanie przycisku BOOT podczas startu.  

### Weryfikacja
Po poprawnym wgraniu programu monitor portu szeregowego pokazywał działającą pętlę `app_main` z logami „Licznik: …”.

**Status:** Sterowanie podstawowe dla LED działało poprawnie.

## 4. Etap: Podłączenie taśmy LED
1. Podłączono taśmę SK6812 (RGBW) do GPIO 13.  
2. Zainstalowano komponent `led_strip` ręcznie w folderze `components`.  
3. Napisano sterownik z użyciem RMT.  
4. Poprawiono format kolorów z `GRB` na `GRBW` (wcześniej biały kanał był przesunięty).

**Efekt:** Taśma świeci poprawnie, kanał biały działa, animacje gotowe do integracji z czujnikiem ruchu.

## 5. Etap: Sterowanie obecnością (Single Radar)
### Warstwa sprzętowa
- Radar HLK-LD2410B podłączony do ESP32 w trybie GPIO.  
- Wejście sygnału: GPIO 14 (Pull-Down).  
- Zasilanie: 5V z płytki.

### Kalibracja sensora
- Ograniczono zasięg detekcji do ~1,5 m (Gate 2), aby uniknąć fałszywych wykryć od ścian lub mebli.  
- Ustawiono czułość na wykrywanie ruchów drobnych (np. ręka, oddech).  

### Algorytmika oświetlenia
- Animacja LED działa stopniowo – kolejne diody zapalają się po sobie (efekt „Wipe”).  
- Animacja uruchamiana jest tylko przy zmianie stanu sensora z LOW → HIGH.  
- Światło podtrzymywane jest w stanie HOLD, dopóki sensor wykrywa obecność.

**Efekt:** Płynne włączanie i wygaszanie światła, lepszy komfort wizualny.

## 6. Architektura systemu

### FSM (Maszyna Stanów)
1. **INIT:** Konfiguracja GPIO i RMT.  
2. **IDLE:** Oczekiwanie na wejście w strefę (Rising Edge).  
3. **ACTIVE_A / ACTIVE_B:** Animacja w odpowiednim kierunku.  
4. **HOLD:** Podtrzymanie światła – system ignoruje nowe wejścia dopóki oba sensory nie wrócą do LOW.

### Rozwiązanie problemów
- **Race Condition:** Jeśli oba sensory wykryją ruch w tym samym momencie, priorytet ma wejście A (Lewo).  
- **Telemetria:** Liczba wyzwalaczy i błędów zliczana i raportowana przez UART.
