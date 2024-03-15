 #include <Arduino.h>

#include <SD.h>
#include <Ticker.h>
#include <M5StackUpdater.h>
#include <M5Unified.h>


#if defined(ARDUINO_M5STACK_Core2)
  // M5Stack Core2用のサーボの設定
  // Port.A X:G33, Y:G32
  // Port.C X:G13, Y:G14
  // スタックチャン基板 X:G19, Y:G27
  #define SERVO_PIN_X 33
  #define SERVO_PIN_Y 32
#elif defined( ARDUINO_M5STACK_FIRE )
  // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
  // I2Cと同時利用は不可
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#if SERVO_PIN_X == 22
  // FireでPort.Aを使う場合は内部I2CをOffにする必要がある。
  #define CORE_PORT_A
#endif

#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  // M5Stack Basic/Gray/Go用の設定
  // Port.A X:G22, Y:G21
  // Port.C X:G16, Y:G17
  // スタックチャン基板 X:G5, Y:G2
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21

#if SERVO_PIN_X == 22
  // CoreでPort.Aを使う場合は内部I2CをOffにする必要がある。
  #define CORE_PORT_A
#endif

#elif defined( ARDUINO_M5STACK_CORES3 )
  // M5Stack CoreS3用の設定 ※暫定的にplatformio.iniにARDUINO_M5STACK_CORES3を定義しています。
  // Port.A X:G1 Y:G2
  // Port.B X:G8 Y:G9
  // Port.C X:18 Y:17
  #define SERVO_PIN_X 1 
  #define SERVO_PIN_Y 2
  #include <gob_unifiedButton.hpp> // 2023/5/12現在 M5UnifiedにBtnA等がないのでGobさんのライブラリを使用
  goblib::UnifiedButton unifiedButton;
#elif defined( ARDUINO_M5STACK_DIAL )
  // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
  // I2Cと同時利用は不可
  #define SERVO_PIN_X 13
  #define SERVO_PIN_Y 15
#endif

int servo_offset_x = 0;  // X軸サーボのオフセット（90°からの+-で設定）
int servo_offset_y = 0;  // Y軸サーボのオフセット（90°からの+-で設定）

#include <Avatar.h> // https://github.com/meganetaaan/m5stack-avatar
#include <ServoEasing.hpp> // https://github.com/ArminJo/ServoEasing       
#include "formatString.hpp" // https://gist.github.com/GOB52/e158b689273569357b04736b78f050d6

using namespace m5avatar;
Avatar avatar;

#define START_DEGREE_VALUE_X 90
#define START_DEGREE_VALUE_Y 90

#define SDU_APP_PATH "/stackchan_tester.bin"
#define TFCARD_CS_PIN 4

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
  for (;;) {
#ifdef ARDUINO_M5STACK_CORES3
    unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
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

    std::string s;

    if (adjustX) {
      s = formatString("%s:%d:BtnB:X/Y", "X", servo_offset_x);
    } else {
      s = formatString("%s:%d:BtnB:X/Y", "Y", servo_offset_y);
    }
    avatar.setSpeechText(s.c_str());

  }
}

void moveRandom() {
  for (;;) {
    // ランダムモード
    int x = random(45, 135);  // 45〜135° でランダム
    int y = random(60, 90);   // 50〜90° でランダム
#ifdef ARDUINO_M5STACK_CORES3
    unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
    M5.update();
    if (M5.BtnC.wasPressed()) {
      break;
    }
    int delay_time = random(10);
    moveXY(x, y, 1000 + 100 * delay_time);
    delay(2000 + 500 * delay_time);
#if !defined( CORE_PORT_A )
    // Basic/M5Stack Fireの場合はバッテリー情報が取得できないので表示しない
    avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
#endif
    //avatar.setSpeechText("Stop BtnC");
    avatar.setSpeechText("");
  }
}
void testServo() {
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

void setup() {
  auto cfg = M5.config();     // 設定用の情報を抽出
  cfg.output_power = true;   // Groveポートの出力をしない
  M5.begin(cfg);              // M5Stackをcfgの設定で初期化
#if defined( ARDUINO_M5STACK_CORES3 )
  unifiedButton.begin(&M5.Display, goblib::UnifiedButton::appearance_t::transparent_all);
#endif
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);
  M5_LOGI("Hello World");
  Serial.println("HelloWorldSerial");
  //USBSerial.println("HelloWorldUSBSerial");
  avatar.init();
#if defined( CORE_PORT_A )
  // M5Stack Fireの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。
  avatar.setBatteryIcon(false);
  M5.In_I2C.release();
#else
  avatar.setBatteryIcon(true);
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
  last_mouth_millis = millis();
  //moveRandom();
  //testServo();
}

void loop() {
#ifdef ARDUINO_M5STACK_CORES3
  unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
  M5.update();
  if (M5.BtnA.pressedFor(2000)) {
    // サーボのオフセットを調整するモードへ
    adjustOffset();
  } else if (M5.BtnA.wasPressed()) {
    moveXY(90, 90);
  }
  
  if (M5.BtnB.wasSingleClicked()) {
    testServo();
  } else if (M5.BtnB.wasDoubleClicked()) {
    if (M5.Power.getExtOutput() == true) {
      M5.Power.setExtOutput(false);
      avatar.setSpeechText("ExtOutput Off");
    } else {
      M5.Power.setExtOutput(true);
      avatar.setSpeechText("ExtOutput On");
    }
    delay(2000);
    avatar.setSpeechText(""); 
  } 
  if (M5.BtnC.pressedFor(5000)) {
    M5_LOGI("Will copy this sketch to filesystem");
    if (saveSketchToFS( SD, SDU_APP_PATH, TFCARD_CS_PIN )) {
      M5_LOGI("Copy Successful!");
    } else {
      M5_LOGI("Copy failed!");
    }
  } else if (M5.BtnC.wasPressed()) {
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
#if !defined( CORE_PORT_A )
    avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
#endif
  }
  // delayを50msec程度入れないとCoreS3でバッテリーレベルと充電状態がおかしくなる。
  delay(0);

}
