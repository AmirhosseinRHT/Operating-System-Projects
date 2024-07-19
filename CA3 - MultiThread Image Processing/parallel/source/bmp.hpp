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
    static void* partial_read_pixels(void* arg);
    static void* partial_file_write(void* arg);
    static void* partial_convolution(void* arg);
    static void* partial_purple_filter(void* arg);
    static void* partial_vertical_flip(void* arg);
    static void* partial_sketch(void* arg);
    void flip_vertically();
    void blur();
    void sketch();
    bool run();
private: 
    vector<vector<Pixel>> image;
    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;
    string file_name;
    Timer timer;
    int buffer_size;
    int rows;
    int cols;
};
#endif