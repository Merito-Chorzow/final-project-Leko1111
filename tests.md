# Raport z Testów Funkcjonalnych (Test Protocol)

**Projekt:** PPN Smart Light System
**Platforma:** ESP32 + 2x LD2410B
**Data testu:** 27.12.2025

## 1. Scenariusze Testowe (Test Cases)

### TC-01: Inicjalizacja Systemu
* **Cel:** Weryfikacja poprawnej konfiguracji peryferiów po restarcie.
* **Akcja:** Podłączenie zasilania / Reset przyciskiem EN.
* **Oczekiwany rezultat:**
    * Logi UART pokazują: `FSM: STATE_INIT` -> `STATE_IDLE`.
    * Taśma LED mruga raz lub pozostaje wygaszona (brak błędów sterownika RMT).
* **Wynik:** ✅ POZYTYWNY

### TC-02: Detekcja Wejścia "Lewo" (Kierunek A)
* **Cel:** Sprawdzenie animacji kierunkowej.
* **Warunki:** System w stanie IDLE.
* **Akcja:** Wejście w pole widzenia sensora L (GPIO 14).
* **Oczekiwany rezultat:**
    * Wykrycie zbocza narastającego.
    * Przejście FSM: `IDLE` -> `ACTIVE_A`.
    * Animacja LED rozwija się od diody 0 do 60.
    * Zliczenie licznika `trigger_count_l` w telemetrii.
* **Wynik:** ✅ POZYTYWNY

### TC-03: Detekcja Wejścia "Prawo" (Kierunek B)
* **Cel:** Sprawdzenie animacji kierunkowej odwrotnej.
* **Warunki:** System w stanie IDLE.
* **Akcja:** Wejście w pole widzenia sensora R (GPIO 27).
* **Oczekiwany rezultat:**
    * Przejście FSM: `IDLE` -> `ACTIVE_B`.
    * Animacja LED rozwija się od diody 60 do 0.
* **Wynik:** ✅ POZYTYWNY

### TC-04: Test Podtrzymania (HOLD State)
* **Cel:** Weryfikacja czy światło nie gaśnie, gdy użytkownik przebywa w strefie.
* **Akcja:** Pozostanie w zasięgu dowolnego czujnika po zakończeniu animacji.
* **Oczekiwany rezultat:**
    * FSM pozostaje w stanie `STATE_HOLD`.
    * Światło świeci ciągle.
    * Ponowne ruchy ręką nie wyzwalają nowej animacji (blokada re-triggeringu).
* **Wynik:** ✅ POZYTYWNY (Zgodne z konfiguracją hardware radaru: Unmanned Duration = 5s).

### TC-05: Test Konfliktu (Race Condition)
* **Cel:** Weryfikacja zachowania przy jednoczesnym wejściu z obu stron.
* **Akcja:** Jednoczesne pobudzenie GPIO 14 i GPIO 27.
* **Oczekiwany rezultat:**
    * System nie zawiesza się.
    * FSM wybiera priorytetowy kierunek (ACTIVE_A).
    * Licznik `errors` w telemetrii zwiększa się o 1.
* **Wynik:** ✅ POZYTYWNY (Logika priorytetów działa poprawnie).

## 2. Telemetria (Przykładowy zrzut logów)

```text
I (12430) APP_CORE: --- TELEMETRIA ---
I (12430) APP_CORE: Triggers L: 5 | R: 3
I (12430) APP_CORE: System State: 1 | Errors: 0
I (12430) APP_CORE: ------------------