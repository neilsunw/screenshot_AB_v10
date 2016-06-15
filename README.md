# screenshot_AB_v10
Arduboy utility to take a screenshot -- screenshotUUencodeBMP()
  
screenshot_AB_v10.ino (started June 13, 2016)

function: screenshotUUencodeBMP()

Capture screen bitmap and output uuencoded 1bit monochrome bitmap file (.bmp) to Serial output.
Useful function for documenting game sketches with screenshots.

Note: BMP raster is normally upside down, (height-1) first and row 0 last.
      Row pixels are bytes left (msbit) to right (lsbit) and padded to 32bit if needed.

https://en.wikipedia.org/wiki/Uuencoding
https://en.wikipedia.org/wiki/BMP_file_format


Code added to Arbuboy HelloWorld example sketch.

Included my favorite helper functions: buttonCheck(), buttonJustPressed() & buttonHolding()
Helps for debouncing the buttons and knowing when buttons have been held for 5 frames.
Would like these button functions added to the Arbuboy library!
