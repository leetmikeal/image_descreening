// OneBitTiff.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "Compose.hpp"

using namespace HalconCpp;
using namespace std;

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

	TIFF *image; image=TIFFOpen(&pathSave[0],"w"); //test.tif�Ƃ����t�@�C��������
	if(image==NULL){ 
		return; 
	} 
	//TIFF�̃^�O�i�⑫���j�ݒ�
	TIFFSetField(image,TIFFTAG_IMAGEWIDTH,width); //�摜��
	TIFFSetField(image,TIFFTAG_IMAGELENGTH,height); //�摜�̍���
	TIFFSetField(image,TIFFTAG_BITSPERSAMPLE,8,8,8,8); //�摜�̐[����8bit
	TIFFSetField(image,TIFFTAG_ROWSPERSTRIP, height);
	TIFFSetField(image,TIFFTAG_SAMPLESPERPIXEL,ch); //�F�̐�
	TIFFSetField(image,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED); //���
	TIFFSetField(image,TIFFTAG_COMPRESSION,COMPRESSION_NONE);//���k�Ȃ� 
	//TIFFSetField(image,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_MINISBLACK); //0�����ŁA�ő�l���� �O���[�摜�̂Ƃ�
	TIFFSetField(image,TIFFTAG_FILLORDER,FILLORDER_MSB2LSB); 
	TIFFSetField(image,TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG); 
	TIFFSetField(image,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT); 
    TIFFSetField(image,TIFFTAG_INKSET, INKSET_CMYK);
    TIFFSetField(image,TIFFTAG_NUMBEROFINKS, 4);
	TIFFSetField(image,TIFFTAG_XRESOLUTION,72.0); //�������𑜓x
	TIFFSetField(image,TIFFTAG_YRESOLUTION,72.0); //���������𑜓x
	TIFFSetField(image,TIFFTAG_RESOLUTIONUNIT,RESUNIT_CENTIMETER);//�𑜓x�̒P�ʂ̓Z���`���[�g�� 
	TIFFSetField(image,TIFFTAG_SOFTWARE,"OneBitTiff"); //�\�t�g�̖��O
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

bool checkFileExistence(const std::string& str)
{
    std::ifstream ifs(str);
    return ifs.is_open();
}

int main(int argc, char* argv[]) 
{
	// time measurement
	std::chrono::system_clock::time_point  start, end;
	start = std::chrono::system_clock::now();

	// resources
	string pathBase         = "C:/Users/tamaki/source/repos/image_descreening/Resources";
	string pathSaveC        = pathBase + "/bfr-8bit-c.tif";
	string pathSaveM        = pathBase + "/bfr-8bit-m.tif";
	string pathSaveY        = pathBase + "/bfr-8bit-y.tif";
	string pathSaveK        = pathBase + "/bfr-8bit-k.tif";
	string pathSaveComposed = pathBase + "/bfr-8bit-4cmyk.tif";

	// get from arguments
	if (argc > 1)
	{
		if (argc != 6)
		{
			cout << "failed : command required only 4 arguments." << endl;
			return 0;
		}

		pathSaveC = argv[1];
		pathSaveM = argv[2];
		pathSaveY = argv[3];
		pathSaveK = argv[4];
		pathSaveComposed = argv[5];
	}

	// check file exists
	if (!checkFileExistence(pathSaveC)
		|| !checkFileExistence(pathSaveM)
		|| !checkFileExistence(pathSaveY)
		|| !checkFileExistence(pathSaveK))
	{
		cout << "failed : input file not found." << endl;
		return 0;
	}

	// make 4 channel image
	if (std::ifstream(pathSaveComposed))
		std::remove(&pathSaveComposed[0]);
	//ComposeByHalcon(pathSaveC, pathSaveM, pathSaveY, pathSaveK, pathSaveComposed);
	ComposeByLibTiff(pathSaveC, pathSaveM, pathSaveY, pathSaveK, pathSaveComposed);

	// time measurement
	end = std::chrono::system_clock::now(); 
	double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); // msec.

	// output
	std::cout << fixed << std::setprecision(2) << elapsed;
	std::cout << " msec." << endl;

	return 0;
}
