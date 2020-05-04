# sports-summary

A program to display images and summaries of the different baseball games played on a specific day. Written in C++(17) and GLSL.
Targets OpenGL 3.3 core profile.

The code is pretty much cross platform (the code to get the current date uses localtime_s instead of localtime, but I think that
is the only Windows specific thing) but currently only builds on Windows using Visual Studio 2019.
This is because the repo includes all the needed binary dependencies already compiled. It builds for 32-bit or 64-bit.

## Building
To build the code, clone the repo and then open the solution in Visual Studio 2019. Then choose Build Solution from the build menu.

## Running
If you build the release configuration, don't try to debug it. In release mode it uses fullscreen, so it can cause problems with
debugging. Debugging works fine in debug mode. Use "Start Debugging" or "Start Without Debugging" as appropriate from withing
Visual Studio.
You can also run by finding the appropriate folder after building. The folder is called Debug or Release, and for 32-bit is in the
solution directory. For 64-bit it is under x64 in the solution directory. Inside the folder, double click "sports-summary.exe".
All the necessary libraries, images, and shader files are automatically copied next to the executable so this should just work.

## Usage
The left and right arrows are used to change with game is selected, ESC is used to quit. *WARNING:* don't use for any day with games
you haven't seen if you don't want the final score spoiled for you!

## Overview
In rough order, the code works like this:
1. Set up GLFW and OpenGL.
2. Show a default window - input doesn't work yet, it is basically a splash screen.
3. Download the main JSON file for the date and parse it.
4. Start asynchronos downloads for the images to be displayed (the urls are found in the JSON).
5. Start the main loop:
6. Check for any finished downloads and add them to the main texture.
7. Check for any inputs
8. Calculate all the geometry and draw it.
9. Go to 6.

### From another perspective, the code looks like this: ###
There are two textures, the first has all the text glypsh, the second has the background image, textures for the frame around the
selected item/game, default textures for images still loading or error (not used yet), and all the textures for the games.
The textures are loaded at startup, but the second texture gets updated in memory as images become available. There are texture managers
to provide the texture coordinates for each image.
The Display::draw function figures out what to draw based on the selected item/game, it holds references to the texture managers and
maintains vectors of points for the item quads and text quads. Finally it calls OpenGlDrawer::draw to do the actual drawing.
The OpenGl drawer sets up the VAO, VBO, EBO, and texture buffers in the constructor. In the draw function it buffers up the latest
geometry, and checks the revision in the texture manager to see if it needs to be buffered as well. There is one set of VBO/EBO/Texture
for the non-text quads and one for the text quads. This is because the non-text quads don't have transparency, but the text ones do. It
would be easy enough to use one texture for everything, but that would add 4MB of texture data for the extra unused transparency
channel, testing would be needed to determine if the extra memory is worth having a single draw call per frame.

# Note
Currently there are no images or details available for the current date, all games are postponed. 
This code does automatically determin the current date for the JSON
URL, but then overwrites it with a hardcoded date. Not only are there no images available, the JSON seems to be slightly different,
giving a string value where a number was expected which causes an error in parsing the JSON. THis could be fixed, but wasn't because:
1. Without a schema, it could break again tomorrow
2. There still wouldn't be any images or text to display
3. There hasn't been time yet.
