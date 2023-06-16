//OpenGL fragment shader that allows for playback of the generated images

//130 is the minimum version, as it allows for the bitwise operations used in decoding the images.
//Since it's quite an old version (Almost 15 years old, at the time of writing), almost all hardware should be able to support this.
#version 130

//The image currently being used
uniform sampler2D texture;
//The frame of the image to extract, 0-indexed.
uniform int frameMask;
//The bits per layer. Should match the value at which the used image was encoded with.
uniform int bitsPerLayer;


void main() {	
	//Read texture color
	vec4 texColor = texture2D(texture, gl_TexCoord[0].xy);

	//Convert the color format from 4 floats to a 32-bit unsigned integer
	uint intColor = (uint(round(texColor.r * 255.f)) << 24u) | (uint(round(texColor.g * 255.f)) << 16u) | (uint(round(texColor.b * 255.f)) << 8u) | (uint(round(texColor.a * 255.f)));
	
	//Calculate the bitmask using the same formula in applyLayerToImage. As stated there, this works as both a bitmask and a maximum alpha value to use for normalization!
	uint bitmask = (1u << (bitsPerLayer - 1)) - 1;

	//Shift the color according to the current frame, and mask out the unused bits
	intColor = (intColor >> uint(frameMask * bitsPerLayer)) & bitmask;

	//Normalize the calculated alpha value back to the 0-1 range.
	texColor.a = float(intColor) / float(bitMask); 

	//Color of the cutout display here. 
	//There's a lot of ways to go about setting the color of this (hardcoded values, color uniform, vertex colors), so I'm leaving it open here.
	texColor.r = 0;
	texColor.g = 0;
	texColor.b = 0;

    gl_FragColor = texColor;
}