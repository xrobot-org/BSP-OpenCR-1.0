# ICM20948

XRobot module for the OpenCR onboard ICM-20948 SPI IMU.

## Required Hardware

- `spi1`
- `IMU_CS`

Optional:

- `IMU_INT`

## Constructor Args

- `sample_period_ms`: default `10`
- `auto_start`: default `true`

## API

- `IsOnline()`
- `GetWhoAmI()`
- `Poll()`
- `GetSample()`

The module reads raw accelerometer, gyroscope, and temperature values, and provides simple scaled SI units. It does not perform attitude fusion or calibration.
