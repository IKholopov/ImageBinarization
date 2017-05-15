#include <Windows.h>
#include <iostream>
#include <regex>
#include "BinarizationImage.h"

void PrintUsage() {
	std::wcout << "Usage: ImageBinarization.exe <image file> [-o <output file>] [--greyscale [<filename>] ] [-k <k_parameter>] [-R <R_parameter>] [--window <window_size> ][--niblack]" << std::endl;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp) {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	if (argc < 2) {
		PrintUsage();
		return 1;
	}
	std::vector<std::wstring> args;
	args.assign(argv + 1, argv + argc);
	std::wstring originalfile;
	originalfile = args[0];
	CBinarizationImage image(originalfile.c_str());
	if( !image.IsLoaded() ) {
		MessageBox(NULL, L"Failed to open requested file!", L"ImageBinarization", MB_ICONERROR);
		return 1;
	}
	bool saveGreyscale = false;
	size_t lastindex = originalfile.find_last_of(L".");
	std::wstring greyscale = originalfile.substr(0, lastindex).append(L"_greyscale.bmp");
	std::wstring binarized = originalfile.substr(0, lastindex).append(L"_binarized.bmp");
	float k = DEFAULT_SAUVOLA_K;
	float R = DEFAULT_SAUVOLA_R;
	int windowSize = DEFAULT_WINDOW_SIZE;
	bool niblack = false;
	for( int i = 1; i < args.size(); ++i ) {
		if( !args[i].compare(L"--greyscale") ) {
			saveGreyscale = true;
			if( i < args.size() - 1 && args[i + 1][0] != L'-' ) {
				greyscale = args[i + 1].append(L".bmp");
			}
		}
		if( !args[i].compare(L"-o") ) {
			if( i == args.size() - 1 ) {
				PrintUsage();
				return 1;
			}
			binarized = args[i + 1].append(L".bmp");
		}
		if( !args[i].compare(L"-k") ) {
			if( i == args.size() - 1 ) {
				PrintUsage();
				return 1;
			}
			k = std::stof(args[i + 1]);
		}
		if(!args[i].compare(L"-R")) {
			if( i == args.size() - 1 ) {
				PrintUsage();
				return 1;
			}
			R = std::stof(args[i + 1]);
		}
		if( !args[i].compare(L"--niblack") ) {
			niblack = true;
			if( k > 0 ) {
				k = DEFAULT_NIBLACK_K;
			}
		}
		if( !args[i].compare(L"--window") ) {
			if( i == args.size() - 1 ) {
				PrintUsage();
				return 1;
			}
			windowSize = std::stoi(args[i + 1]);
		}
	}
	if( niblack ) {
		image.Niblack(binarized, k, windowSize);
	}
	else {
		image.Sauvola(binarized, k, R, windowSize);
	}
	if( saveGreyscale ) {
		image.SaveGreyscale(greyscale);
	}
	
	return 0;
}