// OneBitTiff.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "OneBitTiff.hpp"

using namespace HalconCpp;
using namespace std;

int GetImageSize(string path, int *width, int *height, int *bitPerPixel)
{
	TIFF *image;
	uint16 bps;
	uint32 w, h;

	// Open the TIFF image
	if ((image = TIFFOpen(&path[0], "r")) == NULL) {
		fprintf(stderr, "Could not open incoming image \n");
		//exit(42);
		return -1;
	}
	if ((TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &w) == 0) ) {
		fprintf(stderr, "Either undefined or unsupported width \n");
		return -1;
	}
	if ((TIFFGetField(image, TIFFTAG_IMAGELENGTH, &h) == 0) ) {
		fprintf(stderr, "Either undefined or unsupported height \n");
		return -1;
	}
	if ((TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &bps) == 0) ) {
		fprintf(stderr, "Either undefined or unsupported number of bits per sample \n");
		return -1;
	}

	*width = w;
	*height = h;
	*bitPerPixel = bps;

	return 0;
}

void ConvertOneToEight(string sourcePath, string destinationPath, int scale, int margin)
{
	HRegion region;
	
	region.ReadRegion(&sourcePath[0]);
	Complement(region, &region);

	int width, height, bitPerPixel;
	GetImageSize(sourcePath, &width, &height, &bitPerPixel);

	// image size
	int fromWidth = width;
	int fromHeight = height;
	int toWidth = width / scale;
	int toHeight = height / scale;
	double rateWidth = scale; 
	double rateHeight = scale; 
	//toWidth = 1000;
	//toHeight = 1000;

	int scaleHalf = (int)(floor((double)scale / 2.0));
	int scaleHalfMargin = scaleHalf + margin;
	int scalePlusMarign = scale + margin * 2;

	int marginArea = scalePlusMarign * scalePlusMarign;
	int baseArea = scale * scale + marginArea; // overlap 2 times center area

	byte* newImagePtr = new byte[toWidth * toHeight];

	// to write one line
	HTuple seqX;
	TupleGenSequence(0, toWidth - 1, 1, &seqX);

	for (int i = 0; i < toHeight; i++)
	{
		// base
		int row1 = (int)(i * rateHeight) - scaleHalf;
		int row2 = row1 + scale - 1;

		// margin
		int row1m = row1 - margin;
		int row2m = row2 + margin;

		// for performance, once clip horizontal region
		HRegion bandRegion;
		ClipRegion(region, &bandRegion, row1m, 0, row2m, fromWidth - 1);

		for (int j = 0; j < toWidth; j++)
		{
			// base
			int column1 = (int)(j * rateWidth) - scaleHalf;
			int column2 = column1 + scale - 1;

			// margin
			int column1m = column1 - margin;
			int column2m = column2 + margin;

			HRegion regionClipped;

			// base - clip
			ClipRegion(bandRegion, &regionClipped, row1, column1, row2, column2);

			HTuple area;
			RegionFeatures(regionClipped, "area", &area);

			// margin - clip
			ClipRegion(bandRegion, &regionClipped, row1m, column1m, row2m, column2m);

			HTuple areaMargin;
			RegionFeatures(regionClipped, "area", &areaMargin);

			// area
			byte v = (byte)(((area + areaMargin) * 255.0 / baseArea).D());
			newImagePtr[i * toWidth + j] = v;
		}
	}

	// save file
	HImage newImage;
	HalconCpp::GenImage1(&newImage, "byte", toWidth, toHeight, (Hlong)newImagePtr);
	HalconCpp::WriteImage(newImage, "tiff lzw", 0, &destinationPath[0]);
}

bool checkFileExistence(const std::string& str)
{
    std::ifstream ifs(str);
    return ifs.is_open();
}

void help()
{
	cout << "OneBitTiff [input] [output] [scale] [margin]" << endl;
}

int main(int argc, char* argv[]) 
{
	SetSystem("clip_region", "false");
	SetCheck("all");

	// time measurement
	std::chrono::system_clock::time_point  start, end;
	start = std::chrono::system_clock::now();

	// resources
	string pathBase         = "C:/Users/tamaki/source/repos/image_descreening/Resources";
	string pathC            = pathBase + "/bfr-1bit-c.tif";
	string pathSaveC        = pathBase + "/bfr-8bit-c.tif";
	//string pathSaveComposed = pathBase + "/bfr-8bit-4cmyk.tif";

	// argument
	int scale = 4;
	int margin = 0;
	if (argc > 1)
	{
		if (argc != 5)
		{
			cout << "invalid arguments." << endl;
			help();
			return 0;
		}

		pathC = argv[1];
		pathSaveC = argv[2];

		istringstream sScale(argv[3]);
		if (!(sScale >> scale))
			cerr << "Invalid number " << argv[3] << '\n';

		istringstream sMargin(argv[4]);
		if (!(sMargin >> margin))
			cerr << "Invalid number " << argv[4] << '\n';
	}

	// check file exists
	if (!checkFileExistence(pathC))
	{
		cout << "failed : input file not found." << endl;
		return 0;
	}

	// overwrite file
	if (std::ifstream(pathSaveC))
		std::remove(&pathSaveC[0]);
	// main process
	ConvertOneToEight(pathC, pathSaveC, scale, margin);

	// time measurement
	end = std::chrono::system_clock::now(); 
	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); // msec.

	// output
	std::cout << fixed << std::setprecision(2) << elapsed;
	std::cout << " msec." << endl;

	return 0;
}
