
#include "ssd1306.h"
#include <msp430.h>
#include <stdint.h>
#include "font_5x7.h"
#include "i2cMaster.h"

/* ====================================================================
 * Horizontal Centering Number Array
 * ==================================================================== */
const unsigned char HcenterUL[] = {                                           // Horizontally center number with separators on screen
                               0,                                       // 0 digits, not used but included to size array correctly
                               61,                                      // 1 digit
                               58,                                      // 2 digits
                               55,                                      // 3 digits
                               49,                                      // 4 digits and 1 separator
                               46,                                      // 5 digits and 1 separator
                               43,                                      // 6 digits and 1 separator
                               37,                                      // 7 digits and 2 separators
                               34,                                      // 8 digits and 2 separators
                               31,                                      // 9 digits and 2 separators
                               25                                       // 10 digits and 3 separators
};

static i2cInstance i2c_oled = {
    .slaveAddress = 0x3C
};

void ssd1306_init(void) {
    // SSD1306 init sequence
    ssd1306_command(SSD1306_DISPLAYOFF);                                // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);                        // 0xD5
    ssd1306_command(0x80);

    ssd1306_command(SSD1306_SETMULTIPLEX);                              // 0xA8
    ssd1306_command(SSD1306_LCDHEIGHT - 1);

    ssd1306_command(SSD1306_SETDISPLAYOFFSET);                          // 0xD3
    ssd1306_command(0x0);                                               // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);                        // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                                // 0x8D
    ssd1306_command(0x14);                                              // generate high voltage from 3.3v line internally
    ssd1306_command(SSD1306_MEMORYMODE);                                // 0x20
    ssd1306_command(0x02);                                              // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);

    ssd1306_command(SSD1306_SETCOMPINS);                                // 0xDA
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);                               // 0x81
    ssd1306_command(0x7F);

    ssd1306_command(SSD1306_SETPRECHARGE);                              // 0xd9
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);                             // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);                       // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                             // 0xA6

    ssd1306_command(SSD1306_DEACTIVATE_SCROLL);

    ssd1306_command(SSD1306_DISPLAYON);                                 //--turn on oled panel
} // end ssd1306_init

void ssd1306_command(unsigned char command) {
    buffer[0] = 0x80;
    buffer[1] = command;
    i2c_write(i2c_oled, buffer, 2);
    __bis_SR_register(LPM0_bits + GIE);
}

void ssd1306_clearDisplay(void) {

    uint8_t page;
    for (page = 0; page < 4; page++) {
        ssd1306_setPosition(0, page);
        uint8_t i;
        for (i = 16; i > 0; i--) {
            uint8_t x;
            for(x = 8; x > 0; x--) {
                if (x == 1) {
                    buffer[x-1] = 0x40;
                } else {
                    buffer[x-1] = 0x0;
                }
            }

            i2c_write(i2c_oled, buffer, 9);
        }
    }
}

void ssd1306_setPosition(uint8_t column, uint8_t page) {
    if (column > 128) {
        column = 0;                                                     // constrain column to upper limit
    }

    if (page > 8) {
        page = 0;                                                       // constrain page to upper limit
    }

    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(column);                                            // Column start address (0 = reset)
    ssd1306_command(SSD1306_LCDWIDTH-1);                                // Column end address (127 = reset)

    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(page);                                              // Page start address (0 = reset)
    ssd1306_command(7);                                                 // Page end address
} // end ssd1306_setPosition

void ssd1306_printText(uint8_t x, uint8_t y, char *ptString) {
    ssd1306_setPosition(x, y);

    while (*ptString != '\0') {
        buffer[0] = 0x40;

        if ((x + 5) >= 127) {                                           // char will run off screen
            x = 0;                                                      // set column to 0
            y++;                                                        // jump to next page
            ssd1306_setPosition(x, y);                                  // send position change to oled
        }

        uint8_t i;
        for(i = 0; i < 5; i++) {
            buffer[i+1] = font_5x7[*ptString - ' '][i];
        }

        buffer[6] = 0x0;

        i2c_write(i2c_oled, buffer, 7);
        ptString++;
        x+=6;
    }
} // end ssd1306_printText

void ssd1306_printTextBlock(uint8_t x, uint8_t y, char *ptString) {
    char word[12];
    uint8_t i;
    uint8_t endX = x;
    while (*ptString != '\0'){
        i = 0;
        while ((*ptString != ' ') && (*ptString != '\0')) {
            word[i] = *ptString;
            ptString++;
            i++;
            endX += 6;
        }

        word[i++] = '\0';

        if (endX >= 127) {
            x = 0;
            y++;
            ssd1306_printText(x, y, word);
            endX = (i * 6);
            x = endX;
        } else {
            ssd1306_printText(x, y, word);
            endX += 6;
            x = endX;
        }
        ptString++;
    }

}
static char *hexDigits = "0123456789ABCDEF";
char *ssd1306_addHex(uint16_t num, char *outbuf, uint8_t places)
{
    uint8_t i;
    uint8_t index;
    for(i = 0; i < places; i++)
    {
        *outbuf++ = hexDigits[(num & 0xF000) >> 12];
        num <<= 4;
    }
    return outbuf;
}

//                         "0123456789012345678901";
static char accelText[] = {"A: x     y     z     "};
static char tempText[]  = {"T:       "};
static char gyroText[]  = {"G: x     y     z     "};

void printGyroData(uint8_t *data)
{
    ssd1306_addHex((data[0]<<8)+data[1], &accelText[5], 3);
    ssd1306_addHex((data[2]<<8)+data[3], &accelText[11], 3);
    ssd1306_addHex((data[4]<<8)+data[5], &accelText[17], 3);
    ssd1306_printText(0,0, accelText);

    ssd1306_addHex((data[8]<<8)+data[9], &gyroText[5], 3);
    ssd1306_addHex((data[10]<<8)+data[11], &gyroText[11], 3);
    ssd1306_addHex((data[12]<<8)+data[13], &gyroText[17], 3);
    ssd1306_printText(0,1, gyroText);

    ssd1306_addHex((data[6]<<8)+data[7], &tempText[4], 4);
    ssd1306_printText(0,3, tempText);
}
