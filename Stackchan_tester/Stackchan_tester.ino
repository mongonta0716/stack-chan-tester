#include <Arduino.h>

#if defined(ARDUINO_M5STACK_Core2)
  // M5Stack Core2用のサーボの設定
  // Port.A X:G32, Y:G33
  // Port.C X:G13, Y:G14
  // スタックチャン基板 X:G27, Y:G19
  #include <M5Core2.h>
  #define SERVO_PIN_X 13
  #define SERVO_PIN_Y 14
#elif defined( ARDUINO_M5STACK_FIRE )
  // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
  // I2Cと同時利用は不可
  #include <M5Stack.h>
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  // M5Stack Basic/Gray/Go用の設定
  // Port.A X:G22, Y:G21
  // Port.C X:G16, Y:G17
  // スタックチャン基板 X:G5, Y:G2
  #include <M5Stack.h>
  #define SERVO_PIN_X 16
  #define SERVO_PIN_Y 17
#endif

int servo_offset_x = 10; // X軸サーボのオフセット（90°からの+-で設定）
int servo_offset_y = 0;  // Y軸サーボのオフセット（90°からの+-で設定）

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

void moveX(int x, uint32_t millis_for_move = 0) {
  if (millis_for_move == 0) {
    servo_x.easeTo(x + servo_offset_x);
  } else {
    servo_x.easeToD(x + servo_offset_x, millis_for_move);
  }
}

void moveY(int y, uint32_t millis_for_move = 0) {
  if (millis_for_move == 0) {
    servo_y.easeTo(y + servo_offset_y);
  } else {
    servo_y.easeToD(y + servo_offset_y, millis_for_move);
  }
}

void moveXY(int x, int y, uint32_t millis_for_move = 0) {
  if (millis_for_move == 0) {
    servo_x.setEaseTo(x + servo_offset_x);
    servo_y.setEaseTo(y + servo_offset_y);
  } else {
    servo_x.setEaseToD(x + servo_offset_x, millis_for_move);
    servo_y.setEaseToD(y + servo_offset_y, millis_for_move);
  }
  // サーボが停止するまでウェイトします。
  synchronizeAllServosStartAndWaitForAllServosToStop();
}

void setup() {

#if defined(ARDUINO_M5STACK_Core2)
  M5.begin(true, true, true, false, kMBusModeOutput);
  // M5.begin(true, true, true, false, kMBusModeInput);
#elif defined( ARDUINO_M5STACK_FIRE ) || defined( ARDUINO_M5Stack_Core_ESP32 )
  M5.begin(true, true, true, false); // Grove.Aを使う場合は第四引数(I2C)はfalse
#endif
  if (servo_x.attach(SERVO_PIN_X, 
                     START_DEGREE_VALUE_X + servo_offset_x,
                     DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                     DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
    Serial.print("Error attaching servo x");
  }
  if (servo_y.attach(SERVO_PIN_Y,
                     START_DEGREE_VALUE_Y + servo_offset_y,
                     DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                     DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
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
    moveXY(90, 90);
  }
  if (M5.BtnB.wasPressed()) {
    random_move = false;
    for (int i=0; i<2; i++) {
      avatar.setSpeechText("X 90 -> 0  ");
      moveX(0);
      avatar.setSpeechText("X 0 -> 180  ");
      moveX(180);
      avatar.setSpeechText("X 180 -> 90  ");
      moveX(90);
      avatar.setSpeechText("Y 90 -> 50  ");
      moveY(50);
      avatar.setSpeechText("Y 50 -> 90  ");
      moveY(90);
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
     moveXY(x, y + 50);
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
