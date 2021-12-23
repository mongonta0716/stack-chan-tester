#include <Arduino.h>

#if defined(ARDUINO_M5STACK_Core2)
  // M5Stack Core2用のサーボの設定
  // Port.A G32,G33
  // Port.C G13,G14
  // スタックチャン基板 G27,G19
  #include <M5Core2.h>
  #define SERVO_PIN_X 13
  #define SERVO_PIN_Y 14
#elif defined( ARDUINO_M5STACK_FIRE )
  // M5Stack Fireの場合はPort.A(G21,G22)のみです。
  // I2Cと同時利用は不可
  #include <M5Stack.h>
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  // M5Stack Basic/Gray/Go用の設定
  // Port.A G21,G22
  // Port.C G16,G17
  // スタックチャン基板 G5,G2
  #include <M5Stack.h>
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#endif

#include <Avatar.h> // https://github.com/meganetaaan/m5stack-avatar
#include <ServoEasing.hpp> // https://github.com/ArminJo/ServoEasing       

using namespace m5avatar;
Avatar avatar;

#define START_DEGREE_VALUE_X 90
#define START_DEGREE_VALUE_Y 90

ServoEasing servo_x;
ServoEasing servo_y;

bool random_move = false;

const char* lyrics[] = { "BtnA:MoveTo90  ", "BtnB:ServoTest  ", "BtnC:RandomMode  "};
const int lyrics_size = sizeof(lyrics) / sizeof(char*);
int lyrics_idx = 0;

void setup() {

#if defined(ARDUINO_M5STACK_Core2)
  M5.begin(true, true, true, false, kMBusModeOutput);
  // M5.begin(true, true, true, false, kMBusModeInput);
#elif defined( ARDUINO_M5STACK_FIRE ) || defined( ARDUINO_M5Stack_Core_ESP32 )
  M5.begin(true, true, true, false); // Grove.Aを使う場合は第四引数(I2C)はfalse
#endif
  if (servo_x.attach(SERVO_PIN_X, START_DEGREE_VALUE_X, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
    Serial.print("Error attaching servo x");
  }
  if (servo_y.attach(SERVO_PIN_Y, START_DEGREE_VALUE_Y, DEFAULT_MICROSECONDS_FOR_0_DEGREE, DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
    Serial.print("Error attaching servo y");
  }
  servo_x.setEasingType(EASE_QUADRATIC_IN_OUT);
  servo_y.setEasingType(EASE_QUADRATIC_IN_OUT);
  setSpeedForAllServos(60);
  avatar.init();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    random_move = false;
    servo_x.setEaseTo(90);
    servo_y.setEaseTo(90);
    synchronizeAllServosStartAndWaitForAllServosToStop();
  }
  if (M5.BtnB.wasPressed()) {
    random_move = false;
    for (int i=0; i<2; i++) {
      avatar.setSpeechText("X 90 -> 0  ");
      servo_x.easeTo(0);
      avatar.setSpeechText("X 0 -> 180  ");
      servo_x.easeTo(180);
      avatar.setSpeechText("X 180 -> 90  ");
      servo_x.easeTo(90);
      avatar.setSpeechText("Y 90 -> 50  ");
      servo_y.easeTo(50);
      avatar.setSpeechText("Y 50 -> 90  ");
      servo_y.easeTo(90);
    }
  }
  if (M5.BtnC.wasPressed()) {
    random_move = !random_move;
  }

  if (random_move) {
    int x = random(180);
    int y = random(40);
    M5.update();
    if (M5.BtnC.wasPressed()) {
      random_move = false;
    }
    servo_x.setEaseTo(x);
    servo_y.setEaseTo(y + 40);
    synchronizeAllServosStartAndWaitForAllServosToStop();
    int delay_time = random(10);
    delay(2000 + 100*delay_time);
    avatar.setSpeechText("Stop BtnC");
  }
  if (!random_move) {
    const char* l = lyrics[lyrics_idx++ % lyrics_size];
    avatar.setSpeechText(l);
    avatar.setMouthOpenRatio(0.7);
    delay(200);
    avatar.setMouthOpenRatio(0.0);
    delay(2000);
  }

}
