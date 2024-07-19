#include "bmp.hpp"

Bitmap::Bitmap(string _file_name)
{
    file_name = _file_name;
    buffer_size = 0 ;
    kernel = 
    {{1, 2, 1},
     {2, 4, 2},
     {1, 2, 1}};
}

bool Bitmap::read_bitmap_headers(char*& buffer) 
{
    timer.start_timer();
    std::ifstream file(file_name, std::ios::binary);
    if (!file) 
    {
        std::cout << "File " << file_name << " doesn't exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);
    buffer = new char[length];
    file.read(&buffer[0], length);
    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[sizeof(BITMAPFILEHEADER)]);
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    buffer_size = file_header->bfSize;
    timer.stop_timer("Read");
    return true;
}

void Bitmap::read_bitmap_pixels(char* buffer) 
{
    timer.start_timer();
    int count = 0 , extra = (4 - (cols * 3) % 4) % 4;
    for (int i = 0; i < rows; i++) 
    {
        vector <Pixel> row;
        for (int j = 0; j < cols; j++) 
        {
            Pixel pixel;
            for (int k = 0; k < 3; k++) 
            {
                if (k == 0)
                    pixel.blue = buffer[buffer_size-1 - count];
                else if(k == 1)
                    pixel.green = buffer[buffer_size-1 - count];
                else if (k == 2)
                    pixel.red = buffer[buffer_size-1 - count];
                count++;
            }
            row.push_back(pixel);
        }
        image.push_back(row);
        count += extra;
    }
    timer.stop_timer("Save Pixels");
}

void Bitmap::write_bitmap_file(string output_file) 
{
    timer.start_timer();
    ofstream file(output_file, ios::binary);
    if (!file) 
    {
        cout << "Failed to file " << output_file << endl;
        exit(EXIT_FAILURE);
    }
    file.write(reinterpret_cast<const char*>(file_header), sizeof(BITMAPFILEHEADER));
    file.write(reinterpret_cast<const char*>(info_header), sizeof(BITMAPINFOHEADER));

    int padding = (4 - (image[0].size() * 3) % 4) % 4;
    for (int i = image.size() - 1; i >= 0; i--) 
    { 
        for (int j = image[i].size() - 1 ; j >= 0; j--) 
        {
            file.write(reinterpret_cast<const char*>(&image[i][j].blue), sizeof(unsigned char));
            file.write(reinterpret_cast<const char*>(&image[i][j].green), sizeof(unsigned char));
            file.write(reinterpret_cast<const char*>(&image[i][j].red), sizeof(unsigned char));
        }
        char padding_bytes[3] = {0x00, 0x00, 0x00};
        file.write(padding_bytes, padding);
    }
    file.close();
    timer.stop_timer("Write");
}

void Bitmap::purple_haze_filter() 
{
    timer.start_timer();
    for (int i = 0; i < image.size(); i++)
        for (int j = 0; j < image[i].size(); j++)
        {
            Pixel temp = {image[i][j].red , image[i][j].green , image[i][j].blue};
            image[i][j].red = min(255, (unsigned char)(0.5 * (temp.red + temp.blue)) + (unsigned char)(0.3 * temp.green));
            image[i][j].green = min(255,(unsigned char)(0.16 * (temp.red + temp.blue)) + (unsigned char)(0.5 * temp.green));
            image[i][j].blue = min(255 ,(unsigned char)(0.6 * temp.red) + (unsigned char)(0.2 * temp.green) + (unsigned char)(0.8 * temp.blue));
        }
    timer.stop_timer("Purple");
}

void Bitmap::flip_vertically()
{
    timer.start_timer();
    vector <Pixel> temp;
    for(int i  =  0 ; i  < image.size() / 2 ; i++)
    {
        temp = image[i];
        image[i] = image[image.size() - i - 1];
        image[image.size() - i - 1] = temp;
    }
    timer.stop_timer("Flip");
}

void Bitmap::blur() 
{
    timer.start_timer();
    int rows = image.size() , cols = image[0].size();
    int kernel_half_size = kernel.size() / 2;
    for (int i = kernel_half_size; i < rows - kernel_half_size; i++) 
        for (int j = kernel_half_size; j < cols - kernel_half_size; j++) 
            convolution(i , j, kernel_half_size);
    timer.stop_timer("Blur");
}

void Bitmap::convolution(int i , int j , int kernel_half_size)
{
    double sum_red = 0 , sum_green = 0 , sum_blue = 0;
    for (int ki = 0; ki < kernel.size(); ki++) 
        for (int kj = 0; kj < kernel.size(); kj++) 
        {
            sum_red += (image[i - kernel_half_size + ki][j - kernel_half_size + kj].red * kernel[ki][kj]) / 16;
            sum_green += (image[i - kernel_half_size + ki][j - kernel_half_size + kj].green * kernel[ki][kj]) / 16;
            sum_blue += (image[i - kernel_half_size + ki][j - kernel_half_size + kj].blue * kernel[ki][kj]) / 16;
        }
    image[i][j].red = min(max(sum_red, 0.0), 255.0) ;
    image[i][j].green = min(max(sum_green, 0.0), 255.0) ;
    image[i][j].blue = min(max(sum_blue, 0.0), 255.0) ;
}


void Bitmap::diagonalـline()
{
    timer.start_timer();
    int mid_height = ceil(image.size() / 2 );
    int mid_width = ceil(image[0].size() / 2 );
    for (int i = 0; i < image.size(); i++)
    {
        image[i][i] = WHITE;
        if( i  < image.size() / 2)
        {
            image[i + mid_width][i] = WHITE;
            image[i][i + mid_height] = WHITE;
        }
    }
    timer.stop_timer("Sketch");
}

bool Bitmap::run()
{
    char* buffer;
    read_bitmap_headers(buffer);
    read_bitmap_pixels(buffer);
    flip_vertically();
    blur();
    purple_haze_filter();
    diagonalـline();
    write_bitmap_file("output.bmp");
    delete[] buffer;
    timer.print_duration();
    return true;
}