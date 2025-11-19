
#include <Arduino.h>
#include <Ticker.h>
#include <SPI.h>

// ---- ピン定義 ----
constexpr uint8_t PIN_SCK  = D8;
constexpr uint8_t PIN_MISO = D9;
constexpr uint8_t PIN_MOSI = D10;
constexpr uint8_t DRIVER_SDI  = D10;  // データ入力（SDI）
constexpr uint8_t DRIVER_SCLK = D8;   // シリアルクロック（SCLK）
constexpr uint8_t DRIVER_SDO  = D9;   // 
constexpr uint8_t DRIVER_LE   = 17;   // ラッチイネーブル（LE）
constexpr uint8_t DRIVER_OE   = 16;   // 出力イネーブル（OE, Low で有効）
constexpr uint8_t BATT = 0;
constexpr uint8_t LED_R = 22;
constexpr uint8_t LED_G = 23;

Ticker tickerBatt;
Ticker tickerLED;

bool flicker = true;
void onTickLED() {
  if (flicker == true){
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
  }
  else{
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
  }
  flicker = !flicker;
}

void onTickBatt() {
  float Vbattf = readBatteryVolt();
  Serial.println(Vbattf, 3); 
}

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

void setup() {
  pinMode(BATT,  INPUT); 
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(DRIVER_LE,   OUTPUT);
  pinMode(DRIVER_OE,   OUTPUT);
  digitalWrite(DRIVER_OE, HIGH); // 無効化してから

  SPI.begin(DRIVER_SCLK, DRIVER_SDO, DRIVER_SDI);
  digitalWrite(DRIVER_OE, LOW);  // 有効化
  writeTLC5917_spi(0x00);
  delay(100);
  writeTLC5917_spi(0xFF); // 全点灯

  tickerBatt.attach_ms(2000, onTickBatt);
  tickerLED.attach_ms(500, onTickLED);

  //init serial
  Serial.begin(115200);
  delay(100);
  Serial.println("setup complete");
}

uint8_t pattern = 0x01;
void loop() {
}