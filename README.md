# SensorStream

Register sensors by **name + unit**, push `float` values, and the library streams them over BLE in a compact, self-describing packet format. A central (PC, phone, the Python/Bleak reader) reads one metadata characteristic and learns every sensor's id, name and unit at runtime — no hardcoded id tables on either side.

// TODO Python code will still be programmed and explained

## Install

**Inside this project** it already lives in `lib/SensorStream/`, so PlatformIO
compiles it automatically — nothing to do.

**In another PlatformIO project**, add it to `platformio.ini`:

```ini
lib_deps = https://github.com/ISellyouTACOS/SensorStream.git
```

NimBLE-Arduino is pulled in automatically (declared in `library.json`).

## Use

```cpp
#include <SensorStream.h>

uint8_t id_temp, id_accel;

void setup() {
  id_temp  = registerSensor("temp", "C");      // call once per sensor
  id_accel = registerSensor("accel_x", "m/s2");
  bleSetup("sensor-name");                    // start advertising under this name
}

void loop() {
  addRecord(id_temp,  readTemp());             // push values; sent over BLE
  addRecord(id_accel, readAccelX());
}
```

That's the whole API:

| Function | Purpose |
| --- | --- |
| `uint8_t registerSensor(name, unit = "")` | Reserve a free id for a sensor. Returns the id (or `0xFF` if full). Call once in `setup()`, before `bleSetup()`. |
| `void bleSetup(name = "Mark esp")` | Initialise BLE, publish the sensor registry, start advertising under `name`. Call after all `registerSensor` calls. Give each ESP a distinct name so a central can tell several nodes apart. |
| `void addRecord(uint8_t id, float value)` | Buffer one measurement. Records sharing a millis timestamp share one timestamp on the wire; the buffer is flushed over BLE automatically. |

## Multiple ESPs at once

All nodes keep the **same** service/characteristic UUIDs — that is what lets one
central discover them all by scanning for the service. Devices are told apart by
their BLE address and by the **name** you pass to `bleSetup()`, so just give each
node its own:

```cpp
bleSetup("sensor-node-A");// on ESP #1
bleSetup("sensor-node-B");// on ESP #2
```

One PC connects to several nodes at the same time.

## Wire format & decoding

The stream is a sequence of 5-byte chunks (`1 tag byte + 4 payload bytes`); tag `0xFF` carries a timestamp, any other tag is a sensor id whose payload is a little-endian
`float32`.

## Limits

- Up to `MAX_SENSORS` (28) sensors. The whole registry must fit in one BLE
  characteristic value (max 512 bytes), so names are capped at 8 chars and units
  at 6 chars (longer ones are truncated), and 28 is what fits in that budget.
- `0xFF` is reserved as the timestamp marker and is never assigned as an id.
- One BLE central at a time.
