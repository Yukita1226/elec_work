# Knee Project Instructions

**Repository:** [elec_work / 4knee_project](https://github.com/Yukita1226/elec_work/tree/main/4knee_project)

---

## 1. Backend (ESP32-C3)

**Board:** ESP32-C3 Dev Module

### ⚠️ Upload Note
Before connecting the board to your PC:
1. **Hold the BOOT button**
2. **Then** plug in / connect the COM port
3. Release BOOT after the port connects

> If you skip this step, the board may connect and disconnect repeatedly instead of enumerating properly.

### Setup Steps
1. If you swap to a **different ESP32 board**, you'll need to update the `receiverMac` address in `back_c3.ino`.
2. To get the new board's MAC address:
   - Upload `checkaddress.ino` to the new ESP32
   - Read the printed MAC address from Serial Monitor
   - Copy it into `receiverMac[]` in `back_c3.ino`

### File Location