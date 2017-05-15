#include "BinarizationImage.h"

#include <iostream>

CBinarizationImage::CBinarizationImage(const wchar_t * path)
{
	original = std::move(std::unique_ptr<Gdiplus::Bitmap>(Gdiplus::Bitmap::FromFile(path, true)));
	width = original->GetWidth() - original->GetWidth() % 2;
	height = original->GetHeight();
}

bool CBinarizationImage::IsLoaded()
{
	return original->GetLastStatus() == Gdiplus::Ok;
}

void CBinarizationImage::GenerateGreyscale()
{
	imageData = std::move(std::unique_ptr<BYTE[]>(new BYTE[width * height]));
	greyscaleByte = std::move(std::unique_ptr<BYTE[]>(new BYTE[2 * width * height + 1]));
	greyscaleByte[0] = 0;
	for( int i = 0; i < height; ++i ) {
		for( int j = 0; j < width; ++j ) {
			Gdiplus::Color color;
			original->GetPixel(j, i, &color);
			BYTE value = (color.GetRed() + color.GetGreen() + color.GetBlue()) / 3;
			imageData[i * width + j] = value;
			PixelToBin(value, greyscaleByte.get(), j, i);
		}
	}
}

void CBinarizationImage::Sauvola(std::wstring path, float k, float R, int windowSize)
{
	GenerateGreyscale();
	std::unique_ptr<BYTE[]> binarizedImage(new BYTE[2*width*height + 1]);
	binarizedImage[0] = 0;
	for( int i = 0; i < height / windowSize + (height % windowSize ? 1 : 0); ++i) {
		for( int j = 0; j < width / windowSize + (width % windowSize ? 1 : 0); ++j ) {
			ApplySauvolaThreashhold(binarizedImage.get(), j, i, k, R, windowSize);
		}
	}
	Gdiplus::Bitmap bitmap(width, height, 2 * width / 4 * 4, PixelFormat16bppRGB565, &binarizedImage[0]);
	CLSID pngClsid;
	GetEncoderClsid(L"image/bmp", &pngClsid);
	auto status = bitmap.Save(path.c_str(), &pngClsid);
	if( status != Gdiplus::Ok ) {
		MessageBox(NULL, L"Failed to save file", L"ImageBinarization", MB_ICONERROR);
	}
}

void CBinarizationImage::Niblack(std::wstring path, float k, int windowSize)
{
	GenerateGreyscale();
	std::unique_ptr<BYTE[]> binarizedImage(new BYTE[2 * width*height + 1]);
	binarizedImage[0] = 0;
	for( int i = 0; i < height / windowSize + (height % windowSize ? 1 : 0); ++i ) {
		for( int j = 0; j < width / windowSize + (width % windowSize ? 1 : 0); ++j ) {
			ApplyNiblackThreashhold(binarizedImage.get(), j, i, k, windowSize);
		}
	}
	Gdiplus::Bitmap bitmap(width, height, 2 * width / 4 * 4, PixelFormat16bppRGB565, &binarizedImage[0]);
	CLSID pngClsid;
	GetEncoderClsid(L"image/bmp", &pngClsid);
	auto status = bitmap.Save(path.c_str(), &pngClsid);
	if( status != Gdiplus::Ok ) {
		std::wcout << path.c_str() << std::endl;
		MessageBox(NULL, L"Failed to save file", L"ImageBinarization", MB_ICONERROR);
	}
}

void CBinarizationImage::SaveGreyscale(std::wstring path)
{
	Gdiplus::Bitmap bitmap(width, height, 2*width/4*4 , PixelFormat16bppRGB565, &greyscaleByte[0]);
	CLSID pngClsid;
	GetEncoderClsid(L"image/bmp", &pngClsid);
	auto status = bitmap.Save(path.c_str(), &pngClsid);
	if( status != Gdiplus::Ok ) {
		MessageBox(NULL, L"Failed to save file", L"ImageBinarization", MB_ICONERROR);
	}
}

int CBinarizationImage::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for( UINT j = 0; j < num; ++j )
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void CBinarizationImage::ApplySauvolaThreashhold(BYTE * binarized, int windowX, int windowY, float k, float R, int windowSize)
{
	int windowStartX = windowX * windowSize;
	int windowStartY = windowY * windowSize;
	int windowWidth = (windowX < width / windowSize) ? windowSize : width % windowSize;
	int windowHeight = (windowY < height / windowSize) ? windowSize : height % windowSize;
	double mean = Mean(windowStartX, windowStartY, windowWidth, windowHeight);
	double stDiv = StDiv(windowStartX, windowStartY, windowWidth, windowHeight, mean);
	double threshold = mean * (1 + k * (stDiv / R - 1));
	for( int i = windowStartY; i < windowStartY + windowHeight; ++i ) {
		for( int j = windowStartX; j < windowStartX + windowWidth; ++j ) {
			BYTE value = imageData[i * width + j] >= threshold ? 255 : 0;
			PixelToBin(value, binarized, j, i);
		}
	}
}

void CBinarizationImage::ApplyNiblackThreashhold(BYTE * binarized, int windowX, int windowY, float k, int windowSize)
{
	int windowStartX = windowX * windowSize;
	int windowStartY = windowY * windowSize;
	int windowWidth = (windowX < width / windowSize) ? windowSize : width % windowSize;
	int windowHeight = (windowY < height / windowSize) ? windowSize : height % windowSize;
	double mean = Mean(windowStartX, windowStartY, windowWidth, windowHeight);
	double stDiv = StDiv(windowStartX, windowStartY, windowWidth, windowHeight, mean);
	double threshold = mean + k * stDiv;
	for( int i = windowStartY; i < windowStartY + windowHeight; ++i ) {
		for( int j = windowStartX; j < windowStartX + windowWidth; ++j ) {
			BYTE value = imageData[i * width + j] >= threshold ? 255 : 0;
			PixelToBin(value, binarized, j, i);
		}
	}
}

double CBinarizationImage::Mean(int windowStartX, int windowStartY, int windowWidth, int windowHeight)
{
	double mean = 0;
	for( int i = windowStartY; i < windowStartY + windowHeight; ++i ) {
		for( int j = windowStartX; j < windowStartX + windowWidth; ++j ) {
			mean += imageData[i * width + j];
		}
	}
	mean /= windowWidth * windowHeight;
	return mean;
}

double CBinarizationImage::StDiv(int windowStartX, int windowStartY, int windowWidth, int windowHeight, double mean)
{
	double stDiv = 0;
	for( int i = windowStartY; i < windowStartY + windowHeight; ++i ) {
		for( int j = windowStartX; j < windowStartX + windowWidth; ++j ) {
			stDiv += std::pow((imageData[i * width + j] - mean), 2);
		}
	}
	stDiv = std::sqrt(stDiv / (windowWidth * windowHeight));
	return stDiv;
}

void CBinarizationImage::PixelToBin(BYTE value, BYTE * dest, int x, int y)
{
	dest[2 * (y * width + x) + 1] = (value / 8 << 3) | (value / 4 >> 3);
	dest[2 * (y * width + x) + 2] = (value / 8) | (value / 4 << 5);
}
