#include <iostream>
#include <SDL2/SDL.h>
#include <string.h>
#include <fstream>

#define BYTE 8

using namespace std;

bool displayInformation = false;

string filePath;

// Var for Header, field 1
unsigned int imageIDLength;
// Var for Header, field 2
unsigned int colorMapType;
// Var for Header, field 3
unsigned int compressionType;
// Var for Header, field 4
unsigned int firstColorMapEntryIndex;
unsigned int colorMapLength;
unsigned int colorMapEntrySize;
bool hasColorMap;
// Var for Header, field 5
unsigned int x_origin;
unsigned int y_origin;
unsigned int img_width;
unsigned int img_height;
unsigned int pixel_depth;
Uint8 img_descriptor;
unsigned int alpha_channel_depth;
unsigned int pixel_order[] = {0 /*[0]=vertical ordering*/, 0/*[1]=horizontal ordering*/};
unsigned int data_storage_interleaving_mode;
int pixelByteSize = 4; // 2: 16-bit color, 3: 24-bit color, 4: 32-bit color
int totalPixelAmount;


typedef struct {
	int x = -1;
	int y = -1;
	Uint32 color;
} Pixel;

Pixel* allPixels;

/*void drawPixel(Pixel p) {

}*/

void readBody(int byteNum, const char* fileContent) {
	// skip the ID
	byteNum += imageIDLength;
	if (hasColorMap) {
		// handle color map reading, maybe for later
		byteNum += colorMapLength;
		cout << "\n\nThere may be issues with rendering the image; this image uses a color map. Handling color maps has not yet been implemented.\n\n";	
	}
	
	totalPixelAmount = img_width * img_height;
	allPixels = new Pixel[img_width * img_height];


	int current_pixel = 0;
	for (unsigned int y = 0; y < img_height; y++) {
		unsigned int x;
		for (x = 0; x < img_width; x++) {
			Pixel p;
			p.x = x;
			p.y = y;
			if (pixel_depth == 32) {
				p.color = (Uint32)fileContent[byteNum] + ((Uint32)fileContent[byteNum+1] << BYTE) + ((Uint32)fileContent[byteNum+2] << BYTE*2) + (/*(Uint32)fileContent[byteNum+3]*/0xFF << BYTE*3);
				byteNum += 4;			
			} else if (pixel_depth == 24) {
				p.color = (Uint32)fileContent[byteNum] + ((Uint32)fileContent[byteNum+1] << BYTE) + ((Uint32)fileContent[byteNum+2] << BYTE*2);
				byteNum += 3;
			}

			allPixels[current_pixel] = p;
			//cout << "    p" << current_pixel << ": x=" << p.x << ", y=" << p.y << ", color:" << p.color << "\n";
			current_pixel++;
		}
		x = 0;
	}
}

void readHeader(const char* fileContent) {
	colorMapType = -1;
	hasColorMap = false;
	
	imageIDLength = fileContent[0];
	
	colorMapType = fileContent[1];
	
	compressionType = fileContent[2];
	
	firstColorMapEntryIndex = fileContent[3] + (fileContent[4] << BYTE); // Combine the values together

	colorMapLength = fileContent[6] + (fileContent[5] << BYTE);
	
	colorMapEntrySize = fileContent[7];
	
	// Probably shouldn't matter whether they are || or && operators
	if (firstColorMapEntryIndex != 0 && colorMapLength != 0 && colorMapEntrySize != 0) {
		hasColorMap = true;
	}

	x_origin = (Uint8)fileContent[8] + ((Uint8)fileContent[9] << BYTE);
	y_origin = (Uint8)fileContent[10] + ((Uint8)fileContent[11] << BYTE);
	img_width = (Uint8)fileContent[12] + ((Uint8)fileContent[13] << BYTE);
	img_height = (Uint8)fileContent[14] + ((Uint8)fileContent[15] << BYTE);
	pixel_depth = (Uint8)fileContent[16];
	img_descriptor = (Uint8)fileContent[17];
	// Get the bits from the img descriptor
	// Might have to be tested further
	int *img_desc_bits = (int*)malloc(sizeof(int) * BYTE);
	for (int i = 0; i < 8; i++) {
		int mask = 1 << i;
		int masked_img_desc = img_descriptor & mask;
		img_desc_bits[i] = masked_img_desc >> i;
	}
	// Use the bits from the img descriptor
	alpha_channel_depth = img_desc_bits[0]+img_desc_bits[1]+img_desc_bits[2];
	
	if (img_desc_bits[3]) pixel_order[0] = 1;
	if (img_desc_bits[4]) pixel_order[1] = 1;


	int byteNum = 18; // (byteNum = current to check byte) last opened byte was 17, so it should be 18
	
	if (displayInformation) {
		cout << "Image ID Length:" << imageIDLength << "\n";
		cout << "Color Map Type: " << colorMapType << "\n";
		cout << "Compression type: " << compressionType << "\n";
		cout << "First Color Map Entry Index: " << firstColorMapEntryIndex << "\n";
		cout << "Color Map Length: " << colorMapLength << "\n";
		cout << "Has Color Map: " << hasColorMap << "\n";
		cout << "Color Map Entry Size: " << colorMapEntrySize << "\n";
		cout << "Dimensions: x=" << x_origin << ", y=" << y_origin << "\n  width=" << img_width << ", height=" << img_height << "\n";
		cout << "Pixel Depth: " << pixel_depth << "\n";
		cout << "Image Descriptor: " << img_descriptor << "\n";
		cout << "  alpha_channel_depth=" << alpha_channel_depth << "\n";
		cout << "  pixel_order[0]=" << pixel_order[0] << ", pixel_order[1]=" << pixel_order[1] << "\n";
	}

	if (compressionType != 2) {
		cout << "\nWARNING: The inputted .tga file has the compression type: " << compressionType << ".\n  This compression type is not supported by this program. You may see a corrupted image. If you wish to actually see the image then please open it using a different programm.\n";
	}

	readBody(byteNum, fileContent);
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_EVERYTHING);	
	
	SDL_Event eve; // Event to manage (for later)
	
	bool running = true;
	
	if (argc == 2) {
		filePath = argv[1];
		cout << filePath + "\n";	
	} else if (argc == 3) {
		filePath = argv[1];
		if (strcmp(argv[2],"display-information") == 0) {
			displayInformation = true;		
		}
	} else {
		cout << "Please enter a file path to open!";
		return 0;
	}

	ifstream readFileContent(filePath, ifstream::binary);
	if (!readFileContent) {cout << "Could not read \"" << filePath << "\" please enter a valid file.\n"; return -1;}
	/*string lineContenti;
	string fileContent;*/
	
	/*
	while (getline (readFileContent, lineContent)) {
		fileContent += lineContent;
	}*/

	readFileContent.seekg(0, readFileContent.end);
	int fileLength = readFileContent.tellg();
	readFileContent.seekg(0, readFileContent.beg);
	
	char* fileContent = new char[fileLength];

	readFileContent.read(fileContent, fileLength);

	/*string fileContent(
    		(istreambuf_iterator<char>(filePath)),
	    	istreambuf_iterator<char>()
	);*/

	readHeader(fileContent/*.c_str()*/);
	delete[] fileContent;

	SDL_Window *wind = SDL_CreateWindow(".tga Image Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, img_width, img_height, /*SDL_WINDOW_SHOWN || SDL_WINDOW_OPENGL*/0);
	SDL_Surface *wind_surf = SDL_GetWindowSurface(wind);
	SDL_UpdateWindowSurface(wind);

	if (wind == nullptr || wind_surf == nullptr) {cout << "Failed to launch"; return -1;}
	
	int debugPixelCount = 0;
	// Display Pixels:
	for (int i = 0; i < totalPixelAmount; i++) {
		SDL_Rect pixel;
		pixel.x = allPixels[i].x;
		pixel.y = allPixels[i].y;
		pixel.w = 1; pixel.h = 1;
		debugPixelCount++;
		SDL_FillRect(wind_surf, &pixel, allPixels[i].color);
	}
	SDL_UpdateWindowSurface(wind);
	if (displayInformation) cout << "\nPixel Count: " << debugPixelCount << "\n";


	while (running) {
		if (SDL_WaitEvent(&eve)) {
			if (eve.type == SDL_QUIT) {
				running = false;
			}
		}
		SDL_Delay(1);
	}

	SDL_Quit();
	return 0;
}
