To create a 4-bit image for Nokia displays on LM4F120 or TM4C123G
Jonathan Valvano, 6/26/2015
See BmpConvert.cpp for how it works
1) Create a bmp file
   width should be an even number of pixels, multiple of 8 works better
   save the image as a 4-bit bmp file
   store in same directory as BmpConvert.exe
2) Execute BmpConvert.exe
   Type the image name 
   E.g., if the file is horse.bmp, then type horse
3) Open the corresponding txt file, select all, copy
4) Open uVision or CCS compiler
   paste new image as a data constant
5) Draw the image by calling Nokia5110_PrintBMP


