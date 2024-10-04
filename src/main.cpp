 #include <Arduino.h>

#include <SD.h>
#include <Update.h>
#include <Ticker.h>
#include <M5StackUpdater.h>
#include <M5Unified.h>
#include <Stackchan_system_config.h>
#include <Stackchan_servo.h>
#ifdef ARDUINO_M5STACK_CORES3
#include <gob_unifiedButton.hpp>
goblib::UnifiedButton unifiedButton;
#endif

int servo_offset_x = 0;  // X軸サーボのオフセット（サーボの初期位置からの+-で設定）
int servo_offset_y = 0;  // Y軸サーボのオフセット（サーボの初期位置からの+-で設定）

#include <Avatar.h> // https://github.com/meganetaaan/m5stack-avatar
#include "formatString.hpp" // https://gist.github.com/GOB52/e158b689273569357b04736b78f050d6

using namespace m5avatar;
Avatar avatar;

#define START_DEGREE_VALUE_X 90
#define START_DEGREE_VALUE_Y 90

#define SDU_APP_PATH "/stackchan_tester.bin"
#define TFCARD_CS_PIN 4

StackchanSERVO servo;
StackchanSystemConfig system_config;

uint32_t mouth_wait = 2000;       // 通常時のセリフ入れ替え時間（msec）
uint32_t last_mouth_millis = 0;   // セリフを入れ替えた時間
bool core_port_a = false;         // Core1のPortAを使っているかどうか

const char* lyrics[] = { "BtnA:MoveTo90  ", "BtnB:ServoTest  ", "BtnC:RandomMode  ", "BtnALong:AdjustMode"};
const int lyrics_size = sizeof(lyrics) / sizeof(char*);
int lyrics_idx = 0;

void adjustOffset() {
  // サーボのオフセットを調整するモード
  servo_offset_x = 0;
  servo_offset_y = 0;
  servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree, system_config.getServoInfo(AXIS_Y)->start_degree, 2000);
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
    servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree, system_config.getServoInfo(AXIS_Y)->start_degree, 2000);

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
    int x = random(system_config.getServoInfo(AXIS_X)->lower_limit + 45, system_config.getServoInfo(AXIS_X)->upper_limit - 45);  // 可動範囲の下限+45〜上限-45 でランダム
    int y = random(system_config.getServoInfo(AXIS_Y)->lower_limit, system_config.getServoInfo(AXIS_Y)->upper_limit);   // 可動範囲の下限〜上限 でランダム
#ifdef ARDUINO_M5STACK_CORES3
    unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
    M5.update();
    if (M5.BtnC.wasPressed()) {
      break;
    }
    int delay_time = random(10);
    servo.moveXY(x, y, 1000 + 100 * delay_time);
    delay(2000 + 500 * delay_time);
    if (!core_port_a) {
      // Basic/M5Stack Fireの場合はバッテリー情報が取得できないので表示しない
      avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
    }
    //avatar.setSpeechText("Stop BtnC");
    avatar.setSpeechText("");
  }
}
void testServo() {
  for (int i=0; i<2; i++) {
    avatar.setSpeechText("X center -> left  ");
    servo.moveX(system_config.getServoInfo(AXIS_X)->lower_limit, 1000);
    avatar.setSpeechText("X left -> right  ");
    servo.moveX(system_config.getServoInfo(AXIS_X)->upper_limit, 3000);
    avatar.setSpeechText("X right -> center  ");
    servo.moveX(system_config.getServoInfo(AXIS_X)->start_degree, 1000);
    avatar.setSpeechText("Y center -> lower  ");
    servo.moveY(system_config.getServoInfo(AXIS_Y)->lower_limit, 1000);
    avatar.setSpeechText("Y lower -> upper  ");
    servo.moveY(system_config.getServoInfo(AXIS_Y)->upper_limit, 1000);
    avatar.setSpeechText("Initial Pos.");
    servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree, system_config.getServoInfo(AXIS_Y)->start_degree, 1000);
  }
}

void mumumuServo() {
  for (int i=0; i<30; i++) {
    servo.moveX(120, 250);
    servo.moveX(240, 250);
  }
}

void setup() {
  auto cfg = M5.config();     // 設定用の情報を抽出
  //cfg.output_power = true;    // Groveポートの5V出力をする／しない（TakaoBase用）
  M5.begin(cfg);              // M5Stackをcfgの設定で初期化
#ifdef ARDUINO_M5STACK_CORES3
  unifiedButton.begin(&M5.Display, goblib::UnifiedButton::appearance_t::transparent_all);
#endif
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);
  M5_LOGI("Hello World");
  SD.begin(GPIO_NUM_4, SPI, 25000000);
  delay(2000);
 
  system_config.loadConfig(SD, ""); 
  if (M5.getBoard() == m5::board_t::board_M5Stack) {
    if (system_config.getServoInfo(AXIS_X)->pin == 22) {
      // M5Stack Coreの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。バッテリー表示は不可。
      avatar.setBatteryIcon(false);
      M5.In_I2C.release();
      core_port_a = true;
    }
  } else {
    avatar.setBatteryIcon(true);
  }
  // servo
  servo.begin(system_config.getServoInfo(AXIS_X)->pin, system_config.getServoInfo(AXIS_X)->start_degree,
              system_config.getServoInfo(AXIS_X)->offset,
              system_config.getServoInfo(AXIS_Y)->pin, system_config.getServoInfo(AXIS_Y)->start_degree,
              system_config.getServoInfo(AXIS_Y)->offset,
              (ServoType)system_config.getServoType());

  M5.Power.setExtOutput(!system_config.getUseTakaoBase()); // 設定ファイルのTakaoBaseがtrueの場合は、Groveポートの5V出力をONにする。

  M5_LOGI("ServoType: %d\n", system_config.getServoType());
  //USBSerial.println("HelloWorldUSBSerial");
  avatar.init();
  
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
    // 初期位置へ戻ります。
    servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree, system_config.getServoInfo(AXIS_Y)->start_degree, 2000);
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
    //mumumuServo(); // 左右に高速で首を振ります。（サーボが壊れるのであまり使わないでください。）
    moveRandom(); // ランダムモードになります。
  }

  if ((millis() - last_mouth_millis) > mouth_wait) {
    const char* l = lyrics[lyrics_idx++ % lyrics_size];
    avatar.setSpeechText(l);
    avatar.setMouthOpenRatio(0.7);
    delay(200);
    avatar.setMouthOpenRatio(0.0);
    last_mouth_millis = millis();
    if (!core_port_a) {
      avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
    }
  }
  // delayを50msec程度入れないとCoreS3でバッテリーレベルと充電状態がおかしくなる。
  delay(0);

}
