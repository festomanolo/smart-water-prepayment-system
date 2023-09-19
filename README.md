# Smart Water Prepayment System

Firmware and tools from my August–September 2023 field work on a solar-powered **water prepayment and smart metering system** at Ngurdoto, Tanzania. The project was led by the IDIM Lab (Seoul National University) together with iTEC.

Customers tap an **RFID card**. The controller checks their credit, opens a **motor valve**, and shows the status on an **OLED** screen. Smart meters report usage over **LoRa / serial** to a base station, and a Python service stores the readings in MySQL.

## Layout

```
firmware/
├── base_station_terminal/   collects data from household meters over extra SERCOM UARTs
├── smart_meter/             current/voltage sensing, calibration, EEPROM, RF serial
├── prepayment_test_zero/    first RFID + OLED prepayment test
├── prepayment_system/       RFID card → credit check → valve control
├── oled_display_fix/        minimal RFID + SSD1306 display fix
└── prepayment_lora_sd/      prepayment unit with LoRa uplink and SD-card logging
server/
└── smartmeter_server.py     serial/GSM → MySQL logger and Excel/CSV export
docs/
├── field-notes.md           site visit notes: problems found and tools to bring
└── code-review-notes.md     review and optimisation checklist for the firmware
```

## Hardware

- Arduino Nano 33 IoT / MKR (SAMD21) and Arduino Uno-class boards
- MFRC522 RFID reader, SSD1306 128×64 OLED
- LoRa module, relay and motor valve, SD card module
- Current and voltage sensing for the smart meters

## Libraries

`MFRC522`, `Adafruit_GFX`, `Adafruit_SSD1306`, `SD`, `SoftwareSerial`, `EEPROM`.
Python: `pyserial`, `pymysql`, `pandas`, `openpyxl`, `schedule`, `python-dateutil`, `numpy`.

## Notes

This is field and prototype code, kept as it was on site. The server script connects to a local development MySQL database (`root@127.0.0.1`). Change the credentials before using it anywhere real.
