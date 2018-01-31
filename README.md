# UIPEthernet-HN

This fork of UIPEthernet adds the ability to set the "hostname"

It is 100% backwards compatible with the original (at the moment of forking)

To use the new feature:
`int  Ethernet.setHostname(void* name[, int len]);`

name is length n, where 0 <= n <= 255
...each octet(/uint8/byte/whatever) may contain any value 0..255
if name is NULL, errorcode 1 is returned

if len > 255 or len < -2, errorcode 2 is returned
if len = -1, the name is treated as a NUL terminated C string and the actual length is calculated for you
if len = -2, as per -1, and also the characters are validated as being in spec of RFC 1533/3.14
...if validation fails, errorcode -x is returned where x is the character which failed validation
if len >= 0, name[] is not validated, and the length specified is used

...If len is not specified, -2 is used as a sane default.

example code:
int  err;
if ((err = Ethernet.setHostname(name, len)) != 0) {
  if      (err == 1)  invalid_name_pointer() ;
  else if (err == 2)  invalid_length() ;
  else if (err <  0)  invalid_character_in_name_at_position(-err) ;
  else                unknown_error();
}


--original document begins--

UIPEthernet library for Arduinos (Atmel AVR-s,Atmel SAM3X8E ARM Cortex-M3,STM32F series,ESP8266,Intel ARC32(Genuino101),Nordic nRF51(RFduino),Teensy boards,Realtek Ameba(RTL8195A,RTL8710)), ENC28j60 network chip compatible with Wiznet W5100 API

Original UIPEthernet writed by Norbert Truchsess.

You can find wiring diagram for more board in the hardware directory.

Modifications:
- Replaced import to include, because gcc say 'import is deprecated'.
- Added support for STM32F, and ESP8266 MCU-s.
- Merged martinayotte's modification (Correct s_dhcp ~40K more memory usage with STM32F MCU-s.)
- Moved htons,ntohs,htonl,ntohl definitions to uip.h.
- Corrected infinite loops.
- Set the version to 2.0.3
- Corrected ESP8266 exception(28).
- Added watchdog reset calls in functions for stable running on ESP8266.
- Added geterevid function to get ENC28j60 chip erevid (revision information).
- Changed linkStatus to static for outside call.
- Added functions bypass, if can't communicate with ethernet device.
- Changed debuging/logging. Remove individual debuging. Add global and scalable debuging feature.
You can setup debuging/logging level in utility/logging.h
You can use this header file in Your scetch too.
Add "LogObject" define for serial logging/debuging with board specific default setting.
- Added support to MBED/SMeshStudio IDE. (Compiled and tested on Nucleo-F302R8. (STM32F302R8))

- Added Abstract Print class to MBED for full compatibility (Can use print, println with uip objects.)
- Errata#12 corrected (by seydamir).
- Created v2.0.2 release.

If You use NodeMCU please check wiring first:
https://github.com/UIPEthernet/UIPEthernet/blob/master/hardware/NodeMCU_enc28j60_wiring.PNG

- You can save 5K flash if you disable UDP support.
- Correction code of Errata#12 modified.
- Added support for Intel ARC32(Genuino101), Nordic nRF51(RFduino), Teensy boards
- Issue#4 corrected
- Added support for Realtek Ameba(RTL8195A,RTL8710)
- Added direct broadcast support
- Issue#5 corrected: You can save 5K flash memory with disable UDP support.
- Issue#6 corrected: Added support Eclipse with arduino plugin
- Issue#8, and Issue#9 corrected: Modified DHCP code: Moved timeouts define to dhcp.h
- Issue#11 corrected: Changed ENC28J60_CONTROL_CS pin to 10 on Arduino Due
- New release:2.0.4

- Added support for Arduino_Core_STM32.
- Endianness configuration/detection changed.
- Added support for adafruit wiced feather.
- Arduino Mega2560 upload error (3 !!!) corrected.
- Added setup SS pin to output.
- New release:2.0.5

- Added support ESP32 and SAMD
- New release:2.0.6
