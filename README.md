A small program for combining a set of cutout images into a single image.
Uses SFML for image saving and loading, but any other library capable of turning an image into an array of color bytes and saving it back should be able to slotted in without much issue.

This program makes the assumptions:
	-All images are the same size (It will still work with different sizes, but the results may not be favorable)
	-Input images are ordered in the way they should be put together (Standard alphabetical sort, 0-9 then A-Z) 
	-There is a folder called "Output" to write the images to.

Programmed in C++ 14 (With macro definition to allow for C++ 17 compilation, if one so desires.)

Since this is largely intended for pre-processing images, it's a standalone program as opposed to a library.
Can be run directly or from a script/command line.

Check releases for an example use of this, including a batch script, sample input (rendered by yours truly), and a small playback program (coming soon)!

I personally found use for this program as a way to encode short movement clips for a personal project without needing to bring in a video-handling library or worry about compression artifacts.

The resulting images themselves are quite colorful, and can also be used without the playback shader, if desired.