# Wacom Wireless (CTL-4100) via RP2040 / Pico 2W

Converts a Wacom Intuos S (CTL-4100) into a wireless tablet via Bluetooth Low Energy (BLE) or Wi-Fi (UDP) using a Raspberry Pi Pico 2W.

## Wiring Diagram

```text
 Pico 2W             Wacom CTL-4100
---------            ---------------
  VSYS  <--- 5V --->  VBUS (Micro-USB)
   GND  <--- GND -->  GND  (Micro-USB)
 GPIO0  <--- D+ --->  D+   (Micro-USB)
 GPIO1  <--- D- --->  D-   (Micro-USB)
```

**Important**: Power the Pico from an external 5V source via the VSYS pin. Do not plug the Pico into your computer via USB while also providing external power directly to VSYS unless you've added a Schottky diode to protect the Pico's USB port. The Pico will supply 5V VBUS to the Wacom tablet via USB Host.

## Firmware Configuration

1. Edit `src/config.h`.
2. Select your transport mode by setting `TRANSPORT_MODE` to `TRANSPORT_BLE` or `TRANSPORT_WIFI`.
3. If using Wi-Fi, enter your AP SSID and password. Define the `DAEMON_IP` to your computer's IP address (or use `.255` subnet broadcast).

## Building Firmware

```bash
mkdir build
cd build
cmake ..
make -j4
```
Flash `wacom_wireless.uf2` to the Pico 2W.

## Operating System Requirements

Depending on the transport mode, you may or may not need the background daemon.

| OS      | BLE Mode                        | Wi-Fi Mode                       |
|---------|---------------------------------|----------------------------------|
| Linux   | Native (No daemon needed)*      | Daemon needed (`wacom_wifi_daemon`) |
| Windows | Native (No daemon needed)       | Daemon needed (`wacom_wifi_daemon.exe`) |

*\* OpenTabletDriver detects `/dev/hidraw` directly. Uninstall standard Wacom drivers.*

### OpenTabletDriver Setup
- Uninstall any official Wacom drivers from your system.
- Ensure OpenTabletDriver is installed.
- OpenTabletDriver will natively detect the emulated BLE HID device or the VirtualHID device created by the daemon as a "Wacom CTL-4100" without custom JSON configuration.

## Daemon Installation (Wi-Fi Mode Only)

### Linux
```bash
cd daemon/linux
make
sudo cp wacom_wifi_daemon /usr/local/bin/
cp wacom-wifi-daemon.service ~/.config/systemd/user/
systemctl --user enable --now wacom-wifi-daemon.service
```

### Windows
1. Build `wacom_wifi_daemon.exe` via MinGW/MSVC using `make` in `daemon/windows`.
2. Install [VMulti](https://github.com/djpnewton/vmulti) or a VirtualHID driver wrapper.
3. Run `wacom_wifi_daemon.exe` in the background.

## Known Limitations
- BLE mode has ~10-15ms latency due to protocol overhead.
- Wi-Fi mode requires an active AP (router) and is subject to network jitter.

## Windows Daemon — Virtual HID Driver
The Windows daemon receives the UDP packets but requires a Virtual HID driver to inject them. In `daemon/windows/main.c`, implement the `inject_hid_report` function using one of the following methods depending on your environment:

- **Option A (VMulti)**: Include the vmulti client headers and call `vmulti_update_digitizer()` or `vmulti_update_report()`.
- **Option B (ViGEm/VirtualHID)**: Use the ViGEm APIs to push the report array.
- **Option C (Raw WriteFile)**: Open a handle to a custom virtual HID device and use `WriteFile()`.
