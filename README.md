# PPN Smart Light System (ESP32 + mmWave Radar)
![Demo działania systemu](demo.gif)

Projekt semestralny: Sterownik oświetlenia schodowego/korytarzowego z detekcją kierunku ruchu i obecności.

## Funkcjonalności
* **Dwukierunkowa detekcja:** Rozróżnianie kierunku wejścia (Lewo -> Prawo / Prawo -> Lewo) przy użyciu dwóch niezależnych sensorów.
* **Płynne animacje:** Efekt "Wipe" (rozwijanie paska) realizowany na taśmie adresowalnej SK6812 (RGBW).
* **Architektura FSM (Maszyna Stanów):**
  - **IDLE:** Czuwanie (minimalne zużycie energii).
  - **ACTIVE:** Wykonywanie animacji wejściowej.
  - **HOLD:** Podtrzymanie światła tak długo, jak wykrywana jest obecność (mikroruchy).
* **Telemetria UART:** Cykliczne raportowanie liczby wzbudzeń, błędów kolizji i stanu systemu.

## Sprzęt (Hardware)

### 1. Mikrokontroler
* **ESP32 DevKit V1** (Obsługa logiki, RMT, UART)

### 2. Sensory (Radary mmWave)
* **Model:** 2x HLK-LD2410B (24GHz)
* **Tryb pracy:** GPIO Output (High/Low) z aktywnym Pull-Down.
* **Konfiguracja (via HLKRadarTool App):**
  - **Zasięg (Gate):** Zmieniono rozdzielczość detekcji (Detection Resolution) na 0.2m. Radar działa w trybie bliskiego zasięgu (Gesture Mode), ignorując obiekty oddalone o więcej niż 20-30 cm.
  - **Czułość:** Dostosowana do wykrywania mikroruchów (oddech/siedzenie).
  - **Unmanned Duration:** 5s (sprzętowe podtrzymanie stanu wysokiego).
* **Pinout:**
  - Sensor L: GPIO 14
  - Sensor R: GPIO 27

### 3. Aktuator
* **Taśma LED:** SK6812 RGBW (Neutral White)
* **Sterowanie:** GPIO 13 (ESP32 RMT Peripheral)

## Logika Działania (Algorytm)

System wykorzystuje logikę opartą o stany i detekcję zbocza, aby wyeliminować typowe problemy czujników PIR:

1. **Detekcja Zbocza (Rising Edge):** Animacja wyzwalana jest tylko w momencie *zmiany* stanu z 0 na 1. Zapobiega to wielokrotnemu restartowaniu animacji, gdy osoba stoi w progu.
2. **Priorytetyzacja Kierunku:** W przypadku jednoczesnego wejścia z obu stron (Race Condition), system programowo obsługuje konflikt, nadając priorytet kierunkowi A i logując zdarzenie jako "Soft Error".
3. **Stan HOLD:** Po zakończeniu animacji, system przechodzi w stan podtrzymania. Światło gaśnie dopiero wtedy, gdy oba radary sprzętowo potwierdzą brak obecności (stan niski na obu liniach GPIO).

## Uruchomienie

1. Skonfiguruj środowisko **ESP-IDF** (v5.4.3 lub nowsze).
2. Sklonuj repozytorium i przejdź do katalogu głównego.
3. Zbuduj projekt:
   ```bash
   idf.py build
   ```

4. Wgraj na urządzenie i uruchom monitor:
   ```bash
   idf.py -p COMx flash monitor
   ```


## Dokumentacja i Testy

Szczegółowy opis architektury oraz raport z testów znajdują się w plikach:

- design.md - Opis architektury FSM i decyzji projektowych.
    
- tests.md - Protokół testów funkcjonalnych (Scenariusze TC-01 do TC-05).
    

## Autor

Marcin Lesiak

