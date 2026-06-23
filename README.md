# AoT-Diorama (Titan Diorama) - ESP32 Controller

[Deutsch](#deutsch) | [English](#english)

---

## Deutsch

Dieses Projekt implementiert eine voll funktionsfähige, asynchrone und ereignisgesteuerte Steuerung für ein interaktives Diorama (Attack on Titan Thema). Das System basiert auf einem **ESP32-WROOM-32E** und wird über das **PlatformIO**-Framework (Arduino) programmiert.

### Features
*   **Asynchrone Software-Architektur:** Vollständig blockierungsfreier Betrieb unter Verwendung von Zustandsautomaten (State Machine) auf Basis von `millis()`, `ESPAsyncWebServer` und `WebSockets` für Echtzeit-Kommunikation.
*   **Web-Dashboard:** Ein integriertes, CDN-freies Webinterface, das live Strommesskurven (Skala 0–500 mA), Systemstatus-Meldungen, manuelle Teststeuerungen ("Tippbetrieb" für Motoren, Relais und LED-Bereiche) sowie Einstellungs-Konfigurationen bereitstellt.
*   **Präzise LED-Steuerung:** Verwendung der **FastLED**-Bibliothek zur getrennten Helligkeitsregelung von 7 LED-Segmenten des Haupt-Streifens (226 LEDs) und einer Status-LED.
*   **Motorsteuerung mit H-Brücke:**
    *   **Motor 1 (Linear-Schlitten):** Reziproke Bewegung mit einstellbarer Geschwindigkeit und Strombegrenzung.
    *   **Motor 2 (Linearaktuator):** Endlagen- und Blockierungserkennung rein über Strommessung (Stall-Detektion) zur Vermeidung mechanischer Defekte (keine Endschalter notwendig).
    *   **Sicherheits-Heartbeat:** Im Tippbetrieb stoppen die Motoren automatisch, wenn das Dashboard länger als 800 ms kein Steuersignal sendet.
*   **Over-The-Air (OTA) Updates & mDNS:** Drahtloses Flashen im lokalen Netzwerk und Erreichbarkeit unter `titandiorama.local`.

### Hardware-Belegung (GPIO Pins)
*   **GPIO26:** Externe Stromversorgung aktivieren (`VCC_EN`, HIGH = Aktiv)
*   **GPIO02:** Status-LED (1x WS2812B)
*   **GPIO13:** Haupt-LED-Streifen (226x WS2812B, aufgeteilt in 7 Segmente)
*   **Motor 1:**
    *   **GPIO16:** PWM / Richtung 1
    *   **GPIO25:** PWM / Richtung 2
    *   **GPIO34 (ADC):** Strommessung
*   **Motor 2:**
    *   **GPIO27:** PWM / Richtung 1
    *   **GPIO32:** PWM / Richtung 2
    *   **GPIO36 (ADC):** Strommessung
    *   **GPIO17:** Physischer Endlagenschalter (optional, intern auf PULLUP gesetzt)
*   **Digital-Ausgänge (Schalter / Relais):**
    *   **GPIO14 / GPIO15:** Ausgang 1 (H-Brücke)
    *   **GPIO04 / GPIO12:** Ausgang 2 (H-Brücke)

---

## English

This project implements a fully asynchronous, event-driven controller for an interactive diorama (Attack on Titan themed). The system is built on an **ESP32-WROOM-32E** MCU programmed using the **PlatformIO** framework with the Arduino core.

### Features
*   **Asynchronous Software Architecture:** Fully non-blocking state machine based on `millis()` loops, `ESPAsyncWebServer`, and `WebSockets` for real-time telemetry.
*   **Web Dashboard:** An embedded, CDN-free web interface providing live current monitoring graphs (0–500 mA scale), system state readouts, manual jogging overrides (motors, switches, LED segments), and configuration management.
*   **FastLED Custom Segmentation:** Direct control over 7 distinct LED segments on the main strip (226 WS2812B LEDs) with individual brightness scaling and an on-board status LED.
*   **H-Bridge Motor Control:**
    *   **Motor 1 (Horizontal Slider):** Reciprocating motion with configurable speed and safety current limit.
    *   **Motor 2 (Linear Actuator):** Homing and travel reversals detected purely via current monitoring (stall detection) to prevent mechanical failures.
    *   **Safety Heartbeat:** Manual jogging automatically halts the motors if no dashboard heartbeat is received within 800 ms.
*   **Over-The-Air (OTA) Updates & mDNS:** Network flashing capabilities and accessibility via `titandiorama.local`.

### Hardware Pin Mapping
*   **GPIO26:** External components VCC power enable (`VCC_EN`, active HIGH)
*   **GPIO02:** Status LED (1x WS2812B)
*   **GPIO13:** Main LED Strip (226x WS2812B, divided into 7 segments)
*   **Motor 1:**
    *   **GPIO16:** PWM / Direction 1
    *   **GPIO25:** PWM / Direction 2
    *   **GPIO34 (ADC):** Current sensing
*   **Motor 2:**
    *   **GPIO27:** PWM / Direction 1
    *   **GPIO32:** PWM / Direction 2
    *   **GPIO36 (ADC):** Current sensing
    *   **GPIO17:** Homing Limit Switch (optional, internal Pullup, Active LOW)
*   **Digital Outputs (Switches / Relays):**
    *   **GPIO14 / GPIO15:** Switch Output 1 (H-Bridge)
    *   **GPIO04 / GPIO12:** Switch Output 2 (H-Bridge)

---

## Installation / Build (PlatformIO)

### Prerequisites
1. Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO IDE extension](https://platformio.org/).
2. Connect your ESP32 to the PC via USB.

### Compiling & Flashing
```bash
# Compile the project
pio run

# Upload the firmware via serial port
pio run --target upload

# Open the serial monitor (115200 baud)
pio device monitor
```

### Over-The-Air (OTA) Upload
Once the ESP32 is connected to your WiFi network, you can upload updates wirelessly:
```bash
pio run --target upload --upload-port titandiorama.local
```
