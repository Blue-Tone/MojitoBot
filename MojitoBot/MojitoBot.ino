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

int state = 1; // 
bool oldIsPush = HIGH; // 前回ボタン状態

/* 
ボタン
OLEDディスプレイ

サーボモーター（ラムのワンショットメジャー用)
リレー（エアーポンプ制御用)
*/

void setup() {
  Serial.begin(9600);
  Serial.println("start setup()");
  
  // ピン設定
//  pinMode(LED_PWM_PIN, OUTPUT);
  pinMode(SEL_BTN_PIN, INPUT_PULLUP);
  pinMode(OK_BTN_PIN, INPUT_PULLUP);

  // I2Cアドレスは使用するディスプレイに合わせて変更する
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  

}

void loop() {

  // 選択ボタン
  bool sel = digitalRead(SEL_BTN_PIN);
  if(HIGH == oldIsPush && LOW == sel){
    if(state){                      // true == trueと同じ。
      state = false;
    }else{
      state = true;
    }
  }
  oldIsPush = sel; // 前回のボタン状態を保持
  
  // 画面表示をクリア
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
  delay(100);
}