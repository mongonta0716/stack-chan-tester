# stack-chan-tester

[日本語](README.md) | English

stack-chan test application for pwm servo

# Supported kits
 This kit was developed to check the operation of [Stack-Chan M5GoBottom version assembly kit](https This application was developed to check the operation of  [Stack-Chan M5GoBottom version kit](https://mongonta.booth.pm/) and to set up when assembling the kit. It can also be used for stacked-channel boards by changing the pin settings.

This is only for ArduinoFramework and PWM servos.

# Pin settings for servo
CoreS3 uses Port.C(G18,G17), Core2 uses Port.C(G13,G14), Fire uses Port.A(G22,G21), and Core1 uses Port.C(G16,G17). If you want to use different pins, please rewrite the following section.

https://github.com/mongonta0716/stack-chan-tester/blob/main/src/main.cpp#L7-L35

# Adjustment of servo offset

PWM servos of the SG90 series have many individual differences, and even if 90° is specified, the servo may shift slightly. In this case, please adjust the offset value below. (Set the angle (±) from 90°.)
For the value to be adjusted, press and hold button A to enter the mode to adjust the offset, and find out the value that makes it straight.

Please refer to Okimoku's [How to make a stack chan M5Burner version without disassembly [second half]](https://www.youtube.com/watch?v=YRQYYrQh0oE&t=329s) for how to adjust the offset.(Japanese)

The method of setting the adjusted value depends on the firmware to be written.

- The one that rewrites the initial value in the source. <br>Rewrite the part where 90 is specified in the initial settings and rebuild.
- Rewriting the configuration file. <br>[stackchan-bluetooth-simple](https://github.com/mongonta0716/stackchan-bluetooth-simple) specifies the offset in the configuration file.
- The one to be configured on the web. <br>NoRi's [WebServer-with-stackchan](https://github.com/NoRi-230401/WebServer-with-stackchan) has the ability to configure on the web.

# Usage
* Button A: Rotates the X-axis and Y-axis servos to 90°. Use this button to rotate the servo 90° before fixing.
* Button B: X-axis moves from 0 to 180, Y-axis moves from 90 to 50.<br>Double-click to toggle Grove's 5V (ExtPower) output ON/OFF; turn OFF when checking power feed from behind Stack-chan_Takao_Base.
* Button C: Moves in random mode.
    * Button C: Stop random mode.
* Button A long press: Enter the mode to adjust and examine the offset.
    * Button A: Decrease offset.
    * Button B: toggles between X and Y axis
    * Button C: Increase offset
    * Button B long press: Out the mode to adjust. 

## Button handling in CoreS3
CoreS3 has changed the handling of buttons because the BtnA, B, and C parts of Core2 have been replaced by cameras and microphones. <br>
The screen is divided vertically into three parts: left: BtnA, center: BtnB, and right: BtnC.

# Requirement Libraries
※ If it does not work with the latest version, try matching the library version.
- [M5Unified](https://github.com/m5stack/M5Unified) v0.0.7
- [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar) v0.8.0<br> Prior to v0.7.4, M5Unified is not supported, so M5 double definition errors occur at build time.
- [ServoEasing](https://github.com/ArminJo/ServoEasing) v2.4.0
- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo) v0.11.0

Please refer to the following blog for detailed instructions on how to add libraries in ArduinoIDE. (in Japanese)

[ｽﾀｯｸﾁｬﾝ M5GoBottom版のファームウェアについて | M5Stack沼人の日記]( https://raspberrypi.mongonta.com/softwares-for-stackchan/)

# How to build

v0.1 was ArduinoIDE, but now it is intended to be built with PlatformIO.

# About stack chan
Stack chan is [meganetaaan](https://github.com/meganetaaan) is an open source project.

https://github.com/meganetaaan/stack-chan

# author
 Takao Akaki

# LICENSE
 MIT