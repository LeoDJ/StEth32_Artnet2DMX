#include "globals.h"
#include <menu.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <menuIO/encoderIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/keyIn.h>
#include <menuIO/u8g2Out.h>
#include <Wire.h>

using namespace Menu;

#define fontName u8g2_font_7x13_mf
#define fontX 7
#define fontY 16
#define offsetX 0
#define offsetY 3
#define U8_Width 128
#define U8_Height 64
#define USE_HWI2C
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);

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

encoderIn<PIN_ENCODER_A, PIN_ENCODER_B> encoder;
encoderInStream<PIN_ENCODER_A, PIN_ENCODER_B> encStream(encoder, 4); // sensitivity 4

serialIn menuSerial(MENU_SERIAL);

//a keyboard with only one key as the encoder button
keyMap encBtn_map[] = {{-PIN_ENCODER_BTN, options->getCmdChar(enterCmd)}}; // negative pin numer to activate internal pull-up
keyIn<1> encButton(encBtn_map);

MENU_INPUTS(in, &encStream, &encButton, &menuSerial);

MENU_OUTPUTS(out, MAX_DEPTH, 
    SERIAL_OUT(MENU_SERIAL),
    U8G2_OUT(u8g2, colors, fontX, fontY, offsetX, offsetY, {0, 0, U8_Width / fontX , U8_Height / fontY})
);

TOGGLE(_config.dhcp, menuIpType, "IP Type: ", doNothing, exitEvent, wrapStyle,
    VALUE("Static", false, doNothing, noEvent),
    VALUE("DHCP", true, doNothing, noEvent)
);

MENU(mainMenu, "Artnet2DMX", doNothing, noEvent, wrapStyle,
    OP("Op1", doNothing, noEvent),
    SUBMENU(menuIpType),
    EXIT("Exit")
);

NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);

void initDisplay() {
    Wire.begin();
    u8g2.begin();
    u8g2.setFont(fontName);
}

void loopDisplay() {
    nav.doInput();
    if (nav.changed(0)) {
        u8g2.firstPage();
        do {
            nav.doOutput();
        } while (u8g2.nextPage());
    }
}