#include <stdio.h>
#include <stdlib.h> 
#include <strings.h>
#include <math.h> 
#include "lodepng.h" 


// принимаем на вход: имя файла, указатели на int для хранения прочитанной ширины и высоты картинки
// возвращаем указатель на выделенную память для хранения картинки
// Если память выделить не смогли, отдаем нулевой указатель и пишем сообщение об ошибке
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height) 
{
  unsigned char* image = NULL; 
  int error = lodepng_decode32_file(&image, width, height, filename);
  
  if(error != 0) {
    printf("error %u: %s\n", error, lodepng_error_text(error)); 
  }
  return (image);
}

// принимаем на вход: имя файла для записи, указатель на массив пикселей,  ширину и высоту картинки
// Если преобразовать массив в картинку или сохранить не смогли,  пишем сообщение об ошибке
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  unsigned char* png;
  size_t pngsize;
  int error = lodepng_encode32(&png, &pngsize, image, width, height);
  if(error == 0) {
      lodepng_save_file(png, pngsize, filename);
  } else { 
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  free(png);
}


// // вариант огрубления серого цвета в ЧБ 
// void contrast(unsigned char *col, int bw_size)
// { 
//     // гамма коррекция 
//     float gamma = 1/2.2; // осветление < 1
//     for(int i=0; i < bw_size; i++)
//     {
//        float vout = pow(col[i] / 255.0, gamma); // т.к. A=1, то нужно перевести vin=col[i] в [0;1] см. википедиа 
//        col[i] = (unsigned char) (vout*255.0);
//     } 
//     return; 
// } 

void gamma_correction(unsigned char *col, int bw_size)
{ 
    float gamma = 1/2.2; // осветление < 1
    for(int i=0; i < bw_size; i++)
    {
       float vout = pow(col[i] / 255.0, gamma); // т.к. A=1, то нужно перевести vin=col[i] в [0;1] см. википедиа 
       col[i] = (unsigned char) (vout*255.0);
    } 
    return; 
} 

// Гауссово размыттие
void Gauss_blur(unsigned char *col, unsigned char *blr_pic, int width, int height)
{ 
    int i, j; 
    for(i=1; i < height-1; i++) 
        for(j=1; j < width-1; j++)
        { 
            blr_pic[width*i+j] = 0.084*col[width*i+j] + 0.084*col[width*(i+1)+j] + 0.084*col[width*(i-1)+j]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.084*col[width*i+(j+1)] + 0.084*col[width*i+(j-1)]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i+1)+(j+1)] + 0.063*col[width*(i+1)+(j-1)]; 
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i-1)+(j+1)] + 0.063*col[width*(i-1)+(j-1)]; 
        } 
    return; 
} 

unsigned char OtsuThershold(unsigned char *col, int bw_size) {
    // гистограмма по интенсивности изображения 0-255
    unsigned char hist[256]; // 0-255 = 256
    // инициализация нулями
    memset(hist, 0, 256*sizeof(*hist)); 

    // вычисление гистограммы
    for (int i=0; i<bw_size; i++)
        hist[col[i]]++; 

    // считаем сумму всех интенсивностей
    int all_intensity_sum = 0;
    for (int i=0; i<bw_size; i++)
        all_intensity_sum+=col[i];

    int all_pixel_count = bw_size;
    int best_thresh = 0;
    double best_sigma = 0.0;

    int first_class_pixel_count = 0;
    int first_class_intensity_sum = 0;

    // перебираем границы между классами
    // thresh < 255, так как иначе (all_pixel_count - first_pixel_count)=0 в знаменателе
    for (int thresh=0; thresh < 255; thresh++) {
        first_class_pixel_count +=hist[thresh];
        first_class_intensity_sum += thresh*hist[thresh];

        // w1 и w2 из вики
        double first_class_prob = first_class_pixel_count / (double) all_pixel_count;
        double second_class_prob = 1.0 - first_class_prob;

        // u1 и u2 из вики
        double first_class_mean = first_class_intensity_sum / (double) first_class_pixel_count;
        double second_class_mean = (all_intensity_sum - first_class_intensity_sum) 
            / (double) (all_pixel_count - first_class_pixel_count);

        double mean_delta = first_class_mean - second_class_mean;

        double sigma = first_class_prob * second_class_prob * mean_delta * mean_delta;

        if (sigma > best_sigma) {
            best_sigma = sigma;
            best_thresh = thresh;
        }
    }

    return (unsigned char)best_thresh;
}

void contrast (unsigned char *col, int bw_size) {
    unsigned char thresh = OtsuThershold(col, bw_size);

    for (int i=0; i<bw_size; i++) {
        if (col[i]<thresh) 
            col[i] = 0;
        else 
            col[i]=255;
    }
    return;
}

//  Место для экспериментов // разукрашивание готовой картнки
void color(unsigned char *blr_pic, unsigned char *res, int size)
{ 
  int i;
    for(i=1;i<size;i++) 
    { 
        res[i*4]=40+blr_pic[i]+0.35*blr_pic[i-1]; 
        res[i*4+1]=65+blr_pic[i]; 
        res[i*4+2]=170+blr_pic[i]; 
        res[i*4+3]=255; 
    } 
    return; 
} 

// Преобразуем grayscale в RGBA для записи PNG
void fill_finish(unsigned char* bw_pic, unsigned char* finish, int bw_size) 
{
    for (int i = 0; i < bw_size; i++) 
    {
        unsigned char gray = bw_pic[i];
        finish[i*4 + 0] = gray;  
        finish[i*4 + 1] = gray;  
        finish[i*4 + 2] = gray;  
        finish[i*4 + 3] = 255;   
    }
    return;
}

void rgba_to_grayscale(unsigned char* picture, unsigned char* bw_pic, int bw_size) {
    for (int i=0; i < bw_size; i++) 
    {
        unsigned char r = picture[i*4];
        unsigned char g = picture[i*4+1];
        unsigned char b = picture[i*4+2];
        unsigned char a = picture[i*4+3];

        bw_pic[i] = (0.299*r + 0.587*g + 0.114*b)*(a/255.0);
    }
    return;
}
  
int main() 
{ 
    const char* filename = "input_pic/straitRGB.png"; 
    unsigned int width, height;
    int size;
    int bw_size;
    
    // Чтение картинки
    unsigned char* picture = load_png("input_pic/straitRGB.png", &width, &height); 
    if (picture == NULL)
    { 
        printf("Problem reading picture from the file %s. Error.\n", filename); 
        return -1; 
    } 

    size = width * height * 4;
    bw_size = width * height;
    
    
    unsigned char* bw_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    // unsigned char* blr_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char)); 
    unsigned char* finish = (unsigned char*)malloc(size*sizeof(unsigned char)); 
    
    // Преобразование RGBA в grayscale
    rgba_to_grayscale(picture, bw_pic, bw_size);

    fill_finish(bw_pic, finish, bw_size);
    write_png("output_pic/bw_pic_row.png", finish, width, height);

    // Осветление картинки
    gamma_correction(bw_pic, bw_size); 
    
    fill_finish(bw_pic, finish, bw_size);
    write_png("output_pic/gamma.png", finish, width, height);

    // Размытие по Гауссу
    // Gauss_blur(bw_pic, blr_pic, width, height); 

    // fill_finish(blr_pic, finish, bw_size);
    // write_png("output_pic/gauss_2.png", finish, width, height);
    
    // Контраст с методом Оцу
    contrast(bw_pic, bw_size);

    fill_finish(bw_pic, finish, bw_size);
    write_png("output_pic/contrast.png", finish, width, height);


    // write_png("intermediate_result.png", finish, width, height);
    // color(blr_pic, finish, bw_size); 
    
    // выписали результат
    // write_png("picture_out.png", finish, width, height); 
    
    // не забыли почистить память!
    free(bw_pic); 
    // free(blr_pic); 
    free(finish); 
    free(picture); 
    
    return 0; 
}

