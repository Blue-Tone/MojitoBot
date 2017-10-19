#include<Servo.h>
Servo servoRUM; // ラム用サーボ

#include <MsTimer2.h>// タイマー割り込み
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
#define TONE_PIN 6         // トーン用ピン
#define SODA_PIN 7         // ソーダ用ピン
#define NEO_PIXEL_PIN 9    // ネオピクセル用ピン
#define SODA_ADD_BTN_PIN 12 // ソーダ追加ピン

// ラム用モーターピン
#define RUM_MOTOR_A A0
#define RUM_MOTOR_B A1

// ラム用モーター調整用ボタン
#define MOTOR_CAL_A 10
#define MOTOR_CAL_B 11

#define NUMPIXELS 16
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, STATE_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEO_PIXEL_PIN, NEO_GRB);

#define RUM_TIME 4500      // ラム用モーターの動作時間
#define RUM_WAIT_TIME 4000 // ラム用モーターの２回目前の待ち時間（ワンショットメジャー補充時間）
#define RUM_RELEASE_TIME 2000 // ラム用モータの戻す動作時間
#define SODA_TIME 25000    // ソーダ用の動作時間
#define WAIT_TIME 1000     // 動作の間の時間

int state = 1; // 
bool oldIsSelPush = HIGH; // 前回選択ボタン状態
bool oldIsOkPush = HIGH;  // 前回OKボタン状態

int ledCount = 0;

// タイマ割り込みコールバック
void flash() {
  Serial.println(ledCount);
  ledCount++;
  if(NUMPIXELS < ledCount) ledCount = 0;
  
  // NeoPixel制御
  for(int i=0; i < NUMPIXELS; i++){
    if(i == ledCount){
      pixels.setPixelColor(i, pixels.Color(0, 10, 0));  
    }else{
      pixels.setPixelColor(i, pixels.Color(10, 10, 10));  
    }
  }
  pixels.show();
  
}

void setup() {
  Serial.begin(9600);
  Serial.println("start setup()");
  
  // ピン設定
  pinMode(SEL_BTN_PIN, INPUT_PULLUP);
  pinMode(OK_BTN_PIN, INPUT_PULLUP);
  pinMode(SODA_PIN, OUTPUT);
  digitalWrite(SODA_PIN, LOW);
  pinMode(TONE_PIN, OUTPUT);
  pinMode(RUM_MOTOR_A, OUTPUT);
  pinMode(RUM_MOTOR_B, OUTPUT);
  digitalWrite(RUM_MOTOR_A, LOW);
  digitalWrite(RUM_MOTOR_B, LOW);

  pinMode(SODA_ADD_BTN_PIN, INPUT_PULLUP);
  pinMode(MOTOR_CAL_A, INPUT_PULLUP);
  pinMode(MOTOR_CAL_B, INPUT_PULLUP);

  // I2Cアドレスは使用するディスプレイに合わせて変更する
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  

  servoRUM.attach(RUM_PIN);
  servoRUM.write(1);//初期位置へ

  // NeoPixel
  pixels.begin();
  for(int i=0; i < NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(0, 10, 0));  
    Serial.println(i);
  }
  pixels.show();

  // タイマー割り込み
  MsTimer2::set(100, flash);
  MsTimer2::start();
}

void loop() {
  delay(100);

  // 選択ボタン
  bool sel = digitalRead(SEL_BTN_PIN);
  if(HIGH == oldIsSelPush && LOW == sel){
    // ラムのモーターを逆回転し、戻す
//    digitalWrite(RUM_MOTOR_A, HIGH);
//    delay(1000);
//    digitalWrite(RUM_MOTOR_A, LOW);
    
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
  display.println("   RUM");
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
    pixels.setPixelColor(0, pixels.Color(100, 0, 0));  
    pixels.show();

    // モーター動作（ラムのワンショットメジャー用)
    digitalWrite(RUM_MOTOR_A, HIGH);
    delay(RUM_TIME);
    digitalWrite(RUM_MOTOR_A, LOW);
    delay(3000);

    // ラムのモーターを逆回転し、戻す
    digitalWrite(RUM_MOTOR_B, HIGH);
    delay(RUM_RELEASE_TIME);
    digitalWrite(RUM_MOTOR_B, LOW);

    if(!state){
      // ダブルは2回
      delay(RUM_WAIT_TIME);
      digitalWrite(RUM_MOTOR_A, HIGH);
      delay(RUM_TIME);
      digitalWrite(RUM_MOTOR_A, LOW);
      delay(WAIT_TIME);
      delay(3000);

      // ラムのモーターを逆回転し、戻す
      digitalWrite(RUM_MOTOR_B, HIGH);
      delay(RUM_RELEASE_TIME);
      digitalWrite(RUM_MOTOR_B, LOW);
    }
    
    tone(TONE_PIN, 880, 200);
    
    pixels.setPixelColor(0, pixels.Color(0, 0, 100));  
    pixels.show();

    // リレー（エアーポンプ制御用)
    digitalWrite(SODA_PIN, HIGH);
    delay(SODA_TIME);
    digitalWrite(SODA_PIN, LOW);
    tone(TONE_PIN, 880, 500);
    pixels.setPixelColor(0, pixels.Color(1, 1, 1));  
    pixels.show();
  }

  oldIsOkPush = sel; // 前回のボタン状態を保持

  // ソーダ追加
  if(LOW == digitalRead(SODA_ADD_BTN_PIN)){
    Serial.println("add soda");
    digitalWrite(SODA_PIN, HIGH);
    delay(100);
    digitalWrite(SODA_PIN, LOW);
  }

  // モーター調整
  if(LOW == digitalRead(MOTOR_CAL_A)){
    Serial.println("MOTOR_CAL_A");
    digitalWrite(RUM_MOTOR_A, HIGH);
    delay(100);
    digitalWrite(RUM_MOTOR_A, LOW);
  }
  
  if(LOW == digitalRead(MOTOR_CAL_B)){
    Serial.println("MOTOR_CAL_B");
    digitalWrite(RUM_MOTOR_B, HIGH);
    delay(100);
    digitalWrite(RUM_MOTOR_B, LOW);
  }
}
