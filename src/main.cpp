/*
 * Stack-chan-tester
 * 
 * @author TakaoAkaki
 * Copyright (c) 2021-2024 Takao Akaki. All right reserved
 */

// ------------------------
// ヘッダファイルのinclude
// 
#include <Arduino.h>                         // Arduinoフレームワークを使用する場合は必ず必要
#include <SD.h>                              // SDカードを使うためのライブラリです。
#include <Update.h>                          // 定義しないとエラーが出るため追加。
#include <Ticker.h>                          // 定義しないとエラーが出るため追加。
#include <M5StackUpdater.h>                  // M5Stack SDUpdaterライブラリ
#include <M5Unified.h>                       // M5Unifiedライブラリ
#include <Stackchan_system_config.h>         // stack-chanの初期設定ファイルを扱うライブラリ
#include <Stackchan_servo.h>                 // stack-chanのサーボを動かすためのライブラリ
#ifdef ARDUINO_M5STACK_CORES3
#include <gob_unifiedButton.hpp>
goblib::UnifiedButton unifiedButton;         // M5CoreS3のタッチパネルをボタンA,B,Cとして使うためのライブラリ。
#endif
#include <Avatar.h>                          // 顔を表示するためのライブラリ https://github.com/meganetaaan/m5stack-avatar
#include "formatString.hpp"                  // 文字列に変数の値を組み込むために使うライブラリ https://gist.github.com/GOB52/e158b689273569357b04736b78f050d6
// ヘッダファイルのinclude end 
// ================================== End

// ---------------------------------------------
// グローバル変数の定義エリア
// プログラム全体で利用する変数やクラスを決める
int servo_offset_x = 0;  // X軸サーボのオフセット（サーボの初期位置からの+-で設定）
int servo_offset_y = 0;  // Y軸サーボのオフセット（サーボの初期位置からの+-で設定）

using namespace m5avatar;     // (Avatar.h)avatarのnamespaceを使う宣言（こうするとm5avatar::???と書かなくて済む。)
Avatar avatar;                // (Avatar.h)avatarのクラスを定義
ColorPalette *cps[2];

#define SDU_APP_PATH "/stackchan_tester.bin"  // (SDUpdater.h)SDUpdaterで使用する変数
#define TFCARD_CS_PIN 4                       // SDカードスロットのCSPIN番号

StackchanSERVO servo;                         // (Stackchan_servo.h) サーボを扱うためのクラス
StackchanSystemConfig system_config;          // (Stackchan_system_config.h) プログラム内で使用するパラメータをYAMLから読み込むクラスを定義

uint32_t mouth_wait = 2000;       // 通常時のセリフ入れ替え時間（msec）
uint32_t last_mouth_millis = 0;   // セリフを入れ替えた時間
bool core_port_a = false;         // Core1のPortAを使っているかどうか

const char* lyrics[] = { "BtnA:MoveTo90  ", "BtnB:ServoTest  ", "BtnC:RandomMode  ", "BtnALong:AdjustMode"};  // 通常モード時に表示するセリフ
const int lyrics_size = sizeof(lyrics) / sizeof(char*);  // セリフの数
int lyrics_idx = 0;                                      // 表示するセリフ数用の変数
// ================================== End

// ------------------------------------------------
// Stack-chan-testerの各モードを定義するエリア
// adjustOffset(): オフセットを調べるモード
// moveRandom()  : ランダムでStack-chanが動くモード
// testServo()   : サーボをテストするときのモード

// オフセットの設定モード(loop()でBtnAが押されると実行されます。)
void adjustOffset() {
  // サーボのオフセットを調整するモード
  // 関数内のみで使用する変数の初期化
  servo_offset_x = 0;
  servo_offset_y = 0;
  servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree, system_config.getServoInfo(AXIS_Y)->start_degree, 2000);  // サーボを中央にセット
  bool adjustX = true; // X軸とY軸のモードを切り替えるためのフラグ
  for (;;) { // 無限ループ(BtnBが長押しされるまで繰り返します。)
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
      // 調整モードを終了(loop()へ戻る)
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
    // オフセットを反映した位置にサーボを移動
    servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree + servo_offset_x, system_config.getServoInfo(AXIS_Y)->start_degree + servo_offset_y, 1000);

    std::string s;

    if (adjustX) {
      // X軸の値を設定する。
      s = formatString("%s:%d:BtnB:X/Y", "X", servo_offset_x); // 吹き出し用のStringを作成
    } else {
      // Y軸の値を設定する。
      s = formatString("%s:%d:BtnB:X/Y", "Y", servo_offset_y); // 吹き出し用のStringを作成
    }
    // 
    avatar.setSpeechText(s.c_str()); // 作成したStringをChar型に変換して吹き出しに表示する。

  }
}

// ランダムモード(loop()でBtnCが押されると実行されます。)
void moveRandom() {
  for (;;) { // 無限ループ（BtnCが押されるまでランダムモードを繰り返します。
    // ランダムモード
    int x = random(system_config.getServoInfo(AXIS_X)->lower_limit + 45, system_config.getServoInfo(AXIS_X)->upper_limit - 45);  // 可動範囲の下限+45〜上限-45 でランダム
    int y = random(system_config.getServoInfo(AXIS_Y)->lower_limit, system_config.getServoInfo(AXIS_Y)->upper_limit);            // 可動範囲の下限〜上限 でランダム
#ifdef ARDUINO_M5STACK_CORES3
    unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
    M5.update();
    if (M5.BtnC.wasPressed()) {
      // ランダムモードを抜ける処理。(loop()に戻ります。)
      break;
    }
    uint16_t base_delay_time = 0;
    if (system_config.getServoType() == ServoType::SCS || system_config.getServoType() == ServoType::DYN_XL330) {
      base_delay_time = 500;
    }
    int delay_time = random(10);
    servo.moveXY(x, y, 1000 + (100 + base_delay_time) * delay_time);
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
  // サーボのテストの動き
  for (int i=0; i<2; i++) { // 同じ動きを2回繰り返す。
    avatar.setSpeechText("X center -> left  ");
    servo.moveX(system_config.getServoInfo(AXIS_X)->lower_limit, 1000); // 初期位置から左へ向く
    avatar.setSpeechText("X left -> right  ");
    servo.moveX(system_config.getServoInfo(AXIS_X)->upper_limit, 3000); // 左へ向いたあと右へ向く
    avatar.setSpeechText("X right -> center  ");
    servo.moveX(system_config.getServoInfo(AXIS_X)->start_degree, 1000); // 前を向く
    avatar.setSpeechText("Y center -> lower  ");
    servo.moveY(system_config.getServoInfo(AXIS_Y)->lower_limit, 1000); // 上を向く
    avatar.setSpeechText("Y lower -> upper  ");
    servo.moveY(system_config.getServoInfo(AXIS_Y)->upper_limit, 1000); // 下を向く
    avatar.setSpeechText("Initial Pos.");
    servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree, system_config.getServoInfo(AXIS_Y)->start_degree, 1000); // 初期位置に戻る（正面を向く)
  }
}

// ぼっちちゃんのむむむを再現するために遊びで作った関数。
// 使わないでください。
void mumumuServo() {
  for (int i=0; i<30; i++) {
    servo.moveX(120, 250);
    servo.moveX(240, 250);
  }
}
// ============================================== End


// ------------------------------------------------------------------
// Arduinoフレームワークで一番最初に実行される関数の定義
// void setup()とvoid loop()は必ず必要です。
// void setup()は、最初に1回だけ実行します。
void setup() {
  auto cfg = M5.config();       // 設定用の情報を抽出
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);                // M5Stackをcfgの設定で初期化
#ifdef ARDUINO_M5STACK_CORES3 // CORES3のときに有効になります。
  // 画面上のタッチパネルを3分割してBtnA, B, Cとして使う設定。
  unifiedButton.begin(&M5.Display, goblib::UnifiedButton::appearance_t::transparent_all);
#endif
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);    // M5Unifiedのログ初期化（画面には表示しない。)
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);     // M5Unifiedのログ初期化（シリアルモニターにESP_LOG_INFOのレベルのみ表示する)
  M5.Log.setEnableColor(m5::log_target_serial, false);         // M5Unifiedのログ初期化（ログをカラー化しない。）
  M5_LOGI("Hello World");                                      // logにHello Worldと表示
  SD.begin(GPIO_NUM_4, SPI, 25000000);                         // SDカードの初期化
  delay(2000);                                                 // SDカードの初期化を少し待ちます。
 
  system_config.loadConfig(SD, "");                            // SDカードから初期設定ファイルを読み込む
  if (M5.getBoard() == m5::board_t::board_M5Stack) {           // Core1かどうかの判断
    if (system_config.getServoInfo(AXIS_X)->pin == 22) {       // サーボのGPIOが22であるか確認（本当は21も確認してもいいかもしれないが省略）
      // M5Stack Coreの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。バッテリー表示は不可。
      M5_LOGI("I2CRelease");              // ログに出力
      avatar.setBatteryIcon(false);       // avatarのバッテリーアイコンを表示しないモードに設定
      M5.In_I2C.release();                // I2Cを使わないように設定(IMUやIP5306との通信をしないようにします。)※設定しないとサーボの動きがおかしくなります。
      core_port_a = true;                 // Core1でポートAを使用しているフラグをtrueに設定
    }
  } else {
    avatar.setBatteryIcon(true);          // Core2以降の場合は、バッテリーアイコンを表示する。
  }
  // servoの初期化
  M5_LOGI("attach servo"); // ログへ出力
  // サーボの初期化を行います。（このとき、初期位置（正面）を向きます。）
  servo.begin(system_config.getServoInfo(AXIS_X)->pin, system_config.getServoInfo(AXIS_X)->start_degree,
              system_config.getServoInfo(AXIS_X)->offset,
              system_config.getServoInfo(AXIS_Y)->pin, system_config.getServoInfo(AXIS_Y)->start_degree,
              system_config.getServoInfo(AXIS_Y)->offset,
              (ServoType)system_config.getServoType());

  M5.Power.setExtOutput(!system_config.getUseTakaoBase());       // 設定ファイルのTakaoBaseがtrueの場合は、Groveポートの5V出力をONにする。

  M5_LOGI("ServoType: %d\n", system_config.getServoType());      // サーボのタイプをログに出力
  //USBSerial.println("HelloWorldUSBSerial");
  avatar.init();                   // avatarを初期化して実行開始します。(このときに顔が表示されます。)
  cps[0] = new ColorPalette();
  cps[0]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[0]->set(COLOR_BACKGROUND, TFT_WHITE);
  avatar.setColorPalette(*cps[0]);

  last_mouth_millis = millis();    // loop内で使用するのですが、処理を止めずにタイマーを実行するための変数です。一定時間で口を開くのとセリフを切り替えるのに利用します。
  //moveRandom();
  //testServo();
}


// メインのloop処理。（必ず定義が必要。）
// 基本的にはずっと繰り返し実行されます。
// stack-chan-testerの場合は、ボタンを押して、各モードの関数が実行されると一時停止します。
void loop() {
#ifdef ARDUINO_M5STACK_CORES3
  unifiedButton.update(); // M5.update() よりも前に呼ぶ事
#endif
  M5.update();  // M5Stackのボタン状態を更新します。
  if (M5.BtnA.pressedFor(2000)) {                // ボタンAを2秒長押しを検出したら。
    adjustOffset();                              // サーボのオフセットを調整するモードへ
  } else if (M5.BtnA.wasPressed()) {             // ボタンAが押された場合
    servo.moveXY(system_config.getServoInfo(AXIS_X)->start_degree, system_config.getServoInfo(AXIS_Y)->start_degree, 2000);  // サーボを初期位置へ変更
  }
  
  if (M5.BtnB.wasSingleClicked()) {               // ボタンBが1回クリックされたとき
    testServo();                                  // サーボのテストを実行します。
  } else if (M5.BtnB.wasDoubleClicked()) {        // ボタンBをダブルクリックしたとき
    // Groveポートの出力を切り替えます。
    if (M5.Power.getExtOutput() == true) { 
      M5.Power.setExtOutput(false);
      avatar.setSpeechText("ExtOutput Off");
    } else {
      M5.Power.setExtOutput(true);
      avatar.setSpeechText("ExtOutput On");
    }
    delay(2000);                  // 2秒待ちます。(吹き出しをしばらく表示させるため)
    avatar.setSpeechText("");     // 吹き出しの表示を消します。
  } 
  if (M5.BtnC.pressedFor(5000)) {                                    // ボタンCを5秒長押しした場合(この処理はあまり使わないでしょう。)
    M5_LOGI("Will copy this sketch to filesystem");                  // ログへ出力
    if (saveSketchToFS( SD, SDU_APP_PATH, TFCARD_CS_PIN )) {         // SDUpdater用のbinファイルをSDカードに書き込みます。
      M5_LOGI("Copy Successful!");
    } else {
      M5_LOGI("Copy failed!");
    }
  } else if (M5.BtnC.wasPressed()) {     // ボタンCが押された場合
    //mumumuServo();                     // 左右に高速で首を振ります。（サーボが壊れるのであまり使わないでください。）コメントなので実行されません。
    moveRandom();                        // ランダムモードになります。
  }

  if ((millis() - last_mouth_millis) > mouth_wait) {             // 口を開けるタイミングを待ちます。時間を図ってmouth_waitで設定した時間が経つと繰り返します。
    const char* l = lyrics[lyrics_idx++ % lyrics_size];          // セリフを取り出します。
    avatar.setSpeechText(l);                                     // 吹き出しにセリフを表示します。
    avatar.setMouthOpenRatio(0.7);                               // アバターの口を70%開きます。(70%=0.7)
    delay(200);                                                  // 200ミリ秒口を開いたまま
    avatar.setMouthOpenRatio(0.0);                               // 口を閉じます。(0%=0.0)
    last_mouth_millis = millis();                                // 実行時間を更新。（この時点からまたmouth_wait分待ちます。）
    if (!core_port_a) {                                                              // Core1でポートA「以外」のとき（!はnot）
      avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());    // バッテリー表示を更新します。
    }
  }

}
