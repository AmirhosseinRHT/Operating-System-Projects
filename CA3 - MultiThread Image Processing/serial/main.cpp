#include "source/bmp.hpp"

int main(int argc, char* argv[]) 
{
    Bitmap my_img = Bitmap(argv[1]);
    my_img.run();
    return 0;
}