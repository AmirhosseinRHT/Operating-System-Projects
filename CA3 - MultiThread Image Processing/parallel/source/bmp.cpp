#include "bmp.hpp"
#include "cstring"

Bitmap::Bitmap(string _file_name)
{
    file_name = _file_name;
    buffer_size = 0 ;
    char* buffer;
    read_bitmap_headers(buffer);
    vector <vector <Pixel>> temp (rows , vector<Pixel>(cols));
    image = temp;
    read_bitmap_pixels(buffer);
}

bool Bitmap::read_bitmap_headers(char*& buffer) 
{
    timer.start_timer();
    ifstream file(file_name, ios::binary);
    if (!file) 
    {
        cout << "File " << file_name << " doesn't exist!" << endl;
        exit(EXIT_FAILURE);
    }
    file.seekg(0, ios::end);
    streampos length = file.tellg();
    file.seekg(0, ios::beg);
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

void* Bitmap::partial_read_pixels(void* arg) 
{
    IOThreadData* data = static_cast<IOThreadData*>(arg);
    Bitmap* bitmap = data->bitmap;
    int start_row = data->start_index;
    int end_row = data->end_index;
    char* buffer = data->buffer;
    int cols = bitmap->cols;
    int buffer_size = bitmap->buffer_size;
    int extra = (4 - (cols * 3) % 4) % 4;
    for (int i = start_row; i < end_row; i++)
        for (int j = 0; j < cols; j++)
            for (int k = 0; k < 3; k++)
            {
                int count = i * cols * 3 + j * 3 + k ;
                if (k == 0)
                    bitmap->image[i][j].red = buffer[buffer_size - 1 - count];
                else if (k == 1)
                    bitmap->image[i][j].green = buffer[buffer_size - 1 - count];
                else if (k == 2)
                    bitmap->image[i][j].blue = buffer[buffer_size - 1 - count];
            }
    pthread_exit(NULL);
}

void Bitmap::read_bitmap_pixels(char* buffer) 
{
    timer.start_timer();
    int threads_count = NUM_OF_THREADS / 4;
    pthread_t threads[threads_count];
    IOThreadData threadData[threads_count];
    int row_per_thread = rows / threads_count;
    int remaining_rows = rows % threads_count;
    int start_row = 0 , end_row = 0 ;
    for (int i = 0; i < threads_count; i++)
    {
        start_row = end_row;
        end_row = start_row + row_per_thread + (i < remaining_rows ? 1 : 0);
        threadData[i].start_index = start_row;
        threadData[i].end_index = end_row;
        threadData[i].bitmap = this;
        threadData[i].buffer = buffer;
        pthread_create(&threads[i], NULL, partial_read_pixels, &threadData[i]);
    }
    for (int i = 0; i < threads_count; i++)
        pthread_join(threads[i], NULL);
    timer.stop_timer("Save Pixels");
}


void* Bitmap::partial_file_write(void* arg)
{
    IOThreadData* data = static_cast<IOThreadData*>(arg);
    Bitmap* bitmap = data->bitmap;
    int padding = (4 - (bitmap->image[0].size() * 3) % 4) % 4;
    char *buffer = data->buffer;
    char padding_bytes[3] = {0x00, 0x00, 0x00};
    for (int i = data->end_index - 1; i >= data->start_index; i--)
    {
        size_t offset = (bitmap->image.size() - 1 - i) * bitmap->image[i].size() * (sizeof(Pixel) + padding);
        for (int j = bitmap->image[i].size() - 1; j >= 0; j--)
        {
            unsigned char pixel_data[3] = {bitmap->image[i][j].blue , bitmap->image[i][j].green , bitmap->image[i][j].red};
            memcpy(&buffer[offset], pixel_data, sizeof(Pixel));
            offset += sizeof(Pixel);
        }
        memcpy(&buffer[offset], padding_bytes, padding);
        offset += padding;
    }
    pthread_exit(NULL);
}

void Bitmap::write_bitmap_file(string output_file) 
{
    timer.start_timer();
    int threads_count = NUM_OF_THREADS / 4;
    pthread_t threads[threads_count];
    IOThreadData threadData[threads_count];
    int padding = (4 - (image[0].size() * 3) % 4) % 4;
    char buffer[((cols * sizeof(Pixel) + padding) * rows)];
    int rows_per_thread = image.size() / threads_count;
    int remaining_rows = image.size() % threads_count;
    int start_row = 0 , end_row = 0;
    for (int i = 0; i < threads_count; i++)
    {
        end_row = start_row + rows_per_thread;
        if (i < remaining_rows)
            end_row++;
        threadData[i] = IOThreadData{this, buffer, start_row, end_row};
        pthread_create(&threads[i], nullptr, partial_file_write,  &threadData[i]);
        start_row = end_row;
    }
    for (int i = 0; i < threads_count; i++)
        pthread_join(threads[i], NULL);
    ofstream file(output_file, ios::binary);
    file.write(reinterpret_cast<const char*>(file_header), sizeof(BITMAPFILEHEADER));
    file.write(reinterpret_cast<const char*>(info_header), sizeof(BITMAPINFOHEADER));
    file.write(reinterpret_cast<const char*>(&buffer[0]), sizeof(buffer));
    file.close();
    timer.stop_timer("write");
}


void* Bitmap::partial_convolution(void* arg)
{
    FiltersThreadData* thread_data = static_cast<FiltersThreadData*>(arg);
    vector<vector<Pixel>>& image = *thread_data->image;
    int kernelSize = BLUR_KERNEL.size();
    int kernel_half_size = kernelSize / 2;
    for (int i = thread_data->start_row; i <= thread_data->end_row; i++) 
        for (int j = kernel_half_size; j < image[0].size() - kernel_half_size; j++) 
        {
            double sumRed = 0, sumGreen = 0, sumBlue = 0;
            for (int ki = 0; ki < kernelSize; ki++) 
                for (int kj = 0; kj < kernelSize; kj++) 
                {
                    sumRed += (image[i - kernel_half_size + ki][j - kernel_half_size + kj].red * BLUR_KERNEL[ki][kj]) / 16;
                    sumGreen += (image[i - kernel_half_size + ki][j - kernel_half_size + kj].green * BLUR_KERNEL[ki][kj]) / 16;
                    sumBlue += (image[i - kernel_half_size + ki][j - kernel_half_size + kj].blue * BLUR_KERNEL[ki][kj]) / 16;
                }
            image[i][j].red = min(max(sumRed, 0.0), 255.0);
            image[i][j].green = min(max(sumGreen, 0.0), 255.0);
            image[i][j].blue = min(max(sumBlue, 0.0), 255.0);
        }
    pthread_exit(nullptr);
}

void* Bitmap::partial_purple_filter(void* arg)
{
    FiltersThreadData* thread_data = static_cast<FiltersThreadData*>(arg);
    vector<vector<Pixel>>& temp_image = *thread_data->image;
    for (int i = thread_data->start_row; i <= thread_data->end_row; i++) 
        for (int j = 0; j < temp_image[i].size(); j++) 
        {
            Pixel temp = {temp_image[i][j].red, temp_image[i][j].green, temp_image[i][j].blue};
            temp_image[i][j].red = min(255, static_cast<unsigned char>(0.5 * (temp.red + temp.blue)) + static_cast<unsigned char>(0.3 * temp.green));
            temp_image[i][j].green = min(255, static_cast<unsigned char>(0.16 * (temp.red + temp.blue)) + static_cast<unsigned char>(0.5 * temp.green));
            temp_image[i][j].blue = min(255, static_cast<unsigned char>(0.6 * temp.red) + static_cast<unsigned char>(0.2 * temp.green) + static_cast<unsigned char>(0.8 * temp.blue));
        }
    pthread_exit(nullptr);
}

void Bitmap::purple_haze_filter() 
{
    timer.start_timer();
    vector<pthread_t> threads(NUM_OF_THREADS);
    vector<FiltersThreadData> thread_data(NUM_OF_THREADS);
    int row_per_thread = image.size() / NUM_OF_THREADS;
    for (int i = 0; i < NUM_OF_THREADS; i++) 
    {
        int start_row = i * row_per_thread;
        int end_row = (i == NUM_OF_THREADS - 1) ? image.size() - 1 : start_row + row_per_thread - 1;
        thread_data[i] = {&image, start_row, end_row};
        pthread_create(&threads[i], nullptr, partial_purple_filter, &thread_data[i]);
    }
    for (int i = 0; i < NUM_OF_THREADS; i++) 
        pthread_join(threads[i], nullptr);
    timer.stop_timer("Purple");
}

void* Bitmap::partial_vertical_flip(void* arg) 
{
    FiltersThreadData* data = static_cast<FiltersThreadData*>(arg);
    vector<vector<Pixel>>& temp_image = *data->image;
    int image_height = temp_image.size();
    for (int i = data->start_row; i < data->end_row; i++) 
    {
        vector<Pixel> temp = temp_image[i];
        temp_image[i] = temp_image[image_height - i - 1];
        temp_image[image_height - i - 1] = temp;
    }
    pthread_exit(nullptr);
}

void Bitmap::flip_vertically()
{
    timer.start_timer();
    int thread_count = NUM_OF_THREADS / 4;
    int rows_per_thread = ceil(image.size() / (thread_count) / 2);
    vector<pthread_t> threads(thread_count);
    vector<FiltersThreadData> thread_data(thread_count);
    for (int i = 0; i < thread_count; i++) 
    {
        int start_row = i * rows_per_thread;
        int end_row = min((i + 1) * rows_per_thread, static_cast<int>(image.size()));
        thread_data[i] = {&image , start_row , end_row};
        pthread_create(&threads[i], nullptr, partial_vertical_flip, &thread_data[i]);
    }
    for (int i = 0; i < thread_count; i++)
        pthread_join(threads[i], nullptr);
    timer.stop_timer("Flip");
}

void Bitmap::blur() 
{
    timer.start_timer();
    int rows = image.size() , cols = image[0].size();
    int kernel_half_size = BLUR_KERNEL.size() / 2;
    vector<pthread_t> threads(NUM_OF_THREADS);
    vector<FiltersThreadData> thread_data(NUM_OF_THREADS);
    int row_per_thread = (rows - 2 * kernel_half_size) / NUM_OF_THREADS;
    for (int i = 0; i < NUM_OF_THREADS; i++) 
    {
        int start_row = kernel_half_size + i * row_per_thread;
        int end_row = (i == NUM_OF_THREADS - 1) ? rows - 1 - kernel_half_size : start_row + row_per_thread - 1;
        thread_data[i] = {&image, start_row, end_row };
        pthread_create(&threads[i], nullptr, partial_convolution, &thread_data[i]);
    }
    for (int i = 0; i < NUM_OF_THREADS; i++)
        pthread_join(threads[i], nullptr);
    timer.stop_timer("Blur");
}

void* Bitmap::partial_sketch(void* arg)
{
    FiltersThreadData* threadData = static_cast<FiltersThreadData*>(arg);
    std::vector<std::vector<Pixel>>& image = *(threadData->image);
    int mid_height = image.size() / 2 ;
    int mid_width = image[0].size() /2 ;
    for (int i = threadData->start_row; i < threadData->end_row; i++) 
    {
        image[i][i] = WHITE;
        if (i < mid_height) 
        {
            image[i + mid_width][i] = WHITE;
            image[i][i + mid_height] = WHITE;
        }
    }
    pthread_exit(nullptr);
}

void Bitmap::sketch()
{
    timer.start_timer();
    int height = image.size() , width = image[0].size();
    int threads_count = NUM_OF_THREADS / 4;
    pthread_t threads[threads_count];
    FiltersThreadData threadData[threads_count];
    const int rows_per_thread = ceil(static_cast<double>(height) / threads_count);
    for (int i = 0; i < threads_count; i++) 
    {
        threadData[i].image = &image;
        threadData[i].start_row = i * rows_per_thread;
        threadData[i].end_row = std::min((i + 1) * rows_per_thread, height);
        int result = pthread_create(&threads[i], nullptr, partial_sketch, &threadData[i]);
    }
    for (int i = 0; i < threads_count; i++) 
        int result = pthread_join(threads[i], nullptr);
    timer.stop_timer("Sketch");
}

bool Bitmap::run()
{
    flip_vertically();
    blur();
    purple_haze_filter();
    sketch();
    write_bitmap_file("output.bmp");
    timer.print_duration();
    return true;
}