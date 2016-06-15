# screenshot_AB_v10
Arduboy utility to take a screenshot -- screenshotUUencodeBMP()
  
screenshot_AB_v10.ino (started June 13, 2016)

function: screenshotUUencodeBMP()

Capture screen bitmap and output uuencoded 1bit monochrome bitmap file (.bmp) to Serial output.
Useful function for documenting game sketches with screenshots.

Copy & paste the serial output "begin" thru "end" lines into a file (example data.txt)

$ uudecode data.txt

$ ls -l screen0.bmp

-rw-r--r--  1 neilp  503  1086 Jun 15 00:10 screen0.bmp

