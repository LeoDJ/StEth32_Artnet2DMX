#include "globals.h"
#include <menu.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <menuIO/encoderIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/keyIn.h>
#include <menuIO/u8g2Out.h>
#include <Wire.h>
#include "eth.h"

using namespace Menu;

#define fontName u8g2_font_6x10_mf 
#define fontX 6
#define fontY 12
#define offsetX 0
#define offsetY 3
#define U8_Width 128
#define U8_Height 64
#define USE_HWI2C

// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);

// define menu colors --------------------------------------------------------
// each color is in the format:
//  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
// this is a monochromatic color table
const colorDef<uint8_t> colors[6] MEMMODE= {
    {{0, 0}, {0, 1, 1}}, // bgColor
    {{1, 1}, {1, 0, 0}}, // fgColor
    {{1, 1}, {1, 0, 0}}, // valColor
    {{1, 1}, {1, 0, 0}}, // unitColor
    {{0, 1}, {0, 0, 1}}, // cursorColor
    {{1, 1}, {1, 0, 0}}, // titleColor
};

#define MAX_DEPTH 2

#ifdef MENU_SERIAL
    serialIn menuSerial(MENU_SERIAL);
#endif

MENU_INPUTS(in, 
    #ifdef MENU_SERIAL
        &menuSerial
    #endif
);

MENU_OUTPUTS(out, MAX_DEPTH, 
    #ifdef MENU_SERIAL
        SERIAL_OUT(MENU_SERIAL),
    #else
        NONE,
    #endif
    U8G2_OUT(u8g2, colors, fontX, fontY, offsetX, offsetY, {0, 0, U8_Width / fontX , U8_Height / fontY})
);

TOGGLE(_config.dhcp, menuIpType, "IP Type: ", doNothing, exitEvent, wrapStyle,
    VALUE("Static", false, doNothing, noEvent),
    VALUE("DHCP", true, doNothing, noEvent)
);

char* const ipDigit PROGMEM="0123456789 ";//allowed digits
char* const ipValidator[] PROGMEM={ipDigit,ipDigit,ipDigit,".",ipDigit,ipDigit,ipDigit,".",ipDigit,ipDigit,ipDigit,".",ipDigit,ipDigit,ipDigit};//validators

    // OP("Op1", doNothing, noEvent),
MENU(mainMenu, "Artnet2DMX", doNothing, noEvent, wrapStyle,
    EDIT("IP", ipStr, ipValidator, doNothing, noEvent, noStyle),
    SUBMENU(menuIpType),
    EXIT("Exit")
);

NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);

uint8_t encoderState = 0;
uint16_t encoderPosition = 0, lastEncoderPosition = 0, lastEncStepPosition = 0;
uint32_t lastButtonPress = 0;

void buttonISR() {
    uint32_t now = millis();
    if(now - lastButtonPress > BTN_DEBOUNCE) {
        lastButtonPress = now;
        if(!digitalRead(PIN_ENCODER_BTN)) {
            nav.doNav(enterCmd);
        }
    }
}

void encoderISR() {
    uint8_t p1val = digitalRead(PIN_ENCODER_A);
    uint8_t p2val = digitalRead(PIN_ENCODER_B);
    uint8_t state = encoderState & 3;
    if (p1val) state |= 4;
    if (p2val) state |= 8;
    encoderState = (state >> 2);
    switch (state) {
        case 1: case 7: case 8: case 14:
            encoderPosition++;
            return;
        case 2: case 4: case 11: case 13:
            encoderPosition--;
            return;
        case 3: case 12:
            encoderPosition += 2;
            return;
        case 6: case 9:
            encoderPosition -= 2;
            return;
    }
}

void doEncoderNavigation() {
    if(encoderPosition != lastEncoderPosition) {
        lastEncoderPosition = encoderPosition;
        int16_t encPos = encoderPosition / 4;
        int8_t diff = encPos - lastEncStepPosition;
        lastEncStepPosition = encPos;
        if (diff > 0) {
            nav.doNav(upCmd);
        }
        else if (diff < 0) {
            nav.doNav(downCmd);
        }
    }
}

void scanI2C() {
      byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.println(address, HEX);

      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.println(address, HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("done");
}

void initDisplay() {
    Wire.begin();
    // scanI2C();
    u8g2.begin();
    u8g2.setI2CAddress(0x78);
    u8g2.setFont(fontName);
    u8g2.drawStr(0, 10, "Hello World");

    pinMode(PB11, OUTPUT);      // Use B11 as 3.3V pin
    digitalWrite(PB11, HIGH);

    pinMode(PIN_ENCODER_A, INPUT_PULLUP);
    pinMode(PIN_ENCODER_B, INPUT_PULLUP);
    pinMode(PIN_ENCODER_BTN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_BTN), buttonISR, CHANGE);

    mainMenu[0].disable(); // IP entry readonly
}

void loopDisplay() {
    nav.doInput();
    doEncoderNavigation();
    if (nav.changed(0)) {
        u8g2.firstPage();
        do {
            nav.doOutput();
        } while (u8g2.nextPage());
    }
}