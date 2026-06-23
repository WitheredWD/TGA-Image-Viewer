# TGA-Image-Viewer
This is a a small, not too complex tga image viewer.

## What it contains:
It contains the logic for loading .tga (Truevision TGA) Images and displaying them using SDL2. 

## How to use it:
To use it you have to run it from you terminal. It needs at least 1 extra argument, the first one extra argument is the path of the file, the next argument shows wether to print out some info about the image. To display it you have to type "display-information".



It's a small side project of mine, it obviously doesn't have too much work put into it but it may still be cool and be useful if someone wants to know/see how to read a .tga file in C/C++. Beause I didn't pour much work into it it doesn't contain logic for handling color maps or compressed data. I didn't test it with 32-Bit images so it may only work with 24-bit images (no Alpha Channel), but as said previously I haven't tested it. In its current form it only reads pixels from left-to-right and top-to-down which isn't always how you want to read .tga images. Maybe I will change it later.
