#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <bignum256.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// Touchscreen
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

// Calibrate values
#define TS_MINX 270
#define TS_MINY 200
#define TS_MAXX 880
#define TS_MAXY 940
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// TFT
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4  // Optional

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

String N1, N2, ShowSC, opt;
bool updata=false;
float answers=-1;

String Key[4][4] = {
  { "7", "8", "9", "" },
  { "4", "5", "6", "" },
  { "1", "2", "3", "" },
  { "C", "0", "Ok", "" }
};

int state = 0;

void printKeypad(){
  tft.fillScreen(BLACK);
  tft.fillRect(0, 80, 240, 240, WHITE);
  tft.drawFastHLine(0, 80, 240, BLACK);
  tft.drawFastHLine(0, 140, 240, BLACK);
  tft.drawFastHLine(0, 200, 240, BLACK);
  tft.drawFastHLine(0, 260, 240, BLACK);
  tft.drawFastHLine(0, 320-1, 240, BLACK);
  tft.drawFastVLine(0, 80, 240, BLACK);
  tft.drawFastVLine(60, 80, 240, BLACK);
  tft.drawFastVLine(120, 80, 240, BLACK);
  tft.drawFastVLine(180, 80, 240, BLACK);
  tft.drawFastVLine(240-1, 80, 240, BLACK);
  for (int y=0;y<4;y++) {
    for (int x=0;x<4;x++) {
      tft.setCursor(22 + (60*x), 100 + (60*y));
      tft.setTextSize(3);
      tft.setTextColor(BLACK);
      tft.println(Key[y][x]);
    }
  } // end print keypad
}


void printChoiche(){
  tft.fillScreen(BLACK);
  tft.fillRect(0, 80, 240, 240, WHITE);
  tft.drawFastHLine(0, 80, 240, BLACK);
  tft.drawFastHLine(0, 140, 240, BLACK);
  tft.drawFastHLine(0, 200, 240, BLACK);
  tft.drawFastHLine(0, 260, 240, BLACK);
  tft.drawFastHLine(0, 320-1, 240, BLACK);
  tft.drawFastVLine(0, 80, 240, BLACK);
  tft.drawFastVLine(60, 80, 240, BLACK);
  tft.drawFastVLine(120, 80, 240, BLACK);
  tft.drawFastVLine(180, 80, 240, BLACK);
  tft.drawFastVLine(240-1, 80, 240, BLACK);
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000
TSPoint waitTouch() {
  TSPoint p;
  do {
    p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    Serial.print(F("X ")); Serial.print(p.x);
    Serial.print(F(" Y ")); Serial.println(p.y);
  } while((p.z < MINPRESSURE )|| (p.z > MAXPRESSURE));
  int tmp = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.x = map(p.y, TS_MINY, TS_MAXY, 0, 320);
  p.y = 240 - tmp;
  return p;
}

void getkey(){
 TSPoint p = waitTouch();
  updata = false;
  for (int i1=0;i1<4;i1++) {
    for (int i2=0;i2<4;i2++) {
      if ((p.x>=240-((i1+1)*60)+1&&p.x<=240-(i1*60)-1)&&(p.y>=(i2*60)+1&&p.y<=((i2+1)*60)-1)) {
        if ((i1<=2&&i2<=2)||(i1==3&&i2==1)) {
          if (opt==0) {
            if (answers!=-1) answers = -1;
            N1 = N1 + Key[i1][i2];
            ShowSC = N1;
          } else {
            N2 = N2 + Key[i1][i2];
            ShowSC = opt + N2;
          }
        } else {
          if (Key[i1][i2]=="C") {
            N1 = N2 = "";
            opt = "";
            answers = 0;
            ShowSC = N1;
            state = 3;
          } else if (i2==3) {
            if (N1=="") N1 = String(answers);
            opt = Key[i1][i2];
            ShowSC = Key[i1][i2];
          } else if (Key[i1][i2]=="=") {
            if (opt=="+") answers = N1.toInt() + N2.toInt();
            else if (opt=="-") answers = N1.toInt() - N2.toInt();
            else if (opt=="*") answers = N1.toInt() * N2.toInt();
            else if (opt=="/") answers = N1.toInt() / N2.toInt();
            N1 = N2 = opt = "";
            ShowSC = answers;
          }
        }
        updata = true;
      }
    }
  }

  if (updata) {
    tft.fillRect(0, 0, 240, 80, BLACK);
    tft.setCursor(10, 10);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.println(ShowSC);
  }
  delay(300);
}

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Paint!"));
  tft.reset();
  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    return;
  }

  tft.begin(identifier);
  tft.setRotation(0);

}

int cont = 0;

void loop(){
  // perform math operations on integers larget than  32 bytes

  cont = cont + 1;

  Serial.print("Loop number: ");
  Serial.println(cont);

  // int32_t n =  0x01FF;
  // Serial.print("display n: ");
  // Serial.println(n);

  struct {
    int overflow = 0; // 1 if overflow, 0 if not overflow
    uint32_t d[8] = {0x10,0x1,0xF,0xA,0xB,0x0,0x0,0x0};
  } privkey;

  Serial.print("display privkey: ");
  for(int i=0; i<8; i++){
    Serial.print(privkey.d[i], HEX);
  }
  Serial.println();

  delay(1000);
  
  // skip inserting the number in the serial
  /* int n_side = 0;
  String readString;
  Serial.println("How many sides (2-256)?");
  while(n_side == 0){
    delay(1000);
    while (Serial.available()) {
      char c = Serial.read();  //gets one byte from serial buffer
      readString += c; //makes the String readString
      delay(2);  //slow looping to allow buffer to fill with next character
    }
    if (readString.length() >0) {
      Serial.println(readString);  //so you can see the captured String
      n_side = readString.toInt();  //convert readString into a number
      readString="";
    }
    Serial.println("...");
    Serial.println(n_side);
  }
  // number of rolls with selected dice
  int n_rolls = ceil(256*log(2)/log(n_side));
  Serial.println("Number of rolls:");
  Serial.println(n_rolls);

  uint8_t rolls[n_rolls];

  struct {
    uint32_t overflow;
    uint32_t d[8] = {0xFFFFFFFFF,0xFFFFFFFFF,0xFFFFFFFFF,0xFFFFFFFFF,0xFFFFFFFFF,0xFFFFFFFFF,0xFFFFFFFFF,0x0};
    uint8_t flags;
  } privkey;

  uint8_t r[32];
  //bigSetZero(r);

  uint8_t a[32]  = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xFF};

  //bigAdd(r, a, (uint8_t *)privkey.d);
  Serial.println("display r:");
  for(int i=0; i<32; i++){
    Serial.print(r[i], HEX);
  }
  Serial.println();

  Serial.println("display privkey.d:");
  for(int i=0; i<8; i++){
    Serial.print(privkey.d[i], HEX);
  }

  Serial.println("Start rolling:");
  int roll=0;
  for(int i=0; i<n_rolls; i++){
    do{
      roll=0;
      while (Serial.available()) {
        char c = Serial.read();  //gets one byte from serial buffer
        readString += c; //makes the String readString
        delay(2);  //slow looping to allow buffer to fill with next character
      }
      if (readString.length() > 0) {
        Serial.println(readString);  //so you can see the captured String
        roll = readString.toInt();  //convert readString into a number
        readString="";
      }
    } while(roll < 1 || roll > n_side);
    Serial.print(i);
    Serial.print("-th rolls +++ result:");
    Serial.println(roll);
    rolls[i] = roll - 1;
  }

  Serial.println("Results series:");
  for(int i=0; i<n_rolls; i++){
    Serial.println(rolls[i], HEX);
  }*/

  /*
  switch(state){
    case 0:
      tft.fillScreen(BLUE);
      tft.setCursor(10, 10);
      tft.setTextSize(3);
      tft.setTextColor(WHITE);
      tft.println("Eternity");
      tft.setCursor(10, 60);
      tft.println("Wallet");
      tft.setTextSize(1);
      tft.setCursor(10, 120);
      tft.println("Another useless gadget");
      delay(5000);
      state = 1;
      break;
    case 1:
      printChoiche();
      delay(5000);
      state = 2;
      break;
    case 2:
      printKeypad();
      state = 3;
      break;
    case 3:
      getkey();
      //state = 3;
      break;
    case 4:
      tft.fillScreen(RED);
      tft.setCursor(10, 10);
      tft.setTextSize(3);
      tft.setTextColor(WHITE);
      tft.println("Hello");
      delay(5000);
      state = 5;
      break;
    case 5:
      tft.fillScreen(RED);
      tft.setCursor(10, 10);
      tft.setTextSize(3);
      tft.setTextColor(WHITE);
      tft.println("bello");
      delay(5000);
      state = 6;
      break;
    case 6:
      tft.fillScreen(MAGENTA);
      tft.setCursor(10, 10);
      tft.setTextSize(3);
      tft.setTextColor(WHITE);
      tft.println("bello");
      delay(5000);
      state = 7;
      break;
    case 7:
      tft.fillScreen(CYAN);
      tft.setCursor(10, 10);
      tft.setTextSize(3);
      tft.setTextColor(WHITE);
      tft.println("bello");
      delay(5000);
      state = 8;
      break;
    default:
      state = 0;
      break;
  }
*/
}
