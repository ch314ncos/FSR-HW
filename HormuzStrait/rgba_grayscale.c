#include "functions.h"

// Преобразуем grayscale в RGBA для записи PNG
void grayscale_to_rgba(unsigned char* bw_pic, unsigned char* pic, int bw_size) 
{
    for (int i = 0; i < bw_size; i++) 
    {
        unsigned char gray = bw_pic[i];
        pic[i*4 + 0] = gray;  
        pic[i*4 + 1] = gray;  
        pic[i*4 + 2] = gray;  
        pic[i*4 + 3] = 255;   
    }
    return;
}

void rgba_to_grayscale(unsigned char* bw_pic, unsigned char* pic, int bw_size) {
    for (int i=0; i < bw_size; i++) 
    {
        unsigned char r = pic[i*4];
        unsigned char g = pic[i*4+1];
        unsigned char b = pic[i*4+2];
        unsigned char a = pic[i*4+3];

        bw_pic[i] = (0.299*r + 0.587*g + 0.114*b)*(a/255.0);
    }
    return;
}