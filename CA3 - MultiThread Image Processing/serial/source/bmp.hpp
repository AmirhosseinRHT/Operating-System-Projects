#ifndef BMP_HPP
#define BMP_HPP

#include "timer.hpp"

class Bitmap
{
public:
    Bitmap(string _file_name);
    bool read_bitmap_headers(char*& buffer);
    void read_bitmap_pixels(char* buffer);
    void write_bitmap_file(string output_file);
    void purple_haze_filter();
    void flip_vertically();
    void blur();
    void diagonalـline();
    bool run();
    void convolution(int i , int j , int kernel_half_size);
private: 
    vector<vector<Pixel>> image;
    vector<vector<int>> kernel;
    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;
    string file_name;
    Timer timer;
    int buffer_size;
    int rows;
    int cols;
};

#endif