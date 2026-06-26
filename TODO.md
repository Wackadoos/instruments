# TODOs

Do we need to flush sd to not corrupt in power down failure? How does this work?

“WHO_AM_I” Returns Wrong Value
The MPU9250 should return 0x71 when you read register 0x75. If you get 0x70, you likely have an MPU6500 (no magnetometer). If you get 0x73, you have an MPU9255.

All pins + voltage & vcc
Setup Whackadoos repo & commit to git

Update vesc
115200 baud on vesc to arduino isn't reliable, use 250000.

Stickers for isolator & recovery/tow points
Figure out settings for Flipsky
Enable/disable?

Error handling & display class
Cooperative multitasking
interrupt scheduled + multiple of interval + offset

i2c max speed?
spi speeds?

Preallocate sd card space for race mode

Do second KiCAD page with connectors diagram showing full loom & jst & anderson PP connector locations & pin connections
