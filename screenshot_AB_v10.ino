/* Neil Pierson - June 13, 2016
 *  
 *  screenshot_AB_v10.ino (started June 13, 2016)
 *  
 *  function: screenshotUUencodeBMP()
 *  
 *  Capture screen bitmap and output uuencoded 1bit monochrome bitmap file (.bmp) to Serial output.
 *  Useful function for documenting game sketches with screenshots.
 *  
 *  Note: BMP raster is normally upside down, (height-1) first and row 0 last.
 *        Row pixels are bytes left (msbit) to right (lsbit) and padded to 32bit if needed.
 *  
 *  https://en.wikipedia.org/wiki/Uuencoding
 *  https://en.wikipedia.org/wiki/BMP_file_format
 *  
 *  
 *  Code added to Arbuboy HelloWorld example sketch.
 *  
 *  Included my favorite helper functions: buttonCheck(), buttonJustPressed() & buttonHolding()
 *  Helps for debouncing the buttons and knowing when buttons have been held for 5 frames.
 *  Would like these button functions added to the Arbuboy library!
 *  
 */


#include "Arduboy.h"
#include "bitmaps.h"


// make an instance of arduboy used for many functions
Arduboy arduboy;

// helper functions for Arduboy buttons
#define HOLDFRAMES 5
uint8_t buttonState[HOLDFRAMES];
uint8_t buttonIndex = 0;

void buttonCheck() { // call once per frame
  buttonIndex = (buttonIndex+1) % HOLDFRAMES;
  buttonState[buttonIndex] = arduboy.buttonsState(); // circular buffer of recent button states
}

boolean buttonJustPressed(uint8_t buttons) {
  uint8_t prevIndex = (buttonIndex > 0) ? buttonIndex-1 : HOLDFRAMES-1;
  return (!(buttonState[prevIndex] & buttons) && (buttonState[buttonIndex] & buttons));
}

boolean buttonHolding(uint8_t buttons) {
  uint8_t i,hold = 0xFF;
  for (i=0; i<HOLDFRAMES; i++) hold &= buttonState[i];
  return (hold & buttons);
}


int screen_num = 0;
void screenshotUUencodeBMP(int16_t x, int16_t y, uint8_t w, uint8_t h) {
  int16_t rowsz;
  uint8_t *buf;
  uint8_t curr;
  int16_t index;
  char c = 0;

  if (x < 0 || y < 0 || x+w > WIDTH || y+h > HEIGHT) return;

  // header: 14 + 40 + 8 = 62 bytes
  struct headerBMPtype {
    int16_t sig;          // 0x4d42, from: 0x42, 0x4d = "BM"
    int32_t filesize;     // imagesize + sizeof(struct headerBMPtype)
    int32_t reserved;     // 0
    int32_t offset;       // 62 bytes, sizeof(struct headerBMPtype)
    int32_t headerDIBsz;  // 40 bytes
    int32_t width;
    int32_t height;
    int16_t planes;       // 1 plane
    int16_t bits;         // 1 bit per pixel
    int32_t compression;  // 0 = no compression
    int32_t imagesize;    // width * height / 8
    int32_t xppm;         // 3,780 pixels per meter = 96 dots per inch (DPI)
    int32_t yppm;         // 3,780 = 96 dots per inch (DPI)
    int32_t colors;       // 0 default, 2^1 = 2
    int32_t imp_colors;   // 0 default, important colors (all)
    int32_t color0;       // 0x00000000 black -- (msb) bit 31 [Alpha,Red,Green,Blue] bit 0 (lsb)
    int32_t color1;       // 0x00FFFFFF white
  } header;

  rowsz = ((int16_t)w + 31)/32 * 4; // raster rows padded to 32bit alignment
  header.width       = (int32_t) w;
  header.height      = (int32_t) h;
  header.imagesize   = rowsz * header.height;

  header.sig         = 0x4d42;
  header.filesize    = header.imagesize + sizeof(struct headerBMPtype);
  header.reserved    = 0;
  header.offset      = sizeof(struct headerBMPtype);
  header.headerDIBsz = 40;
  header.planes      = 1;
  header.bits        = 1;
  header.compression = 0;
  header.xppm        = 3780;
  header.yppm        = 3780;
  header.colors      = 0;
  header.imp_colors  = 0;
  header.color0      = 0x00000000;
  header.color1      = 0x00FFFFFF;

  buf = (uint8_t *) &header;

  Serial.print(F("begin 0644 screen"));
  Serial.print(screen_num++);
  Serial.print(F(".bmp")); 

  index = 0;  // byte index in BMP file format
  
  do {
    if (index % 45 == 0) { // new line
      Serial.println();
      
      // <length character><formatted characters><newline>
      // ASCII character determined by adding 32 to the actual byte count
      // 'M' (ASCII 77) = length 45 (maximum bytes encoded per line)

      if (header.filesize - index >= 45) Serial.print('M');
      else { 
        char b = ' ' + (char)(header.filesize - index);
        Serial.print((b == ' ') ? '`' : b);
      }
    }

    // current byte
    if (index < sizeof(struct headerBMPtype)) curr = buf[index]; // header
    else {                                                       // raster
      int row = (index - sizeof(struct headerBMPtype)) / rowsz;
      int col = (index - sizeof(struct headerBMPtype)) % rowsz;

      if (col*8 < w) {
        curr = 0;
        for (byte i=0; i<8; i++) {
          if ((col*8 + i) < w) {
            // assemble raster bits left (msb) to right (lsb)
            curr |= arduboy.getPixel(x+(uint8_t)(col*8 + i),(uint8_t)(y + (int)(h-1) - row));
          }
          if (i < 7) curr <<= 1;
        }

      } else curr = 0; // pad byte

    }
    
    switch (index % 3) {
      case 0:
        c = ' ' +  (curr >> 2);         // 6 bits
        Serial.print((c == ' ') ? '`' : c);
        c = ' ' + ((curr & 0x03) << 4); // 2 bits for next character
        break;
      case 1:
        c += (curr & 0xF0) >> 4;        //+4 bits
        Serial.print((c == ' ') ? '`' : c);
        c = ' ' + ((curr & 0x0F) << 2); // 4 bits for the next character
        break;
      case 2:
        c += (curr & 0xC0) >> 6;        //+2bits
        Serial.print((c == ' ') ? '`' : c);
        c = ' ' +  (curr & 0x3F);       // 6 bits, finish 3 byte group
        Serial.print((c == ' ') ? '`' : c);
    }

    index++;
  } while (index < header.filesize);

  // last partial encode
  if (index % 3 != 0) Serial.print((c == ' ') ? '`' : c); // flush remaining bits
  if (index % 3 == 2) Serial.print('`');                  // extra pad to complete 4 character encode group
  
  
  // last two lines
  Serial.println(F("\n`\nend")); // grave accent "`" (ASCII code 96) signifying zero bytes <newline> "end" <newline>
  
} // screenshotUUencodeBMP()



// This function runs once in your game.
// use it for anything that needs to be set only once in your game.
void setup() {
  byte i;
  
  // initiate arduboy instance
  arduboy.begin();

  // here we set the framerate to 15, we do not need to run at
  // default 60 and it saves us battery life
  arduboy.setFrameRate(15);


  // initialize button tracking variables for buttonCheck(), buttonJustPressed() & buttonHolding()
  for (i=0; i<HOLDFRAMES; i++) buttonState[i] = 0; 
  buttonIndex = 0;
    
  Serial.begin(115200);
  Serial.print(F("\n== screenshot_AB_v10.ino [by Neil Pierson, 13 Jun 2016]\n"));

}


// our main game loop, this runs once every cycle/frame.
// this is where our game logic goes.
void loop() {
  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return;


  buttonCheck(); // call once per frame

  // opportunity to do a screenshot if UP & DOWN buttons are just pressed (before clearing the screen)
  if (buttonJustPressed(UP_BUTTON) && buttonJustPressed(DOWN_BUTTON)) screenshotUUencodeBMP(0,0,128,64);


  // first we clear our screen to black
  arduboy.clear();

  // we set our cursor 5 pixels to the right and 10 down from the top
  // (positions start at 0, 0) 
  arduboy.setCursor(4, 9);

  // then we print to screen what is in the Quotation marks ""
  //arduboy.print(F("Hello, world!"));

  arduboy.drawBitmap(19,18,sudokuLogo,LOGO_W,LOGO_H,WHITE);


  // then we finaly we tell the arduboy to display what we just wrote to the display
  arduboy.display();
}
