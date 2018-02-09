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

	int fromWidth = 1200;
	int fromHeight = 1200;
	int toWidth = 300;
	int toHeight = 300;
	double rateWidth = (double)fromWidth / (double)toWidth; 
	double rateHeight = (double)fromHeight / (double)toHeight; 

	int baseArea = sizeWidth * sizeHeight;

	HImage newImage;
	GenImageConst(&newImage, "byte", toWidth, toHeight);

	HTuple seqX;
	TupleGenSequence(0, toWidth - 1, 1, &seqX);

	for (int i = 0; i < toHeight; i++)
	{
		int row1 = (int)(i * rateHeight);
		int row2 = row1 + sizeHeight - 1;
		HTuple val = HTuple();

		HRegion bandRegion;
		ClipRegion(region, &bandRegion, row1, 0, row2, fromWidth - 1);

		for (int j = 0; j < toWidth; j++)
		{
			int column1 = (int)(j * rateWidth);
			int column2 = column1 + sizeWidth - 1;

			HRegion regionClipped;

			ClipRegion(bandRegion, &regionClipped, row1, column1, row2, column2);

			HTuple area;
			RegionFeatures(regionClipped, "area", &area);

			val.Append(area * 255.0 / baseArea);
		}

		HTuple seqY;
		TupleGenConst(toHeight, i, &seqY);
			
		SetGrayval(newImage, seqY, seqX, val);
	}

	HalconCpp::WriteImage(newImage, "tiff", 0, &destinationPath[0]);

}


int main()
{
	SetSystem("clip_region", "false");

	std::chrono::system_clock::time_point  start, end; // 型は auto で可
	start = std::chrono::system_clock::now(); // 計測開始時間

	// 処理
	string pathBase = "C:/Users/tamaki/Documents/SampleImages/cmyk-1bit";
	string pathC = pathBase + "/gen2/background_crop-c.tif";
	string pathM = pathBase + "/gen2/background_crop-m.tif";
	string pathY = pathBase + "/gen2/background_crop-y.tif";
	string pathK = pathBase + "/gen2/background_crop-k.tif";
	string pathSaveC = pathBase + "/gen2/background_crop-8bit-c.tif";
	string pathSaveM = pathBase + "/gen2/background_crop-8bit-m.tif";
	string pathSaveY = pathBase + "/gen2/background_crop-8bit-y.tif";
	string pathSaveK = pathBase + "/gen2/background_crop-8bit-k.tif";
	int sizeWidth = 4;
	int sizeHeight = 4;

	if (std::ifstream(pathSaveC))
		std::remove(&pathSaveC[0]);
	if (std::ifstream(pathSaveM))
		std::remove(&pathSaveM[0]);
	if (std::ifstream(pathSaveY))
		std::remove(&pathSaveY[0]);
	if (std::ifstream(pathSaveK))
		std::remove(&pathSaveK[0]);

	ConvertOneToEight(pathC, pathSaveC, sizeWidth, sizeHeight);
	ConvertOneToEight(pathM, pathSaveM, sizeWidth, sizeHeight);
	ConvertOneToEight(pathY, pathSaveY, sizeWidth, sizeHeight);
	ConvertOneToEight(pathK, pathSaveK, sizeWidth, sizeHeight);

	end = std::chrono::system_clock::now();  // 計測終了時間
	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); //処理に要した時間をミリ秒に変換

	std::cout << fixed << std::setprecision(2) << elapsed << endl;

	return 0;

	//HImage Mandrill("monkey"); // read image from file "monkey"
	//Hlong width, height;
	//Mandrill.GetImageSize(&width, &height);
	//HWindow w(0, 0, width, height); // window with size equal to image
	//Mandrill.DispImage(w); // display image in window
	//w.Click(); // wait for mouse click
	//w.ClearWindow();
	//HRegion Bright = Mandrill >= 128; // select all bright pixels
	//HRegion Conn = Bright.Connection(); // get connected components
	//// select regions with a size of at least 500 pixels
	//HRegion Large = Conn.SelectShape("area", "and", 500, 90000);
	//// select the eyes out of the instance variable Large by using
	//// the anisometry as region feature:
	//HRegion Eyes = Large.SelectShape("anisometry", "and", 1, 1.7);
	//Eyes.DispRegion(w); // display result image in window
	//w.Click(); // wait for mouse click

	return 0;
}
