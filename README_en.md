# stack-chan-tester

[日本語](README.md) | English

stack-chan test application for pwm servo

# Supported kits
 This kit was developed to check the operation of [Stack-Chan M5GoBottom version assembly kit](https This application was developed to check the operation of  [Stack-Chan M5GoBottom version kit](https://mongonta.booth.pm/) and to set up when assembling the kit. It can also be used for stacked-channel boards by changing the pin settings.

This is only for ArduinoFramework and PWM servos.

# Pin settings for servo
Core2 uses Port.C(G13,G14), Fire uses Port.A(G22,G21), and Core1 uses Port.C(G16,G17). If you want to use different pins, please rewrite the following section.

https://github.com/mongonta0716/stack-chan-tester/blob/main/Stackchan_tester/Stackchan_tester.ino#L3-L24

# Adjustment of servo offset
There are many individual differences among SG90 series PWM servos, and even if 90° is specified, the servo may shift a little. In this case, please adjust the following offset value. (Set the angle (±) from 90°.)
For the value to be adjusted, press and hold button A to enter the mode to adjust the offset, and find out the value that makes it straight.

https://github.com/mongonta0716/stack-chan-tester/blob/main/Stackchan_tester/Stackchan_tester.ino#L27-L28

# Usage
* Button A: Rotates the X-axis and Y-axis servos to 90°. Use this button to rotate the servo 90° before fixing.
* Button B: X-axis moves from 0 to 180, Y-axis moves from 90 to 50.
* Button C: Moves in random mode.
    * Button C: Stop random mode.
* Button A long press: Enter the mode to adjust and examine the offset.
    * Button A: Decrease offset.
    * Button B: toggles between X and Y axis
    * Button C: Increase offset
    * Button B long press: Out the mode to adjust. 
# Requirement Libraries
※ If it does not work with the latest version, try matching the library version.
- [M5Unified](https://github.com/m5stack/M5Unified) v0.0.7
- [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar) v0.8.0<br> Prior to v0.7.4, M5Unified is not supported, so M5 double definition errors occur at build time.
- [ServoEasing](https://github.com/ArminJo/ServoEasing) v2.4.0
- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo) v0.11.0

Please refer to the following blog for detailed instructions on how to add libraries in ArduinoIDE. (in Japanese)

[ｽﾀｯｸﾁｬﾝ M5GoBottom版のファームウェアについて | M5Stack沼人の日記]( https://raspberrypi.mongonta.com/softwares-for-stackchan/)

# Version of arduino-esp32

※ We have confirmed that it works with Arduino-esp32 v2.0.2. If it does not work with 2.0.2 or later, please try reverting the version in the board manager.

# How to build

You can build with ArduinoIDE by referring to the following link. (in Japanese)
 
 [初心者向けM5Stack Core2の始め方（ArduinoIDE編）](https://raspberrypi.mongonta.com/howto-start-m5stack-core2arduinoide/)

# About stack chan
Stack chan is [meganetaaan](https://github.com/meganetaaan) is an open source project.

https://github.com/meganetaaan/stack-chan

# author
 Takao Akaki

# LICENSE
 MIT