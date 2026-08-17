# AGENTS.md

In-vehicle instruments firmware for a Greenpower electric car. PlatformIO project targeting an Arduino Mega 2560 (ATmega2560: 8 KB RAM / 248 KB flash), Arduino framework, C++17 (`-std=gnu++17 -O2`).

## Build & verify

- Build: `pio run`
- Upload: `pio run -t upload`
- Serial monitor: `pio device monitor` (500000 baud; `monitor_speed` in platformio.ini)
- No tests or CI exist; a successful `pio run` is the verification step.

## Layout

- `src/` — the firmware; the only directory PlatformIO compiles.
- `src/main.cpp` — declares the `ScheduledTask tasks[]` array and `setup()`/`loop()`.
- `src/main.h` — minimal Arduino include; the single-TU header for main.
- `src/hardware.*` — `HARDWARE` class: pin setup, init order for all modules, I2C timeout handling. Pin map and setpoint thresholds live in `hardware.h`.
- `src/state.h` — the `State` struct: `static inline` members holding all shared sensor/UI state.
- `src/state.cpp` — non-trivial `State` members (currently empty, reserved).
- `src/utils/appmode.cpp` — the `AppMode` state machine (IDLE/RACE/DEBUG) on `State`, including `enterMode()`/`saveRaceState()`/`tryResumeRace()`.
- `src/settings.*` — `SettingsBlock` persisted to external I2C EEPROM (version + Fletcher-16 checksum). `SETTINGS::apply()` reconfigures `SPEED` from settings.
- `src/modules/display.*` — the display UI framework: `WidgetBase`, `Widget<T>`, `SetpointWidget<T>`, `Button`, `StaticWidget`, `TextWidget<T>`, `DebugText`, `Page`. Drives an ILI9488 via Arduino_GFX and XPT2046 touch. Touch hold-repeat support.
- `src/widgets.*` — concrete widget instances and page layouts (`RACE_PAGE`, `SETTINGS_PAGE`, `DEBUG_PAGE`) declared as `extern`s in `widgets.h`.
- `src/modules/*` — one static class per device. Pattern: `init(...)`, `update()`/`run()`, `isEnabled()`, an `IntervalMetric dataProcessTime` member; writes results into `State` members; reports failures via `Errors::logError(Error::...)`. Modules: `ath20`, `bmp280`, `rtc`, `imu`, `vesc`, `speed`, `temps`, `GPS`, `eeprom`, `display`, `battery`, `sd`. `timing.*` are empty WIP stubs.
- `src/utils/` — scheduler (`Scheduler`), metrics (`IntervalMetric`/`METRICS`), error log (`Errors`), logging (`Logging`), debug page data (`debug.*`), RAM measurement (`memory.h`). `system.*` is an empty WIP stub.
- `lib/cfg/` — config overrides for installed libraries, copied in by PlatformIO `pre` extra_scripts. Currently: `NeoGPS/` (GPSport, NMEAGPS_cfg, NeoGPS_cfg, GPSfix_cfg) via `scripts/patch_neogps.py`, and `VescUart/` (crc.cpp) via `scripts/patch_vescuart.py`.
- `demo/` — standalone one-off sketches plus vendored library examples; NOT compiled by the project.
- `circuitry/` — KiCad schematics, not code.

## Scheduler

`Timer1` emits a 1 ms tick; `ScheduledTask(period, initialOffset, fn)` tasks run in `loop()` via `Scheduler::runTasks()`, never in the ISR. The initial offset staggers tasks within a period. Current task table:

| Period | Function | Notes |
|--------|----------|-------|
| 200 ms | `measureRam` | Free RAM sample |
| 100 ms | `SPEED::update` | Optical gate speed sensor |
| 250 ms | `VESC::update` | ESC telemetry + battery estimator |
| 500 ms | `ATH::update` | Humidity sensor |
| 500 ms | `BMP::update` | Barometric pressure/temp |
| 500 ms | `TEMPS::update` | DS18B20 motor temps |
| 500 ms | `SD::logFrame` | Write one frame to SD ring buffer |
| 1000 ms | `RTC::update` | RTC + GPS time sync |
| 10 s | `State::saveRaceState` | Persist race state to EEPROM |

`loop()` also calls non-scheduled work directly: `HARDWARE::run()`, `TEMPS::run()` (polls OneWire conversion), `IMU::run()` (data-ready interrupt driven), `GPS::run()` (new-fix driven), `Display::run()` (widget refresh), `SD::run()` (drains log ring buffer).

## SD logging

SD card binary logging of all numeric `State` members during RACE mode. Format (little-endian, AVR native):

- 12-byte file header: `"INSTRLOG"` + `u16` format version + `u16` reserved.
- Fixed-size frames (`LOG_FRAME_BYTES` = 138): `u16` magic `0x4C47` + `u32` epoch (RTC seconds) + `u32` millis + 128-byte `LogPayload`.

The Python decoder lives in `main.py` (`uv run main.py`), matching the frame layout in `src/modules/sd.h` — keep them in sync. `Errors::flushToLogfile` is a TODO.

## Gotchas

- Init order in `HARDWARE::init()` matters: SPI devices first (Display, SD), then I2C devices (EEPROM, RTC, BMP, ATH, IMU), then `Wire.setClock(400000)` after all device init. A 25 ms `Wire.setWireTimeout(25000, true)` is installed; the flag is checked/cleared in `HARDWARE::run()`.
- The `DEBUG_LOGGING` build flag (platformio.ini) enables `Logging::logDebug` prints AND adds a 3-second boot delay in `HARDWARE::init()`. Don't mistake the delay for a hang.
- Settings persist to an external I2C EEPROM (`src/settings.cpp`, module `EEPROM`), validated by version + Fletcher-16 checksum; `SETTINGS_BLOCK_VERSION` is currently 3. If you modify `SettingsBlock` layout, bump the version in `settings.h`.
- Race mode persists to settings: `saveRaceState()` (a 10 s task) writes RTC-relative timestamps plus battery-estimator state; `tryResumeRace()` re-engages RACE at boot if under an hour has elapsed.
- GPS: `Serial3` via NeoHWSerial (NeoSerial3), module switched 9600 → 57600 during `GPS::init()`. NeoGPS runs in interrupt mode; the PPS pin (D2) drives `UTCsecondStart()` so `RTC::update()` can auto-sync the DS3231 (adjusts if >10 s out of sync). `rtc.h` does `#undef SECONDS_PER_DAY` to fix a collision with NeoGPS.
- Battery: `BatteryEstimator` (Peukert + temperature derate + 1-min rolling current window) is fed from VESC data inside `VESC::update()`; `State::battery_stats` is its display string.
- Shared state lives as `static inline` members (C++17) on `State` and module `enabled` flags. With 8 KB RAM, avoid dynamic allocation.
- Serial: `Serial2` → VESC/ESC (250000 baud), `Serial3` → GPS. SPI (50/51/52) is shared by display, touchscreen, and SD card via separate CS pins (53/49/48).
- VESC controller settings in `README.md` are hardware config; leave them unless explicitly asked.
- `compile_commands.json` is gitignored but generated locally for clangd (the configured IDE engine; C_Cpp is disabled). `.clang-format` is Google style with `ColumnLimit: 0`.
- Commits use conventional style (`feat:`, `fix:`).
