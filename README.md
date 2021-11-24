# Firmware for a POC IoT device

This respository contain a firmware for an IoT POC device.  
The device is based on the RAK5010 board with a NRF52840 SoC.

The sensors present on the board consist of:
- Accelerometer (LSM6DSM)
- Pressure sensor (Interlink FSR 400)
- Temperature / humidity sensor (SHTC3)

In order to sync with BG96, there is need to remove the RTS/CTS  
pins from the board dts.
