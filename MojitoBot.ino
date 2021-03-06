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
#define SELECT_NEO_PIXEL_PIN 5  // 選択状態表示用ネオピクセルピン 

// ラム用モーター調整用ボタン
#define MOTOR_CAL_A 11
#define MOTOR_CAL_B 12

#define SODA_ADD_BTN_PIN 13 // ソーダ追加ボタンピン
#define SODA_SENSOR_START A8 // ソーダ残量センサー開始ピン

// ラム用モーターピン
#define RUM_MOTOR_A A0
#define RUM_MOTOR_B A1

#define RUM_NUMPIXELS    60
#define PANEL_NUMPIXELS  16
#define SODA_NUMPIXELS    8
#define SELECT_NUMPIXELS  2

Adafruit_NeoPixel rumPixels = Adafruit_NeoPixel(RUM_NUMPIXELS, RUM_NEO_PIXEL_PIN, NEO_GRB);
Adafruit_NeoPixel panelPixels = Adafruit_NeoPixel(PANEL_NUMPIXELS, PANEL_NEO_PIXEL_PIN, NEO_GRB);
Adafruit_NeoPixel sodaPixels  = Adafruit_NeoPixel(SODA_NUMPIXELS, SODA_NEO_PIXEL_PIN, NEO_GRB);
Adafruit_NeoPixel selectPixels  = Adafruit_NeoPixel(SELECT_NUMPIXELS, SELECT_NEO_PIXEL_PIN, NEO_GRB);

//#define LED_MAX 1

#define LED_MAX 255
//#define LED_MAX 50
//#define LED_LOW 10
#define LED_LOW 255
#define NP_BLUE  rumPixels.Color(0, 0, LED_MAX)
#define NP_GREEN rumPixels.Color(0, LED_MAX, 0)
#define NP_WHITE rumPixels.Color(LED_MAX, LED_MAX, LED_MAX)
#define NP_OFF   rumPixels.Color(0, 0, 0)

#define NP_BLUE_LOW  rumPixels.Color(0, 0, LED_LOW)
#define NP_GREEN_LOW rumPixels.Color(0, LED_LOW, 0)
#define NP_WHITE_LOW rumPixels.Color(LED_LOW, LED_LOW, LED_LOW)


// 時間関連
//#define RUM_TIME          2500  // ラム用モーターの動作時間
#define RUM_TIME          1800  // ラム用モーターの動作時間
#define RUM_WAIT_TIME     3000  // ラム用モーターの２回目前の待ち時間（ワンショットメジャー補充時間）
//#define RUM_RELEASE_TIME  1800  // ラム用モータの戻す動作時間
#define RUM_RELEASE_TIME  1000  // ラム用モータの戻す動作時間
//#define SODA_TIME         1400 // ソーダ用の動作時間 // 水は1400
#define SODA_TIME         1900 // ソーダ用の動作時間 // 水は1400
#define WAIT_TIME         1000  // 動作の間の時間

#define RUM_POW 180
#define RUM_REL_POW 100

#define MODE_SINGLE true
#define MODE_DOUBLE false
int  mode         = MODE_SINGLE; // シングル/ダブルモード
bool oldIsSelPush = HIGH; // 前回選択ボタン状態
bool oldIsOkPush  = HIGH; // 前回OKボタン状態

int ledCount = 0;         // 流れるLED用カウンタ
enum enumStatus{
  STATE_IDLE,
  STATE_RUM,
  STATE_SODA,
};

int state = STATE_IDLE;

byte r = 0;
byte g = 0;
byte b = 0;

// ソーダ量調整
#define SODA_ADJ_MAX 7
int sodaAdjCount = 0;
int sodaAdj[SODA_ADJ_MAX] = {16 , 32, 64, 128, 256, 512, 512};

// パネルNeoPixel用
int panelTarget = 0;
int panelLedValList[PANEL_NUMPIXELS] = {0};
int panelLedSpeed = 16;

// タイマ割り込みコールバック
void flash() {
//  Serial.println(state);
  
  ledCount++;
  if(RUM_NUMPIXELS < ledCount) ledCount = 0;

  switch(state){
    case STATE_IDLE:
      // ラムLedをゆっくり流す
      for(int i=0; i < RUM_NUMPIXELS; i++){
        if(i % 20 == ledCount % 20){
          rumPixels.setPixelColor(i, NP_GREEN_LOW);  
        }else{
          rumPixels.setPixelColor(i, NP_WHITE_LOW);  
        }
      }
      rumPixels.show();
      break;
    case STATE_RUM:
      // ラムLedをたくさん流す
      for(int i=0; i < RUM_NUMPIXELS; i++){
        if(i % 4 == ledCount % 4){
          rumPixels.setPixelColor(i, NP_GREEN_LOW);  
        }else{
          rumPixels.setPixelColor(i, NP_WHITE_LOW);  
        }
      }
      rumPixels.show();
      break;
    case STATE_SODA:
      Serial.print(r);
      Serial.print(",");
      Serial.println(b);
      // ソーダLedを色変化
      if(0 == r) b = b+32;
      if(0 == b) r = r+32;
      for(int i=0; i < SODA_NUMPIXELS; i++){
        sodaPixels.setPixelColor(i, sodaPixels.Color(r, 0, b));  
      }
      sodaPixels.show();
      break;
    default:
      break;
  }  

  // パネルの色設定　もわっとランダムに
  bool isDone = true;
  for(int i=0; i < PANEL_NUMPIXELS; i++){
    // ターゲットが白。その周りは半分づつ暗くなる
    // 緑は常にMAX。減るのは早い。増えるのゆっくり設定

    int pos = abs(i-panelTarget);         // ターゲットからの距離
    int targetVal = LED_MAX / (pos + 1);  // 目標の色

    // ターゲットの色になっていなかったら色設定
    if(targetVal != panelLedValList[i]){
      if(targetVal > panelLedValList[i]){
        // 明るくするのはゆっくり
        panelLedValList[i] = panelLedValList[i] + panelLedSpeed / 2;
      }else{
        // 暗くするのは早く
        panelLedValList[i] = panelLedValList[i] - panelLedSpeed;
      }
      // 差が変化より小さい場合はターゲットと同じにする。
      if( abs(targetVal - panelLedValList[i])-1 < panelLedSpeed) panelLedValList[i] = targetVal;
      if(targetVal != panelLedValList[i]) isDone = false;// ターゲットに達していない場合、未完了に
    }

/*    Serial.print(i);
    Serial.print("Done=");
    Serial.print(isDone);
    Serial.print(",targ");
    Serial.print(targetVal);
    Serial.print(",List");
    Serial.println(panelLedValList[i]);
*/    
    panelPixels.setPixelColor(i, panelPixels.Color(panelLedValList[i], LED_MAX, panelLedValList[i]));  
  }
  panelPixels.show();
  
  // 完了なら次のターゲットとスピード設定
  if(isDone){
    panelLedSpeed = random(4,32);
//    panelLedSpeed = random(4,16);
    panelTarget = random(0,PANEL_NUMPIXELS-1);
  }
/*    Serial.print("Speed=");
    Serial.print(panelLedSpeed);

    Serial.print(", Target=");
    Serial.println(panelTarget);  
*/
}

void setup() {
  Serial.begin(115200);
  Serial.println("start setup()");
  
  // ピン設定
  pinMode(SEL_BTN_PIN, INPUT_PULLUP);
  pinMode(OK_BTN_PIN, INPUT_PULLUP);
  pinMode(SODA_PIN, OUTPUT);
  digitalWrite(SODA_PIN, HIGH);
  pinMode(TONE_PIN, OUTPUT);
  pinMode(RUM_MOTOR_A, OUTPUT);
  pinMode(RUM_MOTOR_B, OUTPUT);
  digitalWrite(RUM_MOTOR_A, LOW);
  digitalWrite(RUM_MOTOR_B, LOW);

  pinMode(SODA_ADD_BTN_PIN, INPUT_PULLUP);
  pinMode(MOTOR_CAL_A, INPUT_PULLUP);
  pinMode(MOTOR_CAL_B, INPUT_PULLUP);

//  for(int i=0; i < SODA_ADJ_MAX; i++){
//    pinMode(SODA_SENSOR_START+i, INPUT);
//  }
  // I2Cアドレスは使用するディスプレイに合わせて変更する
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  

  // NeoPixel
  rumPixels.begin();
  panelPixels.begin();
  sodaPixels.begin();
  selectPixels.begin();

  for(int i=0; i < RUM_NUMPIXELS; i++){
    rumPixels.setPixelColor(i, NP_GREEN);  
  }
  rumPixels.show();
  
  for(int i=0; i < PANEL_NUMPIXELS; i++){
    panelPixels.setPixelColor(i, NP_GREEN);  
  }
  panelPixels.show();
  
  for(int i=0; i < SODA_NUMPIXELS; i++){
    sodaPixels.setPixelColor(i, NP_BLUE);  
  }
  sodaPixels.show();

  // シングルを光らせる
  selectPixels.setPixelColor(1, NP_GREEN);  
  selectPixels.setPixelColor(0, NP_OFF);  
  selectPixels.show();
  
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
    if(MODE_SINGLE == mode){
      mode = MODE_DOUBLE;
      // ダブルを光らせる
      selectPixels.setPixelColor(1, NP_OFF);  
      selectPixels.setPixelColor(0, NP_GREEN);  
    }else{
      mode = MODE_SINGLE;
      // シングルを光らせる
      selectPixels.setPixelColor(1, NP_GREEN);  
      selectPixels.setPixelColor(0, NP_OFF);  
    }
    selectPixels.show();

  }
  oldIsSelPush = sel; // 前回のボタン状態を保持
  
  // ディスプレイ------
  // OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);

    Serial.println("1-2-3-4-5-6-7");
    // ソーダ残量確認
    sodaAdjCount = 0;
    for(int i=0; i < SODA_ADJ_MAX; i++){
//      bool val = digitalRead(SODA_SENSOR_START+i);
      int val = analogRead(SODA_SENSOR_START+i);
//      if(HIGH != val ){
      if(200 > val ){
        sodaAdjCount = i;
        display.print(0);
        Serial.print(val);
        Serial.print("-");
  //      break;
      }else{
        display.print(1);
        Serial.print(val);
        Serial.print("-");
      }
    }
    Serial.println("");
    
    display.setTextSize(2);
    display.print(":");
    display.print(sodaAdjCount);
    display.print(":");
    display.println(sodaAdj[sodaAdjCount]);
    
    display.setTextSize(1);
    display.println("");
    display.setTextSize(2);
    if(MODE_SINGLE == mode){
      display.setTextColor(BLACK, WHITE); // 反転
    }else{
      display.setTextColor(WHITE);
    }
    display.println("-Single");
    display.setTextSize(1);
    display.println("");
    display.setTextSize(2);
  
    if(MODE_SINGLE != mode){
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
    // ソーダの調整カウンタを進める
//    sodaAdjCount++;
//    if(SODA_ADJ_MAX == sodaAdjCount) sodaAdjCount = 0;

    tone(TONE_PIN, 880, 200);
    state = STATE_RUM;
    // 押下時
    Serial.println("push OK");
    rumPixels.setPixelColor(0, NP_GREEN);  
    rumPixels.show();

    // モーター動作（ラムのワンショットメジャー用)
//    digitalWrite(RUM_MOTOR_A, HIGH);
    analogWrite(RUM_MOTOR_A, RUM_POW);
    tone(TONE_PIN, 880, 100);
    delay(RUM_TIME);
    digitalWrite(RUM_MOTOR_A, LOW);
    tone(TONE_PIN, 880, 100);

    delay(RUM_WAIT_TIME);
    
    // ラムのモーターを逆回転し、戻す
    digitalWrite(RUM_MOTOR_B, RUM_REL_POW);
    tone(TONE_PIN, 880, 100);
    delay(RUM_RELEASE_TIME);
    digitalWrite(RUM_MOTOR_B, LOW);

    if(!mode){
      // ダブルは2回
      tone(TONE_PIN, 880, 100);
      delay(RUM_WAIT_TIME);
//      digitalWrite(RUM_MOTOR_A, HIGH);
      analogWrite(RUM_MOTOR_A, RUM_POW);
      tone(TONE_PIN, 880, 100);
      delay(RUM_TIME);
      digitalWrite(RUM_MOTOR_A, LOW);
      tone(TONE_PIN, 880, 100);
      delay(RUM_WAIT_TIME);
      tone(TONE_PIN, 880, 100);

      // ラムのモーターを逆回転し、戻す
      digitalWrite(RUM_MOTOR_B, RUM_REL_POW);
      delay(RUM_RELEASE_TIME);
      digitalWrite(RUM_MOTOR_B, LOW);
    }
    
    tone(TONE_PIN, 880, 200);
    Serial.println(state);
    state = STATE_SODA;
    Serial.println(state);
    
    // リレー（エアーポンプ制御用)
    digitalWrite(SODA_PIN, LOW);
    delay(SODA_TIME + sodaAdj[sodaAdjCount]);
    digitalWrite(SODA_PIN, HIGH);
    delay(1000);
    tone(TONE_PIN, 880, 500);
    state = STATE_IDLE;
    delay(50);
    for(int i=0; i < SODA_NUMPIXELS; i++){
      sodaPixels.setPixelColor(i, NP_BLUE);  
    }
    sodaPixels.show();

  }

  oldIsOkPush = sel; // 前回のボタン状態を保持

  // ソーダ追加
  if(LOW == digitalRead(SODA_ADD_BTN_PIN)){
    Serial.println("add soda");
    digitalWrite(SODA_PIN, LOW);
    delay(100);
    digitalWrite(SODA_PIN, HIGH);
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
