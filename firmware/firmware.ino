
#include <Arduino.h>
#include <Ticker.h>
#include <SPI.h>

constexpr uint8_t PIN_SCK  = D8;
constexpr uint8_t PIN_MISO = D9;
constexpr uint8_t PIN_MOSI = D10;

// ---- ピン定義 ----
constexpr uint8_t DRIVER_SDI  = D10;  // データ入力（SDI）
constexpr uint8_t DRIVER_SCLK = D8;   // シリアルクロック（SCLK）
constexpr uint8_t DRIVER_SDO  = D9;   // 
constexpr uint8_t DRIVER_LE   = 17;   // ラッチイネーブル（LE）
constexpr uint8_t DRIVER_OE   = 16;   // 出力イネーブル（OE, Low で有効）

constexpr uint8_t BATT = 0;

constexpr uint8_t LED_R = 22;
constexpr uint8_t LED_G = 23;



Ticker ticker;
bool flicker = true;
void onTick() {
  Serial.println("100ms タスク実行");
  if (flicker == true){
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
  }
  else{
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
  }
  flicker = !flicker;
  
  float Vbattf = readBatteryVolt();
  Serial.println(Vbattf, 3); 
}

// void writeTLC5917(uint8_t value) {
//   // LE を Low にしてシフトレジスタにデータを入れる
//   digitalWrite(DRIVER_LE, LOW);

//   // shiftOut は LSB/MSB の順番を指定できる。TLC5917 は MSB が OUT7 に対応。
//   shiftOut(DRIVER_SDI, DRIVER_SCLK, MSBFIRST, value);

//   // LE を High → Low とすることで出力ラッチを更新
//   digitalWrite(DRIVER_LE, HIGH);
//   delayMicroseconds(1);  // 最小ラッチ時間を確保
//   digitalWrite(DRIVER_LE, LOW);
// }

void writeTLC5917_spi(uint8_t value) {
  // 出力を一時無効化（任意）
  digitalWrite(DRIVER_OE, HIGH);
  digitalWrite(DRIVER_LE, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1MHzで十分
  SPI.transfer(value);
  SPI.endTransaction();

  // ラッチ
  digitalWrite(DRIVER_LE, HIGH);
  delayMicroseconds(1);
  digitalWrite(DRIVER_LE, LOW);

  digitalWrite(DRIVER_OE, LOW);
}

float readBatteryVolt(){
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt += analogReadMilliVolts(BATT); // Read and accumulate ADC voltage
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0;     // Adjust for 1:2 divider and convert to volts
  return Vbattf;
}

// void writeTLC5917_bitbang(uint8_t value) {
//   // 出力を一時無効化（安定化のため推奨）
//   digitalWrite(DRIVER_OE, HIGH);

//   digitalWrite(DRIVER_LE, LOW);

//   for (int i = 7; i >= 0; --i) { // MSBFIRST 相当
//     digitalWrite(DRIVER_SDI, (value >> i) & 0x01);
//     // ↑データ安定 -> 立ち上がりでサンプルされる想定（Mode0系）

//     // 明示的にクロックを出す
//     digitalWrite(DRIVER_SCLK, HIGH);
//     delayMicroseconds(10);   // 最低でも数百ns〜1us程度
//     digitalWrite(DRIVER_SCLK, LOW);
//     delayMicroseconds(10);
//   }

//   // ラッチ
//   digitalWrite(DRIVER_LE, HIGH);
//   delayMicroseconds(1);
//   digitalWrite(DRIVER_LE, LOW);

//   // 出力有効化
//   digitalWrite(DRIVER_OE, LOW);
// }

void setup() {
  pinMode(BATT, INPUT); 

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  
  // pinMode(DRIVER_SDI, OUTPUT);
  // pinMode(DRIVER_SCLK, OUTPUT);
  // pinMode(DRIVER_LE, OUTPUT);
  // pinMode(DRIVER_OE, OUTPUT);
  // pinMode(DRIVER_SDO, INPUT);

  // digitalWrite(DRIVER_SDI, LOW);
  // digitalWrite(DRIVER_SCLK, LOW);
  // digitalWrite(DRIVER_LE, LOW);
  // //activate output
  // digitalWrite(DRIVER_OE,   HIGH); // まず無効化
  // writeTLC5917_bitbang(0x00);              // 既知状態に
  // digitalWrite(DRIVER_OE, LOW);
  // writeTLC5917_bitbang(0x00);

  
  pinMode(DRIVER_LE,   OUTPUT);
  pinMode(DRIVER_OE,   OUTPUT);
  digitalWrite(DRIVER_OE, HIGH); // 無効化してから

  SPI.begin(DRIVER_SCLK, DRIVER_SDO, DRIVER_SDI); // ピン明示
  //SPI.begin();
  digitalWrite(DRIVER_OE, LOW);           // 有効化
  writeTLC5917_spi(0x00);                 // 既知化


  ticker.attach_ms(2000, onTick); // 100msごとに実行

  //init serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("setup complete");
}

uint8_t pattern = 0x01;
void loop() {

  // Serial.println("全灯");
  // writeTLC5917_spi(0xFF); // 全点灯
  // delay(1000);
  // Serial.println("消灯");
  // writeTLC5917_spi(0x00); // 全消灯
  // delay(1000);

  pattern <<= 1;
  if (pattern == 0) {
    pattern = 0x01;
  }
  writeTLC5917_spi(pattern);

  delay(100);
}