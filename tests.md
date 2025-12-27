# Protokół Testowy (Test Report)

**Projekt:** PPN Smart Light System
**Platforma:** ESP32 + 2x LD2410B
**Data raportu:** 27.12.2025
**Wersja:** v1.0.0 (MVP)

## 1. Zakres i Metodyka Testów

Projekt był rozwijany w modelu iteracyjnym. Testy przeprowadzano na trzech poziomach:

1.  **Poziom Sprzętowy (Unit Tests):** Weryfikacja poszczególnych modułów (LED, Radar) w izolacji.
2.  **Poziom Integracyjny:** Łączenie modułów (np. Radar + LED) i weryfikacja ich współpracy.
3.  **Poziom Systemowy (Acceptance Tests):** Testy gotowego urządzenia z pełną logiką FSM.

Niniejszy dokument podsumowuje kluczowe testy rozwojowe oraz przedstawia szczegółowe scenariusze testowe dla finalnej wersji systemu.

---

## 2. Podsumowanie Testów Rozwojowych (Development Log)

W fazie budowy prototypu wykonano następujące testy weryfikacyjne:

| Moduł | Test | Wynik / Obserwacje |
| :--- | :--- | :--- |
| **LED Driver** | Obsługa barw RGBW | **Pozytywny.** Początkowo zaobserwowano błędne mapowanie kolorów (biblioteka oczekiwała 3 bajtów RGB, pasek wymagał 4 bajtów RGBW). Skorygowano funkcję sterującą RMT. |
| **Radar LD2410** | Kalibracja zasięgu | **Pozytywny.** Domyślny zasięg (6m) powodował wykrywanie ścian. W aplikacji HLKRadarTool ograniczono `Detection Accuracy` do wartości 0,2m, co wyeliminowało fałszywe wzbudzenia. |
| **Integracja** | Single Radar Logic | **Pozytywny.** Potwierdzono działanie animacji "Wipe" przy użyciu jednego czujnika na GPIO 14. |


**Uwaga:**  
Powyższe testy były wykonywane manualnie w trakcie rozwoju projektu,  
bez użycia frameworków automatyzujących testy.


---

## 3. Scenariusze Testowe Systemu (Final MVP)

Poniższe testy dotyczą wersji oddanej do oceny (Dual Radar Setup).

### TC-01: Inicjalizacja Systemu (Cold Boot)
* **Cel:** Weryfikacja poprawnej konfiguracji peryferiów po włączeniu zasilania.
* **Akcja:** Fizyczny reset urządzenia przyciskiem EN.
* **Oczekiwany rezultat:**
    * Logi UART pokazują sekwencję: `FSM: STATE_INIT` -> `STATE_IDLE`.
    * Brak błędów krytycznych (Panic/Reset loop).
    * Taśma LED pozostaje wygaszona.
* **Wynik:** ✅ POZYTYWNY

### TC-02: Detekcja Kierunku A (Lewo -> Prawo)
* **Cel:** Sprawdzenie poprawności animacji dla wejścia z lewej strony.
* **Warunki:** System w stanie IDLE.
* **Akcja:** Wejście w pole widzenia sensora L (GPIO 14).
* **Oczekiwany rezultat:**
    * Wykrycie zbocza narastającego (Rising Edge).
    * Przejście FSM: `IDLE` -> `ACTIVE_A`.
    * Animacja LED rozwija się płynnie od diody 0 do 60.
    * Inkrementacja licznika `trigger_count_l` w telemetrii.
* **Wynik:** ✅ POZYTYWNY

### TC-03: Detekcja Kierunku B (Prawo -> Lewo)
* **Cel:** Sprawdzenie poprawności animacji dla wejścia z prawej strony.
* **Warunki:** System w stanie IDLE.
* **Akcja:** Wejście w pole widzenia sensora R (GPIO 27).
* **Oczekiwany rezultat:**
    * Przejście FSM: `IDLE` -> `ACTIVE_B`.
    * Animacja LED rozwija się płynnie od diody 60 do 0.
* **Wynik:** ✅ POZYTYWNY

### TC-04: Podtrzymanie Obecności (Histereza)
* **Cel:** Weryfikacja czy światło nie gaśnie, gdy użytkownik przebywa w strefie.
* **Akcja:** Pozostanie w zasięgu czujnika po zakończeniu animacji wejściowej.
* **Oczekiwany rezultat:**
    * FSM pozostaje w stanie `STATE_HOLD`.
    * System ignoruje ponowne ruchy (brak re-triggeringu animacji).
    * Światło gaśnie dopiero po opuszczeniu strefy przez oba czujniki (stan niski na obu GPIO).
* **Wynik:** ✅ POZYTYWNY (Zgodne z parametrem `Unmanned Duration = 5s`).

### TC-05: Test Konfliktu (Race Condition & Error Handling)
* **Cel:** Weryfikacja stabilności przy jednoczesnym pobudzeniu obu wejść.
* **Akcja:** Symulowane jednoczesne wejście w obie strefy.
* **Oczekiwany rezultat:**
    * System nie ulega zawieszeniu.
    * FSM rozwiązuje konflikt priorytetem programowym (Kierunek A).
    * Licznik `errors` w telemetrii zwiększa się o 1.
* **Wynik:** ✅ POZYTYWNY

## 4. Telemetria

Przykładowy zrzut logów potwierdzający przejście testów:

```text
I (281) APP_CORE: FSM: STATE_INIT
I (291) APP_CORE: FSM: STATE_IDLE (System Ready)
I (341) APP_CORE: FSM: ACTIVE_A (Wipe Right)
I (3391) APP_CORE: FSM: HOLD -> IDLE (Wygaszanie)
I (10291) APP_CORE: --- TELEMETRIA ---
I (10291) APP_CORE: Triggers L: 1 | R: 0
I (10291) APP_CORE: System State: 1 | Errors: 0