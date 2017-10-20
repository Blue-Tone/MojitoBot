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

#define SEL_BTN_PIN         2   // 選択ボタンピン
#define OK_BTN_PIN          3   // 決定ボタンピン

#define TONE_PIN            6   // トーン用ピン
#define SODA_PIN            7   // ソーダ用エアーポンプリレーピン
#define RUM_NEO_PIXEL_PIN   8   // ラム用ネオピクセルピン
#define PANEL_NEO_PIXEL_PIN 9   // パネル用ネオピクセルピン
#define SODA_NEO_PIXEL_PIN  10  // ソーダ用ネオピクセルピン

// ラム用モーター調整用ボタン
#define MOTOR_CAL_A 11
#define MOTOR_CAL_B 12

#define SODA_ADD_BTN_PIN 13 // ソーダ追加ボタンピン

// ラム用モーターピン
#define RUM_MOTOR_A A0
#define RUM_MOTOR_B A1

#define RUM_NUMPIXELS   60
#define PANEL_NUMPIXELS 16
#define SODA_NUMPIXELS  32

Adafruit_NeoPixel rumPixels   = Adafruit_NeoPixel(RUM_NUMPIXELS, RUM_NEO_PIXEL_PIN, NEO_GRB);
Adafruit_NeoPixel panelPixels = Adafruit_NeoPixel(PANEL_NUMPIXELS, PANEL_NEO_PIXEL_PIN, NEO_GRB);
Adafruit_NeoPixel sodaPixels  = Adafruit_NeoPixel(SODA_NUMPIXELS, SODA_NEO_PIXEL_PIN, NEO_GRB);

#define NP_BLUE  rumPixels.Color(5, 0, 0)
#define NP_GREEN rumPixels.Color(0, 5, 0)
#define NP_WHITE rumPixels.Color(5, 5, 5)

// 時間関連
#define RUM_TIME          4500  // ラム用モーターの動作時間
#define RUM_WAIT_TIME     4000  // ラム用モーターの２回目前の待ち時間（ワンショットメジャー補充時間）
#define RUM_RELEASE_TIME  2000  // ラム用モータの戻す動作時間
#define SODA_TIME         25000 // ソーダ用の動作時間
#define WAIT_TIME         1000  // 動作の間の時間

int  state        = true; // シングル/ダブル状態
bool oldIsSelPush = HIGH; // 前回選択ボタン状態
bool oldIsOkPush  = HIGH; // 前回OKボタン状態

int ledCount = 0;         // 流れるLED用カウンタ

// タイマ割り込みコールバック
void flash() {
//  Serial.println(ledCount);
  ledCount++;
  if(RUM_NUMPIXELS < ledCount) ledCount = 0;
  
  // NeoPixel制御
  for(int i=0; i < RUM_NUMPIXELS; i++){
    if(i == ledCount){
      rumPixels.setPixelColor(i, NP_GREEN);  
    }else{
      rumPixels.setPixelColor(i, NP_WHITE);  
    }
  }
  rumPixels.show();
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

  // NeoPixel
  rumPixels.begin();
  panelPixels.begin();
  sodaPixels.begin();

  for(int i=0; i < RUM_NUMPIXELS; i++){
    rumPixels.setPixelColor(i, NP_GREEN);  
  }
  rumPixels.show();
  
  for(int i=0; i < PANEL_NUMPIXELS; i++){
    panelPixels.setPixelColor(i, NP_GREEN);  
  }
  panelPixels.show();
  
  for(int i=0; i < SODA_NUMPIXELS; i++){
    sodaPixels.setPixelColor(i, NP_GREEN);  
  }
  sodaPixels.show();

  // タイマー割り込み
  MsTimer2::set(50, flash);
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
  
  // ディスプレイ------
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
  // ディスプレイ------

  // OKボタン
  sel = digitalRead(OK_BTN_PIN);
  if(HIGH == oldIsOkPush && LOW == sel){
    tone(TONE_PIN, 880, 200);
    // 押下時
    Serial.println("push OK");
    rumPixels.setPixelColor(0, NP_GREEN);  
    rumPixels.show();

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
    
    rumPixels.setPixelColor(0, NP_BLUE);  
    rumPixels.show();

    // リレー（エアーポンプ制御用)
    digitalWrite(SODA_PIN, HIGH);
    delay(SODA_TIME);
    digitalWrite(SODA_PIN, LOW);
    tone(TONE_PIN, 880, 500);
//    rumPixels.setPixelColor(0, );  
//    rumPixels.show();
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
