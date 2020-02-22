#include <Arduino.h>
#include <Wire.h>
#include <Si4735.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <SendOnlySoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

SendOnlySoftwareSerial mySerial(A0);
DFRobotDFPlayerMini myDFPlayer;
uint32_t timer = 0;

uint16_t STATION = 7310;
int volume = 15;

#define FIX_BAND    RADIO_BAND_FM    //Radio Band -FM
#define MP3_START   8
#define MP3_STOP    13
#define VOL_PLUS    9
#define VOL_MINUS   10
#define FREQ_PLUS   11
#define FREQ_MINUS  12
#define RESET       A3

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

char lcdBuff[16] = {0};
bool mp3start = false;
bool mp3 = false;

Si4735 radio;

void setup() {
  Serial.begin(115200);

  // Write EEPROM params
  /*
    EEPROM.write(1, volume);
    EEPROM.write(2, STATION>>8);
    EEPROM.write(3, STATION);
  */


  // Pin init
  pinMode(RESET, OUTPUT);
  pinMode(VOL_MINUS, INPUT_PULLUP);
  pinMode(VOL_PLUS, INPUT_PULLUP);
  pinMode(FREQ_PLUS, INPUT_PULLUP);
  pinMode(FREQ_MINUS, INPUT_PULLUP);
  pinMode(MP3_START, INPUT_PULLUP);
  pinMode(MP3_STOP, INPUT_PULLUP);

  // Reset radio module
  digitalWrite(RESET, LOW);
  delay(100);
  digitalWrite(RESET, HIGH);


  STATION = (EEPROM.read(2) << 8 | EEPROM.read(3));
  volume = EEPROM.read(1);

  // Radio init
  radio.begin();
  radio.setMode(FM, MODE_FM_OPT_FULL_BAND);
  //Set the FM Frequency
  radio.tuneFrequency(STATION);

  // SetUp MP3 player
  mySerial.begin(9600);
  myDFPlayer.begin(mySerial);
  myDFPlayer.volume(volume);

  Serial.println("Start!");
  delay(200);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  displayInfo();
}

void loop() {

  if (!(digitalRead(VOL_MINUS))) {
    delay(500);
    Serial.println("vol-:");
    volume --;
    if (volume < 0) volume = 0;
    //radio.setVolume(volume);
    myDFPlayer.volume(volume);
    EEPROM.write(1, volume);
    displayInfo();
  }

  if (!(digitalRead(VOL_PLUS))) {
    delay(500);
    Serial.println("vol+:");
    volume ++;
    if (volume > 15) volume = 15;
    //radio.setVolume(volume);
    myDFPlayer.volume(volume);
    EEPROM.write(1, volume);
    displayInfo();
  }

  if (!(digitalRead(FREQ_PLUS))) {
    delay(100);
    Serial.println("freq+:");

    if (STATION < 7500) STATION = STATION + 1;
    else STATION = STATION + 10;

    radio.tuneFrequency(STATION);
    displayInfo();

    EEPROM.write(2, STATION >> 8);
    EEPROM.write(3, STATION);
  }

  if (!(digitalRead(FREQ_MINUS))) {
    delay(100);
    Serial.println("freq-:");

    if (STATION < 7500) STATION = STATION - 1;
    else STATION = STATION - 10;

    radio.tuneFrequency(STATION);
    displayInfo();

    EEPROM.write(2, STATION >> 8);
    EEPROM.write(3, STATION);
  }

  if (!(digitalRead(MP3_START))) {

    mp3start = !mp3start;

    if (mp3start) {
      delay(500);
      mp3 = true;
      radio.mute();
      Serial.println("mp3 start");
      myDFPlayer.play(1);
      displayInfo();
    }
    else {
      delay(500);
      mp3 = false;
      radio.unmute();
      Serial.println("mp3 stop");
      myDFPlayer.stop();
      displayInfo();
    }
  }
}

void displayInfo(void) {
  lcd.clear();
  for (uint8_t i = 0; i < 16; i++) lcdBuff[i] = 0;
  if (!mp3) {
    float station = STATION / 100.0;
    lcd.setCursor(0, 0);
    sprintf(lcdBuff, "Freq:");
    lcd.print(lcdBuff);

    lcd.setCursor(6, 0);
    lcd.print(station);

    lcd.setCursor(13, 0);
    sprintf(lcdBuff, "MHz");
    lcd.print(lcdBuff);

    sprintf(lcdBuff, "Volume: %d", volume);
    lcd.setCursor(0, 1);
    lcd.print(lcdBuff);
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    sprintf(lcdBuff, "Start mp3: 1.mp3");
    lcd.print(lcdBuff);
    lcd.setCursor(0, 1);
    sprintf(lcdBuff, "Volume %d", volume);
    lcd.print(lcdBuff);
  }
}
