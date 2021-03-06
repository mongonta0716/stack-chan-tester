#include <Arduino.h>

#include <M5Unified.h>
#if defined(ARDUINO_M5STACK_Core2)
  // M5Stack Core2用のサーボの設定
  // Port.A X:G33, Y:G32
  // Port.C X:G13, Y:G14
  // スタックチャン基板 X:G27, Y:G19
  #define SERVO_PIN_X 13
  #define SERVO_PIN_Y 14
#elif defined( ARDUINO_M5STACK_FIRE )
  // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
  // I2Cと同時利用は不可
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  // M5Stack Basic/Gray/Go用の設定
  // Port.A X:G22, Y:G21
  // Port.C X:G16, Y:G17
  // スタックチャン基板 X:G5, Y:G2
  #define SERVO_PIN_X 16
  #define SERVO_PIN_Y 17
#endif

int servo_offset_x = 0;  // X軸サーボのオフセット（90°からの+-で設定）
int servo_offset_y = 0;  // Y軸サーボのオフセット（90°からの+-で設定）

#include <Avatar.h> // https://github.com/meganetaaan/m5stack-avatar
#include <ServoEasing.hpp> // https://github.com/ArminJo/ServoEasing       

using namespace m5avatar;
Avatar avatar;

#define START_DEGREE_VALUE_X 90
#define START_DEGREE_VALUE_Y 90

ServoEasing servo_x;
ServoEasing servo_y;

uint32_t mouth_wait = 2000; // 通常時のセリフ入れ替え時間（msec）
uint32_t last_mouth_millis = 0;

const char* lyrics[] = { "BtnA:MoveTo90  ", "BtnB:ServoTest  ", "BtnC:RandomMode  ", "BtnALong:AdjustMode"};
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

void adjustOffset() {
  // サーボのオフセットを調整するモード
  servo_offset_x = 0;
  servo_offset_y = 0;
  moveXY(90, 90);
  bool adjustX = true;
  char text[20] = "AdjustMode";
  for (;;) {
    M5.update();
    if (M5.BtnA.wasPressed()) {
      // オフセットを減らす
      if (adjustX) {
        servo_offset_x--;
      } else {
        servo_offset_y--;
      }
    }
    if (M5.BtnB.pressedFor(2000)) {
      // 調整モードを終了
      break;
    }
    if (M5.BtnB.wasPressed()) {
      // 調整モードのXとYを切り替え
      adjustX = !adjustX;
    }
    if (M5.BtnC.wasPressed()) {
      // オフセットを増やす
      if (adjustX) {
        servo_offset_x++;
      } else {
        servo_offset_y++;
      }
    }
    moveXY(90, 90);

    if (adjustX) {
      sprintf(text, "X:%d:BtnB:X/Y", servo_offset_x);
    } else {
      sprintf(text, "Y:%d:BtnB:X/Y", servo_offset_y);
    }
    const char* l = (const char*)text;

    avatar.setSpeechText(l);
  }
}

void moveRandom() {
  for (;;) {
    // ランダムモード
    int x = random(60, 120);  // 45〜135° でランダム
    int y = random(60, 90);   // 50〜90° でランダム
    M5.update();
    if (M5.BtnC.wasPressed()) {
      break;
    }
    int delay_time = random(10);
    moveXY(x, y, 1000 + 100 * delay_time);
    delay(2000 + 500 * delay_time);
    //avatar.setSpeechText("Stop BtnC");
    avatar.setSpeechText("");
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
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
  last_mouth_millis = millis();
  //moveRandom();
}

void loop() {
  M5.update();
  if (M5.BtnA.pressedFor(2000)) {
    // サーボのオフセットを調整するモードへ
    adjustOffset();
  }
  if (M5.BtnA.wasPressed()) {
    moveXY(90, 90);
  }
  
  if (M5.BtnB.wasPressed()) {
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
    // ランダムモードへ
    moveRandom();
  }

  if ((millis() - last_mouth_millis) > mouth_wait) {
    const char* l = lyrics[lyrics_idx++ % lyrics_size];
    avatar.setSpeechText(l);
    avatar.setMouthOpenRatio(0.7);
    delay(200);
    avatar.setMouthOpenRatio(0.0);
    last_mouth_millis = millis();
  }

}
