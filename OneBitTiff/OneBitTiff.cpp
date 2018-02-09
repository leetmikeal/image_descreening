// OneBitTiff.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "HalconCpp.h"

using namespace HalconCpp;
using namespace std;

void ConvertOneToEight(string sourcePath, string destinationPath, int sizeWidth, int sizeHeight)
{
	HRegion region;
	
	region.ReadRegion(&sourcePath[0]);
	Complement(region, &region);

	// image size
	int fromWidth = 1200;
	int fromHeight = 1200;
	int toWidth = 300;
	int toHeight = 300;
	double rateWidth = (double)fromWidth / (double)toWidth; 
	double rateHeight = (double)fromHeight / (double)toHeight; 

	int baseArea = sizeWidth * sizeHeight;

	HImage newImage;
	GenImageConst(&newImage, "byte", toWidth, toHeight);

	// to write one line
	HTuple seqX;
	TupleGenSequence(0, toWidth - 1, 1, &seqX);

	for (int i = 0; i < toHeight; i++)
	{
		int row1 = (int)(i * rateHeight);
		int row2 = row1 + sizeHeight - 1;
		HTuple val = HTuple();

		// for performance, once clip horizontal region
		HRegion bandRegion;
		ClipRegion(region, &bandRegion, row1, 0, row2, fromWidth - 1);

		for (int j = 0; j < toWidth; j++)
		{
			int column1 = (int)(j * rateWidth);
			int column2 = column1 + sizeWidth - 1;

			HRegion regionClipped;

			// clip
			ClipRegion(bandRegion, &regionClipped, row1, column1, row2, column2);

			HTuple area;
			RegionFeatures(regionClipped, "area", &area);

			// area
			val.Append(area * 255.0 / baseArea);
		}

		HTuple seqY;
		TupleGenConst(toHeight, i, &seqY);
			
		// write one line
		SetGrayval(newImage, seqY, seqX, val);
	}

	// save file
	HalconCpp::WriteImage(newImage, "tiff", 0, &destinationPath[0]);
}

void ComposeByLibTiff(string pathC, string pathM, string pathY, string pathK, string pathSave)
{
	// read source images
	HImage imageC, imageM, imageY, imageK;
	imageC.ReadImage(&pathC[0]);
	imageM.ReadImage(&pathM[0]);
	imageY.ReadImage(&pathY[0]);
	imageK.ReadImage(&pathK[0]);

	// get pointer
	HTuple pointerC, pointerM, pointerY, pointerK;
	HTuple t, w, h;
	GetImagePointer1(imageC, &pointerC, &t, &w, &h);
	GetImagePointer1(imageM, &pointerM, &t, &w, &h);
	GetImagePointer1(imageY, &pointerY, &t, &w, &h);
	GetImagePointer1(imageK, &pointerK, &t, &w, &h);

	unsigned int width = w.I();
	unsigned int height = h.I();
	unsigned int ch = 4;
	//unsigned char *ptrC, *ptrM, *ptrY, *ptrK;
	char * ptrC = (char *)pointerC.L();
	char * ptrM = (char *)pointerM.L();
	char * ptrY = (char *)pointerY.L();
	char * ptrK = (char *)pointerK.L();

	char* buffer = (char *)malloc(width*height*ch * sizeof(char));
	for (unsigned i = 0; i < width; i++) {
		for (unsigned j = 0; j < height; j++) {
			buffer[(j*width + i)*ch + 0] = ptrC[j*width + i];
			buffer[(j*width + i)*ch + 1] = ptrM[j*width + i];
			buffer[(j*width + i)*ch + 2] = ptrY[j*width + i];
			buffer[(j*width + i)*ch + 3] = ptrK[j*width + i];
		}
	} 

	TIFF *image; image=TIFFOpen(&pathSave[0],"w"); //test.tifというファイルをつくる
	if(image==NULL){ 
		return; 
	} 
	//TIFFのタグ（補足情報）設定
	TIFFSetField(image,TIFFTAG_IMAGEWIDTH,width); //画像幅
	TIFFSetField(image,TIFFTAG_IMAGELENGTH,height); //画像の高さ
	TIFFSetField(image,TIFFTAG_BITSPERSAMPLE,8,8,8,8); //画像の深さは8bit
	TIFFSetField(image,TIFFTAG_ROWSPERSTRIP, height);
	TIFFSetField(image,TIFFTAG_SAMPLESPERPIXEL,ch); //色の数
	TIFFSetField(image,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED); //種類
	TIFFSetField(image,TIFFTAG_COMPRESSION,COMPRESSION_NONE);//圧縮なし 
	//TIFFSetField(image,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_MINISBLACK); //0が黒で、最大値が白 グレー画像のとき
	TIFFSetField(image,TIFFTAG_FILLORDER,FILLORDER_MSB2LSB); 
	TIFFSetField(image,TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG); 
	TIFFSetField(image,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT); 
    TIFFSetField(image,TIFFTAG_INKSET, INKSET_CMYK);
    TIFFSetField(image,TIFFTAG_NUMBEROFINKS, 4);
	TIFFSetField(image,TIFFTAG_XRESOLUTION,72.0); //横方向解像度
	TIFFSetField(image,TIFFTAG_YRESOLUTION,72.0); //高さ方向解像度
	TIFFSetField(image,TIFFTAG_RESOLUTIONUNIT,RESUNIT_CENTIMETER);//解像度の単位はセンチメートル 
	TIFFSetField(image,TIFFTAG_SOFTWARE,"OneBitTiff"); //ソフトの名前
	TIFFWriteEncodedStrip(image,0,buffer,width*height*ch); 
	TIFFClose(image); 
	free(buffer);
}

void ComposeByHalcon(string pathC, string pathM, string pathY, string pathK, string pathSave)
{
	// read source images
	HImage imageC, imageM, imageY, imageK;
	imageC.ReadImage(&pathC[0]);
	imageM.ReadImage(&pathM[0]);
	imageY.ReadImage(&pathY[0]);
	imageK.ReadImage(&pathK[0]);

	// commpose
	HImage image;
	Compose4(imageC, imageM, imageY, imageK, &image);

	// save
	image.WriteImage("tiff packbits", 0, &pathSave[0]);
}


int main()
{
	SetSystem("clip_region", "false");

	// time measurement
	std::chrono::system_clock::time_point  start, end;
	start = std::chrono::system_clock::now();

	// resources
	string pathBase         = "C:/Users/tamaki/source/repos/image_descreening/Resources";
	string pathC            = pathBase + "/bfr-1bit-c.tif";
	string pathM            = pathBase + "/bfr-1bit-m.tif";
	string pathY            = pathBase + "/bfr-1bit-y.tif";
	string pathK            = pathBase + "/bfr-1bit-k.tif";
	string pathSaveC        = pathBase + "/bfr-8bit-c.tif";
	string pathSaveM        = pathBase + "/bfr-8bit-m.tif";
	string pathSaveY        = pathBase + "/bfr-8bit-y.tif";
	string pathSaveK        = pathBase + "/bfr-8bit-k.tif";
	string pathSaveComposed = pathBase + "/bfr-8bit-4cmyk.tif";

	// average window size
	int sizeWidth = 4;
	int sizeHeight = 4;

	// overwrite file
	if (std::ifstream(pathSaveC))
		std::remove(&pathSaveC[0]);
	if (std::ifstream(pathSaveM))
		std::remove(&pathSaveM[0]);
	if (std::ifstream(pathSaveY))
		std::remove(&pathSaveY[0]);
	if (std::ifstream(pathSaveK))
		std::remove(&pathSaveK[0]);
	// main process
	ConvertOneToEight(pathC, pathSaveC, sizeWidth, sizeHeight);
	ConvertOneToEight(pathM, pathSaveM, sizeWidth, sizeHeight);
	ConvertOneToEight(pathY, pathSaveY, sizeWidth, sizeHeight); 
	ConvertOneToEight(pathK, pathSaveK, sizeWidth, sizeHeight);

	// time measurement
	end = std::chrono::system_clock::now(); 
	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); // msec.

	// make 4 channel image
	if (std::ifstream(pathSaveComposed))
		std::remove(&pathSaveComposed[0]);
	//ComposeByHalcon(pathSaveC, pathSaveM, pathSaveY, pathSaveK, pathSaveComposed);
	ComposeByLibTiff(pathSaveC, pathSaveM, pathSaveY, pathSaveK, pathSaveComposed);

	// output
	std::cout << fixed << std::setprecision(2) << elapsed;
	std::cout << " msec." << endl;

	return 0;
}
