//Converts a set of images into cutouts based off of their alpha channel, and packs them all into a set of RGBA32 images.
//Created with the SFML library, compiled with C++ 14.
//This can be compiled in C++ 17 and forward by changing this to a 1:
#define USING_CPP_17 0

//Theoretically, you could use any image library for this, since it's mostly direct pixel reading/writing.
//I'm mainly using SFML for this because I'm familiar with it.
#include <SFML/Graphics/Image.hpp>

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>

#if USING_CPP_17 == 1
#include <filesystem>
namespace fs = std::filesystem;
#else
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 1
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

//This could pretty easily be parallelized if necessary: Each iteration of the loops accesses a separate pixel, so there's little risk of race conditions.
//(Care would need to be taken to ensure that the underlying code doesn't access neighboring pixels, if you convert the sf::Image into a large array.)
void applyLayerToImage(sf::Image& dest, const sf::Image& src, const int offset, const int bitsPerLayer) {
	for (size_t x = 0; x < dest.getSize().x; x++) {
		for (size_t y = 0; y < dest.getSize().y; y++) {
			//Get the relevant pixels from the source and destination
			uint32_t destBuffer = dest.getPixel(x, y).toInteger();
			sf::Color srcBuffer = src.getPixel(x, y);

			//Formula for getting the maximum value a layer can have for a given bits per layer is (2^BPL - 1).
			//I opt to use a bitwise shift instead of the powf() function, but the result is the same.
			//This value also doubles as a bitmask, which is used in the provided shader!
			uint8_t alphaBitMax = (1u << bitsPerLayer) - 1;
			//Normalize the current layer's alpha to the 0-1 range, then multiply by the maximum
			uint32_t alphaValue = (uint32_t)roundf(((float)srcBuffer.a / 255.f) * (float)alphaBitMax);
			//Bitwise or the resulting value into the destination pixel, and store it back
			destBuffer |= alphaValue << offset;
			dest.setPixel(x, y, sf::Color(destBuffer));
		}
	}
}

int main(int argc, char* argv[]) {

	std::string currentDirectory = fs::current_path().generic_string();
	std::string inputPath = "";
	int bitsPerLayer = 1;
	if (argc <= 1) {
		//run with no arguments, use console input to get them
		std::string consoleInput = "";
		std::cout << "Please type the name of the folder with the images to pack. (All images in the folder will be used.)" << std::endl;
		std::getline(std::cin, consoleInput, '\n');
		inputPath = consoleInput;

		std::cout << "Please type how many bits should be assigned to each layer.  \n(Range 1-32, recommended to use a power of 2.)" << std::endl;
		std::getline(std::cin, consoleInput, '\n');
		try {
			bitsPerLayer = std::stoi(consoleInput);
		}
		catch (std::exception&) {
			std::cout << "Error: Cannot convert that value to a number!" << std::endl;
			std::cin.get();
			return -1;
		}
		//clamp bitsPerLayer to the 1-32 range
		bitsPerLayer = std::max(1, std::min(bitsPerLayer, 32));
	}
	else if (argc == 3) {
		//run with arguments
		inputPath = argv[1];
		try {
			bitsPerLayer = std::stoi(argv[2]);
		}
		catch (std::exception&) {
			std::cout << "Error: Cannot convert bits per layer value (" << argv[2] << ") to a number!" << std::endl;
			std::cin.get();
			return -1;
		}
		//clamp bitsPerLayer to the 1-32 range
		bitsPerLayer = std::max(1, std::min(bitsPerLayer, 32));
	}
	else {
		std::cout << "For running this program from command line/script, you must pass 2 parameters: " << std::endl;
		std::cout << "1) The relative path to the folder from the present working directory" << std::endl;
		std::cout << "2) The amount of bits sould be assigned to each layer (Range 1-32, recommended to use a power of 2.)" << std::endl;
		return -1;
	}


	std::string imageDirectoryPath = currentDirectory + '/' + inputPath;

	//This is just a list of image formats that SFML's image loader supports. You're welcome to add or remove any you need.
	std::unordered_set<std::string> supportedFiletypes;
	supportedFiletypes.insert(".jpg");
	supportedFiletypes.insert(".png");
	supportedFiletypes.insert(".bmp");
	supportedFiletypes.insert(".tga");
	supportedFiletypes.insert(".gif");
	supportedFiletypes.insert(".psd");
	supportedFiletypes.insert(".hdr");
	supportedFiletypes.insert(".pic");

	//Display a warning message if the user uses .jpg files, since the program relies on the alpha channel.
	bool jpgWarning = false;

	std::vector<std::string> imageFilenames;
	try {
		for (const auto& entry : fs::directory_iterator(imageDirectoryPath)) {
			std::string filepath = entry.path().generic_string();
			std::string extension = filepath.substr(filepath.find_last_of('.'));
			if (supportedFiletypes.find(extension) != supportedFiletypes.end()) {
				imageFilenames.emplace_back(filepath);
			}
			if (extension == ".jpg") {
				jpgWarning = true;
			}
		}
	}
	catch (const std::exception&) {
		std::cout << "Could not open the directory " << imageDirectoryPath << std::endl;
		std::cin.get();
		return -1;
	}

	if (jpgWarning) {
		std::cout << "Warning! .jpg files will likely work poorly with this program, since it relies on the alpha channel (or images with transparency).\n" <<
			"It's recommended to use a different format like .png for your input images." << std::endl;
	}

	if (imageFilenames.size() == 0) {
		std::cout << "Could not find any supported image files in the given directory!" << std::endl;
		std::cin.get();
		return -1;
	}

	//Now make sure the filenames are sorted properly (since directory_iterator does not guarantee this)
	//Using the built in sort for simplicity
	std::stable_sort(imageFilenames.begin(), imageFilenames.end());

	sf::Image bufferLoadImage;
	sf::Image bufferStoreImage;

	//Now, get the output image size from the first image (all images are assumed to have the same size)
	bufferLoadImage.loadFromFile(imageFilenames[0]);
	int imageSizeX = bufferLoadImage.getSize().x;
	int imageSizeY = bufferLoadImage.getSize().y;


	const std::string outputPath = "Output/";
	int outputImageCount = (int)ceilf(imageFilenames.size() / (32.f / bitsPerLayer));
	int layersPerImage = 32 / bitsPerLayer;
	size_t currentLoadedImage = 0;
	for (int x = 0; x < outputImageCount; x++) {
		bufferStoreImage.create(imageSizeX, imageSizeY, sf::Color(0, 0, 0, 0));
		for (int y = 0; y < layersPerImage; y++) {
			if (currentLoadedImage < imageFilenames.size()) {
				bufferLoadImage.loadFromFile(imageFilenames[currentLoadedImage]);
				applyLayerToImage(bufferStoreImage, bufferLoadImage, y * bitsPerLayer, bitsPerLayer);
				currentLoadedImage++;
			}
			else {
				break;
			}
		}
		bufferStoreImage.saveToFile(outputPath + "output_" + std::to_string(x) + ".png");
	}


	return 0;
}

