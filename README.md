# stack-chan-tester
スタックチャンを作成するときに、PWMサーボの調整及びテストを行うためのアプリケーションです。<br>
stack-chan test application for pwm servo

※ ArduinoFramework及びPWMサーボのみです。

# サーボのピンの設定
Core2はPort.A 13,14、FireとCore1は Port.A 21,22を使うようになっています。違うピンを使用する場合は下記の箇所を書き換えてください。

https://github.com/mongonta0716/stack-chan-tester/blob/main/Stackchan_tester/Stackchan_tester.ino#L3-L24

# サーボのオフセット調整
SG90系のPWMサーボは個体差が多く、90°を指定しても少しずれる場合があります。その場合は下記のオフセット値を調整してください。
https://github.com/mongonta0716/stack-chan-tester/blob/main/Stackchan_tester/Stackchan_tester.ino#L27-L28

# 使い方
- ボタンA： X軸、Y軸のサーボを90°に回転します。固定前に90°にするときに使用してください。
- ボタンB： X軸は0〜180, Y軸は90〜50まで動きます。
- ボタンC： ランダムで動きます。

# 必要なライブラリ
- [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar)
- [ServoEasing](https://github.com/ArminJo/ServoEasing)
- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)

# ビルド方法
 下記のリンクを参照してArduinoIDEでビルドできます。<br>
 [初心者向けM5Stack Core2の始め方（ArduinoIDE編）](https://raspberrypi.mongonta.com/howto-start-m5stack-core2arduinoide/)

# <del>書き込み後再起動を繰り返す場合</del>(M5Stack-Avatar 0.7.4で修正されました。)
<del> 2021/12現在、Arduino-esp32のバージョンをv2.0.0以上にすると、書き込み後再起動を繰り返すという現象が見られます。</del>

## 対処①
<del> ボードマネージャーでarduino-esp32 ver.1.0.6に戻してビルドする。</del>

## 対処②
<del> librariesの中にあるm5stack-avatar/src/Avatar.cppを修正してください。修正箇所は下記のリンクにあります。
 - [fix platformio link error #66](https://github.com/meganetaaan/m5stack-avatar/pull/66/commits/f28efa87d482a730237565a666d67d7422e638f4)</del>

# author
 Takao Akaki

# LICENSE
 MIT