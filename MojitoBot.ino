#include<Servo.h>
Servo servoRUM; // ラム用サーボ

#include <Adafruit_NeoPixel.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// コンパイル時にヘッダーファイルが適切に編集されていない場合に
// "Height incorrect, please fix Adafruit_SSD1306.h!"
// というエラーを表示するための記述
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// RSTピンがない互換品を使用するので-1を指定
Adafruit_SSD1306 display(-1);

#define SEL_BTN_PIN 2      // 選択ボタンピン
#define OK_BTN_PIN 3       // 決定ボタンピン
#define RUM_PIN 4          // ラム用サーボピン
#define SODA_PIN 5         // ソーダ用ピン
#define TONE_PIN 6         // トーン用ピン

#define RUM_TIME 2000      // ラム用サーボの動作時間
#define RUM_WAIT_TIME 2000 // ラム用サーボの２回目前の待ち時間（ワンショットメジャー補充時間）
#define SODA_TIME 3000     // ソーダ用の動作時間
#define WAIT_TIME 1000     // 動作の間の時間

int state = 1; // 
bool oldIsSelPush = HIGH; // 前回選択ボタン状態
bool oldIsOkPush = HIGH;  // 前回OKボタン状態

void setup() {
  Serial.begin(9600);
  Serial.println("start setup()");
  
  // ピン設定
  pinMode(SEL_BTN_PIN, INPUT_PULLUP);
  pinMode(OK_BTN_PIN, INPUT_PULLUP);
  pinMode(SODA_PIN, OUTPUT);
  digitalWrite(SODA_PIN, LOW);
  pinMode(TONE_PIN, OUTPUT);

  // I2Cアドレスは使用するディスプレイに合わせて変更する
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  

  servoRUM.attach(RUM_PIN);
  servoRUM.write(1);//初期位置へ
}

void loop() {
  delay(100);

  // 選択ボタン
  bool sel = digitalRead(SEL_BTN_PIN);
  if(HIGH == oldIsSelPush && LOW == sel){
    tone(TONE_PIN, 440, 100);
    if(state){
      state = false;
    }else{
      state = true;
    }
  }
  oldIsSelPush = sel; // 前回のボタン状態を保持
  
  // OLED
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("   RAM");
  display.setTextSize(1);
  display.println("");
  display.setTextSize(2);
  if(state){
    display.setTextColor(BLACK, WHITE); // 反転
  }else{
    display.setTextColor(WHITE);
  }
  display.println("-Single");
  display.setTextSize(1);
  display.println("");
  display.setTextSize(2);

  if(!state){
    display.setTextColor(BLACK, WHITE); // 反転
  }else{
    display.setTextColor(WHITE);
  }
  display.println("-Double");
  
  display.display();

  // OKボタン
  sel = digitalRead(OK_BTN_PIN);
  if(HIGH == oldIsOkPush && LOW == sel){
    tone(TONE_PIN, 880, 200);
    // 押下時
    Serial.println("push OK");

    // サーボモーター動作（ラムのワンショットメジャー用)
    servoRUM.write(179);
    delay(RUM_TIME);
    servoRUM.write(1);
    delay(WAIT_TIME);

    if(!state){
      // ダブルは2回
      servoRUM.write(179);
      delay(RUM_TIME);
      servoRUM.write(1);
      delay(WAIT_TIME);
    }
    tone(TONE_PIN, 880, 200);
    
    // リレー（エアーポンプ制御用)
    digitalWrite(SODA_PIN, HIGH);
    delay(SODA_TIME);
    digitalWrite(SODA_PIN, LOW);
    tone(TONE_PIN, 880, 500);
  }

  oldIsOkPush = sel; // 前回のボタン状態を保持    
}
