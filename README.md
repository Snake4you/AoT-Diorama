# AoT-Diorama (Titan Diorama) - ESP32 Controller

[Deutsch](#deutsch) | [English](#english)

---

## Deutsch

> [!NOTE]
> **WICHTIGER HINWEIS:** Dieses Projekt befindet sich aktuell in der Testphase und dient als Testprogramm zur Verifizierung der Hardware-Komponenten und der Steuerungslogik.

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

### Webzugriff & Netzwerk
Standardmäßig startet der ESP32 im **Access Point (AP) Modus**:
*   **SSID (WLAN-Name):** `TitanDioramaAP`
*   **Passwort:** `12345678`
*   **IP-Adresse:** `192.168.4.1` (im Browser aufrufen)
*   **mDNS-Adresse:** `http://titandiorama.local` (falls mDNS vom Betriebssystem unterstützt wird)

Über das Web-Dashboard können auch die Anmeldedaten für das eigene Heimnetzwerk (Station Mode) hinterlegt werden. Nach erfolgreicher Verbindung ist die Weboberfläche unter der vom Heimnetzwerk zugewiesenen IP-Adresse oder ebenfalls über `http://titandiorama.local` erreichbar.

### LED-Segmentaufteilung (Gesamt: 226 WS2812B LEDs)
Die LEDs sind in sieben Segmente aufgeteilt und können unabhängig in der Helligkeit geregelt werden:
1.  **Segment 0: Lower Door** (Start: 0, Länge: 48 LEDs) – Testfarbe: Rot
2.  **Segment 1: Fire Right** (Start: 48, Länge: 16 LEDs) – Testfarbe: Orange
3.  **Segment 2: Lower Books** (Start: 64, Länge: 48 LEDs) – Testfarbe: Grün
4.  **Segment 3: Fire Left** (Start: 112, Länge: 16 LEDs) – Testfarbe: Blau
5.  **Segment 4: Upper Books** (Start: 128, Länge: 48 LEDs) – Testfarbe: Lila
6.  **Segment 5: Upper Frame** (Start: 176, Länge: 48 LEDs) – Testfarbe: Gelb
7.  **Segment 6: Titan Eye** (Start: 224, Länge: 2 LEDs) – Testfarbe: Weiß

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
*   **Digital-Ausgänge (Schalter / Relais):**
    *   **GPIO14 / GPIO15:** Ausgang 1 (H-Brücke)
    *   **GPIO04 / GPIO12:** Ausgang 2 (H-Brücke)

---

## English

> [!NOTE]
> **IMPORTANT NOTE:** This project is currently in the testing phase and serves as a test program to verify the hardware components and the control logic.

This project implements a fully asynchronous, event-driven controller for an interactive diorama (Attack on Titan themed). The system is built on an **ESP32-WROOM-32E** MCU programmed using the **PlatformIO** framework with the Arduino core.

### Features
*   **Asynchronous Software Architecture:** Fully non-blocking state machine based on `millis()` loops, `ESPAsyncWebServer`, and `WebSockets` for real-time telemetry.
*   **Web Dashboard:** An embedded, CDN-free web interface providing live current monitoring graphs (0–500 mA scale), system state readouts, manual jogging overrides (motors, switches, LED segments), and configuration management.
*   **FastLED Custom Segmentation:** Direct control over 7 distinct LED segments on the main strip (226 WS2812B LEDs) with individual brightness scaling and an on-board status LED.
*   **H-Bridge Motor Control:**
    *   **Motor 1 (Horizontal Slider):** Reciprocating motion with configurable speed and safety current limit.
    *   **Motor 2 (Linear Actuator):** Homing and travel reversals detected purely via current monitoring (stall detection) to prevent mechanical failures. No physical limit switches required.
    *   **Safety Heartbeat:** Manual jogging automatically halts the motors if no dashboard heartbeat is received within 800 ms.
*   **Over-The-Air (OTA) Updates & mDNS:** Network flashing capabilities and accessibility via `titandiorama.local`.

### Web Access & Network
By default, the ESP32 starts in **Access Point (AP) mode**:
*   **SSID (WiFi Name):** `TitanDioramaAP`
*   **Password:** `12345678`
*   **IP Address:** `192.168.4.1` (open in your web browser)
*   **mDNS Address:** `http://titandiorama.local` (if supported by your operating system)

You can also use the Web Dashboard to configure your home WiFi credentials (Station Mode). Once connected, the web interface will be accessible under the IP address assigned by your router or via `http://titandiorama.local`.

### LED Segment Mapping (Total: 226 WS2812B LEDs)
The main strip is divided into seven segments, with individual brightness controls:
1.  **Segment 0: Lower Door** (Start: 0, Length: 48 LEDs) – Test Color: Red
2.  **Segment 1: Fire Right** (Start: 48, Length: 16 LEDs) – Test Color: Orange
3.  **Segment 2: Lower Books** (Start: 64, Length: 48 LEDs) – Test Color: Green
4.  **Segment 3: Fire Left** (Start: 112, Length: 16 LEDs) – Test Color: Blue
5.  **Segment 4: Upper Books** (Start: 128, Length: 48 LEDs) – Test Color: Purple
6.  **Segment 5: Upper Frame** (Start: 176, Length: 48 LEDs) – Test Color: Yellow
7.  **Segment 6: Titan Eye** (Start: 224, Length: 2 LEDs) – Test Color: White

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
