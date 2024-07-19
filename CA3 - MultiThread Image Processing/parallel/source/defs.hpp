#ifndef DEFS_HPP
#define DEFS_HPP

#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>
#include <algorithm>
#include <fstream>
#include <pthread.h>

using namespace std;

class Bitmap;

typedef unsigned short ushort;
typedef unsigned int uint;

typedef struct 
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Pixel;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER 
{
    ushort bfType;
    uint bfSize;
    ushort bfReserved1;
    ushort bfReserved2;
    uint bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER 
{
    uint biSize;
    int biWidth;
    int biHeight;
    ushort biPlanes;
    ushort biBitCount;
    uint biCompression;
    uint biSizeBitmap;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    uint biClrUsed;
    uint biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

struct FiltersThreadData 
{
    vector<vector<Pixel>> *image;
    int start_row;
    int end_row;
};

struct IOThreadData 
{
    Bitmap* bitmap;
    char* buffer;
    int start_index;
    int end_index;
};

static vector<vector<int>> BLUR_KERNEL =
    {{1, 2, 1},
     {2, 4, 2},
     {1, 2, 1}};

static const int NUM_OF_THREADS = 16;

static Pixel WHITE = {255 , 255, 255};
#endif