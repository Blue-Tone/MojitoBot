// 入力ピンが入力状態の時、出力ピンを出力する

const int inPin  = 11;  // 入力ピン
const int outPin = 12;  // 出力ピン

void setup() {
  pinMode(inPin, INPUT_PULLUP);  
  pinMode(outPin, OUTPUT);  
}

void loop() {
  if(HIGH == digitalRead(inPin)){
    digitalWrite(outPin, HIGH);
  }else{
    digitalWrite(outPin, LOW);
  }
  delay(100);
}
