#include "functions.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

void gamma_correction(unsigned char *bw_pic, int bw_size)
{ 
    float gamma = 1/2.2; // осветление < 1
    for(int i=0; i < bw_size; i++)
    {
       float vout = pow(bw_pic[i] / 255.0, gamma); // т.к. A=1, то нужно перевести vin=pic[i] в [0;1] см. википедиа 
       bw_pic[i] = (unsigned char) (vout*255.0);
    } 
    return; 
} 


unsigned char otsu_threshold(unsigned char *bw_pic, int bw_size) 
{
    // гистограмма по интенсивности изображения 0-255
    unsigned char hist[256]; // 0-255 = 256
    // инициализация нулями
    memset(hist, 0, 256*sizeof(*hist)); 

    // вычисление гистограммы
    for (int i=0; i<bw_size; i++)
        hist[bw_pic[i]]++; 

    // считаем сумму всех интенсивностей
    int all_intensity_sum = 0;
    for (int i=0; i<bw_size; i++)
        all_intensity_sum+=bw_pic[i];

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

void contrast (unsigned char *bw_pic, int bw_size) 
{
    unsigned char thresh = otsu_threshold(bw_pic, bw_size);

    for (int i=0; i<bw_size; i++) 
    {
        if (bw_pic[i]<thresh) 
            bw_pic[i] = 0;
        else 
            bw_pic[i]=255;
    }
    return;
}

// Заливка 

void flood_fill_untill_barrier(unsigned char* bw_pic, int width, int height, int sx, int sy,
                             unsigned char barrier_color,
                             unsigned char new_color) 
{
    // Проверка начальной точки
    if (bw_pic[sy*width + sx] == barrier_color) return;

    // Создаем карту посещений, чтобы не зацикливаться на черном океане
    unsigned char* visited = (unsigned char*)calloc(width*height, sizeof(unsigned char));
    if (!visited) return;

    // Выделяем память под очередь для BFS
    Point* queue = (Point*)malloc(width*height*sizeof(Point));
    if (!queue) 
    {
        free(visited);
        return;
    }

    int head = 0;
    int tail = 0;

    // Добавляем стартовую точку
    queue[tail++] = (Point){sx, sy};
    visited[sy*width + sx] = 1;
    bw_pic[sy*width + sx] = new_color;

    // Смещения для 4-связной заливки (вверх, вниз, влево, вправо)
    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    while (head < tail) 
    {
        Point p = queue[head++];

        for (int i = 0; i < 4; i++) 
        {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];

            // Проверка границ
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) 
            {
                int idx = ny*width + nx;

                // Идем, если это не барьер и мы здесь еще не были
                // Цвет пикселя (кроме барьерного) теперь не важен для движения
                if (bw_pic[idx] != barrier_color && !visited[idx]) 
                {
                    visited[idx] = 1;     // Помечаем как пройденное
                    bw_pic[idx] = new_color; // Красим в черный
                    queue[tail++] = (Point){nx, ny};
                }
            }
        }
    }

    free(queue);
    free(visited);
}

void flood_fill_untill_barrier_or_water(unsigned char* bw_pic, int width, int height, int sx, int sy,
                             unsigned char barrier_color,
                             unsigned char new_color)       
{
    unsigned char start = bw_pic[sy*width + sx];
    if (start == barrier_color)
        return;

    Point* queue = (Point*)malloc(width*height*sizeof(Point)); 
    int head = 0, tail = 0;
    queue[tail++] = (Point){sx, sy};
    bw_pic[sy*width + sx] = new_color;

    int dx[4] = {1, -1, 0, 0}; 
    int dy[4] = {0, 0, 1, -1};

    while (head < tail) {
        Point p = queue[head++];
        for (int i = 0; i < 4; i++) 
        {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) 
            {
                unsigned char val = bw_pic[ny*width + nx];
                
                if (val != barrier_color && val != new_color) 
                {
                    bw_pic[ny*width + nx] = new_color;
                    queue[tail++] = (Point){nx, ny};
                }
            }
        }
    }
    free(queue);
}



void Bresenham(unsigned char* bw_pic, int width, int height, 
               Point A, Point B, unsigned char color) 
{
    int x = A.x;
    int y = A.y;
    int dx = abs(B.x - A.x);
    int dy = abs(B.y - A.y);
    int sx = (A.x < B.x) ? 1 : -1;
    int sy = (A.y < B.y) ? 1 : -1;
    int err = dx - dy;
    int e2;

    while (1) {
        // Рисуем пиксель, если в пределах изображения
        if (x >= 0 && x < width && y >= 0 && y < height) 
            bw_pic[y * width + x] = color;
        if (x == B.x && y == B.y) 
            break;
        e2 = 2 * err;
        if (e2 > -dy) 
        {   
            err -= dy; 
            x += sx; 
        }
        if (e2 < dx)  
        { 
            err += dx; 
            y += sy; 
        }
    }
}