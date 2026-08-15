# AGENTS.md

In-vehicle instruments firmware for a Greenpower electric car. PlatformIO project targeting an Arduino Mega 2560 (ATmega2560: 8 KB RAM / 248 KB flash), Arduino framework, C++17 (`-std=gnu++17 -O2`).

## Build & verify
- Build: `pio run`
- Upload: `pio run -t upload`
- Serial monitor: `pio device monitor` (500000 baud; `monitor_speed` in platformio.ini)
- No tests or CI exist; a successful `pio run` is the verification step.

## Layout
- `src/` — the firmware; the only directory PlatformIO compiles.
- `src/main.cpp` — declares the `ScheduledTask tasks[]` array and `setup()`/`loop()`. Some tasks are commented out (e.g. `logData`, lap timing) — that is WIP, not an error.
- `src/state.h` — the `State` struct: `static inline` members holding all shared sensor/UI state. `src/utils/appmode.cpp` implements the `AppMode` state machine (IDLE/RACE/DEBUG) on top of it, including race-mode persistence (`enterMode`/`runMode`/`saveRaceState`/`tryResumeRace`).
- `src/modules/*` — one static class per device (BMP, ATH, RTC, IMU, VESC, SPEED, TEMPS, GPS, EEPROM, Display, battery). Pattern: `init(...)`, `update()`/`run()`, `isEnabled()`, an `IntervalMetric dataProcessTime` member; writes results into `State` members; reports failures via `Errors::logError(Error::...)`. `timing.*` are empty WIP stubs.
- `src/utils/` — scheduler, metrics, error log (`Errors`), logging, appmode; `system.*` is an empty WIP stub.
- `src/widgets.h` — the display UI framework: `Widget`, `SetpointWidget`, `Button`, `StaticWidget`, `Page`, plus the concrete pages/widget layouts (`RACE_PAGE`, `SETTINGS_PAGE`).
- `src/settings.*` — `SettingsBlock` persisted to the external I2C EEPROM.
- `lib/cfg/NeoGPS/` — config overrides (GPSport, NMEAGPS_cfg, NeoGPS_cfg) copied into the installed NeoGPS lib by `scripts/patch_neogps.py` (a PlatformIO `pre` extra_script in platformio.ini). If you add a `lib/cfg/...` override for a new lib, you'll need a similar patch script.
- `demo/` — standalone one-off sketches plus vendored library examples; NOT compiled by the project.
- `circuitry/` — KiCad schematics, not code.

## Gotchas
- Init order in `HARDWARE::init()` matters: all I2C devices must init before `Wire.setClock(400000)`. A 25 ms `Wire.setWireTimeout(25000, true)` is installed and the flag is checked/cleared in `HARDWARE::run()`. All modules are currently enabled.
- The `DEBUG_LOGGING` build flag (platformio.ini) enables `Logging::logDebug` prints AND adds a 3-second boot delay in `HARDWARE::init()`. Don't mistake the delay for a hang.
- Scheduler: `Timer1` emits a 1 ms tick; `ScheduledTask(period, initialOffset, fn)` tasks run in `loop()` via `Scheduler::runTasks()`, never in the ISR. The initial offset staggers tasks within a period.
- Settings persist to an external I2C EEPROM (`src/settings.cpp`, module `EEPROM`), validated by version + Fletcher-16 checksum; `SETTINGS_BLOCK_VERSION` is currently 3. If you modify `SettingsBlock` layout, bump the version in `settings.h`. `SETTINGS::apply()` reconfigures `SPEED` from settings.
- Race mode persists to settings: `saveRaceState()` (a 10 s task) writes RTC-relative timestamps plus battery-estimator state; `tryResumeRace()` re-engages RACE at boot if under an hour has elapsed.
- GPS: `Serial3` via NeoHWSerial (NeoSerial3), module switched 9600 → 57600 during `GPS::init()`. NeoGPS runs in interrupt mode; the PPS pin (D2) drives `UTCsecondStart()` so `RTC::update()` can auto-sync the DS3231 (adjusts if >10 s out of sync). `rtc.h` does `#undef SECONDS_PER_DAY` to fix a collision with NeoGPS.
- Battery: `BatteryEstimator` (Peukert + temperature derate + 1-min rolling current window) is fed from VESC data inside `VESC::update()`; `State::battery_stats` is its display string.
- Shared state lives as `static inline` members (C++17) on `State` and module `enabled` flags. With 8 KB RAM, avoid dynamic allocation.
- Serial: `Serial2` → VESC/ESC (250000 baud per `README.md`), `Serial3` → GPS. SPI (50/51/52) is shared by display, touchscreen, and SD card via separate CS pins (53/49/48). SD logging is not wired up yet (`Errors::flushToLogfile` is a TODO).
- Display/touch: ILI9488 via Arduino_GFX (`Arduino_ILI9488_18bit`), touch via XPT2046; calibration constants live in `hardware.h` (resolved via the `demo/touch_calibration.cpp` example).
- VESC controller settings in `README.md` are hardware config; leave them unless explicitly asked.
- `compile_commands.json` is gitignored but generated locally for clangd (the configured IDE engine; C_Cpp is disabled). `.clang-format` is Google style with `ColumnLimit: 0`.
- Commits use conventional style (`feat:`, `fix:`).
