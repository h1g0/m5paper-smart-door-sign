# M5Paper-Smart-Door-Sign

An ultra-low power, E-ink room status display for your home office. Remotely update your "Do Not Disturb" or "In a Meeting" status via Google Sheets.

## Features

* **Remote Update:** Change your status from anywhere using Google Sheets on your phone or PC.
* **Ultra-Low Power:** Utilizes M5Paper's Deep Sleep mode and E-ink display to last for weeks on a single charge.
* **Zero-Power Persistence:** The status remains visible even if the battery runs out.
* **Minimalist Design:** Clean UI with high-contrast Japanese/English font support.

## Hardware Requirements

* **M5Paper** (or M5PaperS3)
* Wi-Fi Connection
* USB-C cable for charging and programming

## System Architecture

1. **Google Sheets:** Acts as the controller. Simply type your status in a cell.
2. **Google Apps Script (GAS):** Serves the status as a JSON API.
3. **M5Paper:** Wakes up periodically, fetches the JSON via Wi-Fi, updates the E-ink screen, and goes back to sleep.

## Installation

### 1. Google Apps Script Setup

1. Create a new **Google Sheet**.
2. Type your status (e.g., `In a Meeting`) in cell **A1**.
3. Go to `Extensions` > `Apps Script` and paste the code from `gas/code.gs`.
4. Deploy as a **Web App** and set access to **"Anyone"**.
5. Copy the provided **Web App URL**.

### 2. Firmware Setup

1. Open `main.ino` in Arduino IDE.
2. Create a `config.h` file in the same tab and fill in your credentials:

```cpp
const char* SSID = "YOUR_WIFI_SSID";
const char* PASS = "YOUR_WIFI_PASSWORD";
const char* GAS_URL = "YOUR_GAS_WEB_APP_URL";
const int SLEEP_MIN = 10;

```

1. Install dependencies: `M5Unified`, `M5GFX`, and `ArduinoJson`.
2. Upload the code to your M5Paper.

## Usage

* The display will update every `SLEEP_MIN` minutes.
* To change the message, simply edit cell **A1** in your Google Sheet.
* The device uses `M5.shutdown()` to minimize power consumption between updates.

## License

MIT License
