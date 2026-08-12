# AGENTS.md

In-vehicle instruments firmware for a Greenpower electric car. PlatformIO project targeting an Arduino Mega 2560 (ATmega2560: 8 KB RAM / 248 KB flash), Arduino framework, C++17 (`-std=gnu++17 -O2`).

## Build & verify
- Build: `pio run`
- Upload: `pio run -t upload`
- Serial monitor: `pio device monitor` (115200 baud)
- No tests or CI exist; a successful `pio run` is the verification step.

## Layout
- `src/` — the firmware; the only directory PlatformIO compiles.
- `src/main.cpp` — declares the `ScheduledTask tasks[]` array and `setup()`/`loop()`.
- `src/modules/*` — one static class per device (BMP, ATH, RTC, IMU, VESC, SPEED, TEMPS, EEPROM, Display). Pattern: `init(...)`, `update()`/`run()`, `isEnabled()`, an `IntervalMetric dataProcessTime` member; writes results into `SensorState` members in `src/state.h`; reports failures via `Errors::logError(Error::...)`.
- `src/utils/` — scheduler, metrics, error log, logging, battery.
- `src/hardware.cpp/.h` — pin map and bus init.
- `demo/` — standalone one-off sketches plus vendored library examples; NOT compiled by the project.
- `circuitry/` — KiCad schematics, not code.

## Gotchas
- Init order in `HARDWARE::init()` matters: all I2C devices must init before `Wire.setClock(400000)`. Several modules are commented out there and in `main.cpp` — that is the current WIP state, not an error.
- The `DEBUG_LOGGING` build flag (platformio.ini) enables `Logging::logDebug` prints AND adds a 3-second boot delay in `HARDWARE::init()`. Don't mistake the delay for a hang.
- Scheduler: `Timer1` emits a 1 ms tick; `ScheduledTask(period, initialOffset, fn)` tasks run in `loop()` via `Scheduler::runTasks()`, never in the ISR. The initial offset staggers tasks within a period.
- Settings persist to an external I2C EEPROM (`src/settings.cpp`), validated by version + Fletcher-16 checksum. If you modify `SettingsBlock` layout, bump `SETTINGS_BLOCK_VERSION` in `settings.h`.
- Shared state lives as `static inline` members (C++17) on header-only structs/classes (`SensorState`, module `enabled` flags). With 8 KB RAM, avoid dynamic allocation.
- Serial: `Serial2` → VESC/ESC (250000 baud per `README.md`), `Serial3` → GPS. SPI (50/51/52) is shared by display, touchscreen, and SD card via separate CS pins.
- VESC controller settings in `README.md` are hardware config; leave them unless explicitly asked.
- `compile_commands.json` is gitignored but generated locally for clangd (the configured IDE engine; C_Cpp is disabled). `.clang-format` is Google style with `ColumnLimit: 0`.
- Commits use conventional style (`feat:`, `fix:`).
