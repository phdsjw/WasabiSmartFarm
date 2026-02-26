# AGENTS.md

This file guides agentic coding assistants working in this repository.
It focuses on build/run commands and the code style actually used here.

## Scope and structure
- Project root for source/docs: `WasabiSmartFarm-main/`
- Arduino firmware lives under `arduino/`
- Node-RED flows and settings live under `nodered/`
- Home Assistant related materials live under `Home_assistant/`
- Docs/specs live under `docs/` and top-level `*.md`

## Existing agent rules
- No Cursor rules found (`.cursor/rules/` or `.cursorrules`)
- No Copilot rules found (`.github/copilot-instructions.md`)

## Build, run, lint, test
This repo is primarily Arduino firmware + Node-RED flows. There are no
repo-level scripts for lint/test/build in a package.json here.

### Arduino firmware (manual build/flash)
- Open the target `.ino` in Arduino IDE 2.x and upload to board
  - Example: `arduino/air_sensor_node/air_sensor_node.ino`
- Board: Arduino Uno R4 WiFi (per README)
- Libraries used across sketches (install in Arduino IDE):
  - WiFiS3, PubSubClient, ArduinoModbus, ArduinoRS485
  - OneWire, DallasTemperature, Adafruit_SHT31, ArduinoJson
- If using Arduino CLI (not configured in repo), typical pattern is:
  - `arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi <sketch>`
  - `arduino-cli upload -p <port> --fqbn arduino:renesas_uno:unor4wifi <sketch>`

### Node-RED (run/import flows)
- Run Node-RED (per docs): `node-red`
- Flow files in repo: `nodered/flows*.json`
- Settings template: `nodered/settings_fixed.js`
- Typical flow import: copy desired flow JSON to your Node-RED user dir
  (see `WASABI_SMARTFARM_SETTING.md` for Windows and Linux paths)

### MQTT broker checks (manual verification)
- Use Mosquitto tools to verify topics while devices run:
  - `mosquitto_sub -v -t "sensor/air/+/data"`
  - `mosquitto_sub -v -t "sensor/+/+/heartbeat"`
- These are the closest equivalents to a “single test” in this repo

### Lint/tests
- No lint configuration found (no .eslintrc, .prettierrc, etc.)
- No automated test runner found (no jest/vitest/pytest configs)
- When changing code, rely on:
  - Arduino IDE compile + serial monitor verification
  - Node-RED runtime validation + MQTT topic observation

### Manual validation ideas
- Sensor node check: monitor the expected topics while device runs
  - `mosquitto_sub -v -t "sensor/air/+/data"`
  - `mosquitto_sub -v -t "sensor/+/+/heartbeat"`
- Basic publish sanity (per docs): `mosquitto_pub -h localhost -t test/topic -m "Hello MQTT"`
- Node-RED health: start `node-red` and ensure flows load without errors

## Coding style conventions (observed)
These are based on existing Arduino firmware and Node-RED settings files.

### Arduino C++ / .ino style
- Indentation: 2 spaces
- Braces: K&R style (opening brace on same line)
- Section separators:
  - Use `// ============================================` for major sections
- File headers:
  - Block comment with component name, author, version, date
- Includes:
  - Arduino/3rd-party headers first, then local headers
  - Example order in `mqtt_handler.h`:
    `Arduino.h`, `WiFiS3.h`, `PubSubClient.h`, `ArduinoJson.h`, then `config.h`
- Constants/config:
  - `#define` with UPPER_SNAKE_CASE
  - Group related defines under section separators
- Naming:
  - Classes: `PascalCase` (e.g., `MQTTHandler`)
  - Methods/functions: `lowerCamelCase` (e.g., `publishSensorData`)
  - Struct fields: `snake_case` (e.g., `air_temp`, `is_valid`)
  - Globals: `lowerCamelCase` with descriptive names
- Logging:
  - Use `Serial.print`/`Serial.println` with `F()` macro for string literals
  - Prefix log lines with tags like `[SETUP]`, `[SENSOR]`, `[ERROR]`
  - Debug logging uses `DEBUG_PRINT` / `DEBUG_PRINTLN` macros from `config.h`
- Error handling:
  - Fail-fast on unrecoverable init failures (infinite LED blink loop)
  - Retry with bounded loops for WiFi/MQTT reconnects
- Timing:
  - Use `millis()`-based intervals for periodic work
  - Short `delay()` calls are acceptable for LED/status pacing
- Memory:
  - Use `StaticJsonDocument<...>` sized to payload
  - Avoid dynamic allocation in tight loops

### Header files (.h)
- Use include guards with UPPER_SNAKE_CASE
- Keep private members first, then public methods
- Prefer forward declarations when possible, but existing files include full deps

### Node-RED config / JS files
- Settings files use 4-space indentation (see `nodered/settings_fixed.js`)
- Keep Node-RED settings structure intact; change only necessary keys
- Prefer comments that match Node-RED default docs (already in file)

### MQTT conventions (from config headers)
- Topics use prefixes like `sensor/<type>/zone<id>/...` or `sensor/<type>/...`
- Keep topic names and IDs in `config.h` per node
- Publish JSON payloads using ArduinoJson

### Home Assistant YAML
- Indentation: 2 spaces (see `Home_assistant/config/configuration.yaml`)
- Use `!include` for modular config blocks (`mqtt.yaml`, `template.yaml`)
- MQTT entities follow a repeated field set:
  - `name`, `unique_id`, `state_topic`, `value_template`, `unit_of_measurement`
  - Optional: `device_class`, `icon`
- Keep topic names stable and consistent with Arduino publishers

### Node-RED Function node JS (in flows)
- Use `const`/`let` for variable declarations
- Keep logic small and composable per function node
- Prefer explicit checks before publish/forward actions

### JSON flows
- Keep flows JSON pretty-printed and stable for diffing
- Avoid manual edits unless necessary; prefer Node-RED editor export

## Practical do/don’t
Do:
- Keep per-node configuration in `config.h` for each Arduino node
- Reuse MQTT topic patterns and prefixes from existing config files
- Preserve section separators and header block comments for consistency
- Update docs if you change wiring, pinouts, or operational procedures

Don’t:
- Introduce new formatting tools without adding config files
- Reformat Node-RED settings or flows without a functional change
- Add secret values directly into `config.h` in committed changes

## When adding new nodes or sensors
- Copy an existing node folder and adjust:
  - `config.h` (ZONE_ID, WiFi, MQTT topics, timing)
  - Sensor-specific modules (`*_sensor.cpp/.h`)
  - MQTT payload fields to match existing naming
- Keep version/date headers in the main `.ino` updated
- Validate with MQTT topic monitor and serial logs

## What is not present
- No repo-local lint/format config (no ESLint/Prettier/clang-format/yamllint files)
- No automated test runner in the repo root

## Key references
- `README.md` (project overview and environment setup)
- `WASABI_SMARTFARM_SETTING.md` (Node-RED/Mosquitto setup details)
- `arduino/air_sensor_node/air_sensor_node.ino` (style reference)
- `arduino/air_sensor_node/config.h` (constants + debug macros)
- `nodered/settings_fixed.js` (Node-RED settings baseline)
