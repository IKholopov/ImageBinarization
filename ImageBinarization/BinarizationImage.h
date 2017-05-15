#pragma once

#include <Windows.h>
#include <gdiplus.h>
#include <memory>
#include <vector>

#define DEFAULT_WINDOW_SIZE 10
#define DEFAULT_SAUVOLA_K 0.5f
#define DEFAULT_SAUVOLA_R 128
#define DEFAULT_NIBLACK_K -0.15f

class CBinarizationImage {
public:
	CBinarizationImage(const wchar_t* path);

	bool IsLoaded();

	void GenerateGreyscale();
	void Sauvola(std::wstring path, float k = DEFAULT_SAUVOLA_K, float R = DEFAULT_SAUVOLA_R, int windowSize = DEFAULT_WINDOW_SIZE);
	void Niblack(std::wstring path, float k = DEFAULT_NIBLACK_K, int windowSize = DEFAULT_WINDOW_SIZE);
	void SaveGreyscale(std::wstring path);
private:

	int width;
	int height;
	std::unique_ptr<Gdiplus::Bitmap> original;
	std::unique_ptr<BYTE[]> imageData;
	std::unique_ptr<BYTE[]> greyscaleByte;

	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	void ApplySauvolaThreashhold(BYTE* binarized, int windowX, int windowY, float k, float R, int windowSize);
	void ApplyNiblackThreashhold(BYTE* binarized, int windowX, int windowY, float k, int windowSize);
	double Mean(int windowStartX, int windowStartY, int windowWidth, int windowHeight);
	double StDiv(int windowStartX, int windowStartY, int windowWidth, int windowHeight, double mean);
	void PixelToBin(BYTE value, BYTE* dest, int x, int y);
};