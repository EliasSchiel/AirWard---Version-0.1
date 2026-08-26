# AirWard — Version 0.1

AirWard is a prototype air-quality monitor built for an ESP32. It reads multiple sensors (gas sensor MQ-7, CO2 sensor MH‑Z19, and temperature/humidity AHTx0), displays values on an ILI9341 TFT, and serves the measurements via a small web server (SPIFFS-hosted UI at `/` and a JSON API at `/data`).

This repository contains the PlatformIO project and the main firmware (src/main.cpp).

---

## Quick overview

- Microcontroller: ESP32 (env: esp32dev)
- Display: ILI9341 (SPI)
- Sensors:
  - MQ‑7 (analog)
  - MH‑Z19 CO₂ (UART)
  - AHTx0 temperature & humidity (I2C)
- Web server: serves `/` (index.html from SPIFFS) and `/data` (JSON with latest readings)
- Platform: PlatformIO (see `platformio.ini`)

---

## Features

- Periodic sensor sampling (every 2 seconds)
- TFT display showing:
  - Gas sensor raw value
  - Temperature (°C)
  - Humidity (%)
  - CO₂ (ppm)
- Wi‑Fi connectivity and simple web UI (SPIFFS)
- JSON API endpoint for programmatic access

---

## Files of interest

- `src/main.cpp` — main firmware (WiFi, sensors, TFT, web server)
- `platformio.ini` — PlatformIO environment and listed library dependencies
- `Data/` — (existing) folder in the repo; for PlatformIO SPIFFS uploads create `data/` with `index.html` (see below)
- `.vscode/`, `include/`, `lib/`, `test/` — project structure folders (may be empty)

---

## Hardware wiring / pinout

The pin mapping used in `src/main.cpp`:

- TFT (ILI9341)
  - TFT_CS  -> GPIO 5
  - TFT_DC  -> GPIO 22
  - TFT_RST -> GPIO 4
- MQ‑7 (analog)
  - analog -> GPIO 34 (SENSOR_MQ7)
- MH‑Z19 (UART)
  - RX (MH‑Z19 TX) -> GPIO 16 (MHZ_RX)
  - TX (MH‑Z19 RX) -> GPIO 17 (MHZ_TX)
- AHTx0 (I2C)
  - SDA -> GPIO 13
  - SCL -> GPIO 14
- LED for test (optional)
  - PIN_LED_TEST -> GPIO 13

Notes:
- On ESP32 many pins can be remapped; the code uses Wire.begin(13, 14) to initialize I2C pins explicitly.
- Verify MQ‑7 wiring and required heater cycle/calibration before attempting to convert raw ADC readings into CO/ppm values.

---

## Software dependencies

PlatformIO libraries referenced in `platformio.ini`:
- adafruit/Adafruit GFX Library
- adafruit/Adafruit ILI9341
- adafruit/DHT sensor library

In code, additional libraries are used and must be available:
- Adafruit_AHTX0 (AHT sensor)
- MHZ19 (MH‑Z19 CO₂)

PlatformIO will auto-install libraries declared in `platformio.ini`. If you add other libraries (e.g., Adafruit AHTX0 or a specific MHZ19 package), add them to `lib_deps` or `lib/`.

---

## Setup & build

1. Clone the repo:
   git clone https://github.com/EliasSchiel/AirWard---Version-0.1.git
   cd AirWard---Version-0.1

2. Edit Wi‑Fi credentials:
   - Open `src/main.cpp`
   - Replace the placeholders for:
     const char* ssid = "Proximo a Editar";
     const char* password = "Proximo a Editar";
   or change the code to load credentials from a config header.

3. Add the web UI (SPIFFS):
   - Create a `data/` folder at the project root (PlatformIO standard).
   - Place an `index.html` file in `data/` (the firmware attempts to read `/index.html` from SPIFFS).
   - If you already have `Data/` in the repo, rename or copy its contents to `data/` so PlatformIO uses them for SPIFFS.

   Example minimal `index.html` (place in `data/index.html`):
   <html><body>
     <h1>AirWard</h1>
     <div id="readings"></div>
     <script>
       async function update(){
         let r = await fetch('/data');
         let j = await r.json();
         document.getElementById('readings').innerText = JSON.stringify(j, null, 2);
       }
       setInterval(update, 2000);
       update();
     </script>
   </body></html>

4. Build and upload firmware (PlatformIO):
   - Using VSCode + PlatformIO: open project and use PlatformIO "Build" and "Upload".
   - CLI:
     - Build: pio run -e esp32dev
     - Upload firmware: pio run -e esp32dev -t upload

5. Upload SPIFFS data (upload the `data/` folder to the device's SPIFFS):
   - pio run -e esp32dev -t uploadfs
   - If needed: pio run -e esp32dev -t buildfs then pio run -e esp32dev -t uploadfs

6. Serial monitor (optional):
   - pio device monitor -e esp32dev --baud 115200
   - If you add logging, you can print WiFi IP address from the code using Serial.println(WiFi.localIP()) after connection.

---

## Web API

- GET /         — serves `index.html` from SPIFFS (web UI)
- GET /data     — returns the latest measurements in JSON format

Sample JSON response:
{
  "mq7": 123,
  "co2": 420,
  "temperatura": 23.4,
  "humedad": 45.2
}

Example curl:
curl http://<device_ip>/data

Find the device IP either in the serial monitor (if you add an IP print) or by checking your router's connected devices.

---

## Calibration & notes

- MQ‑7 is a raw analog sensor; converting its ADC value into meaningful CO concentration requires sensor-specific calibration and heater control (MQ sensors often need pulsed heating profiles).
- MH‑Z19 CO₂ sensors require warm-up time and may need calibration/zeroing for accurate values.
- AHTx0 usually returns stable temperature & humidity but validate with a reference sensor.
- Consider adding:
  - sensor averaging and filtering
  - proper heater control for MQ‑7
  - retries and error handling for MH‑Z19
  - secure web interface (no hard-coded Wi‑Fi creds in public repos)
  - printing the device IP to serial for easier discovery

---

## Troubleshooting

- No web page:
  - Verify Wi‑Fi credentials in `src/main.cpp`.
  - Check Serial Monitor for boot logs.
  - Ensure index.html exists in `data/` and you uploaded SPIFFS (uploadfs).
- /data returns 404 or empty:
  - Verify server handlers in `src/main.cpp` — the code builds a JSON string and returns it at `/data`.
- TFT not working:
  - Double-check SPI wiring and the pin definitions (TFT_CS, TFT_DC, TFT_RST).
  - Confirm correct 3.3V supply and SPI bus configuration.
- MH‑Z19 not returning values:
  - Confirm TX/RX wiring and correct UART pins.
  - Ensure the sensor is powered and warmed up.

---

## Contributing

This is an early prototype. If you'd like to contribute:
- Open an issue describing the enhancement or bug.
- Submit a PR with clear changes and rationale.
- Consider adding a config header (e.g., `include/config.h`) for Wi‑Fi and pin customizations


