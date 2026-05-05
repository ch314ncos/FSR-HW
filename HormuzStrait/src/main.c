#include <stdio.h>
#include <stdlib.h> 
#include <strings.h>
#include <math.h> 
#include "lodepng.h" 
#include "functions.h"


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
    unsigned char* finish = (unsigned char*)malloc(size*sizeof(unsigned char)); 
    
    // Преобразование RGBA в grayscale
    rgba_to_grayscale(bw_pic, picture, bw_size);

    grayscale_to_rgba(bw_pic, finish, bw_size);
    write_png("output_pic/bw_pic_row.png", finish, width, height);

    // Осветление картинки
    gamma_correction(bw_pic, bw_size); 
    
    grayscale_to_rgba(bw_pic, finish, bw_size);
    write_png("output_pic/gamma.png", finish, width, height);

    
    // Контраст с методом Оцу
    contrast(bw_pic, bw_size);

    grayscale_to_rgba(bw_pic, finish, bw_size);
    write_png("output_pic/contrast.png", finish, width, height);

 
    // Рисуем прямые серые ограничивающие ненужные для нас участки. Точки нахожу руками через Paint
    unsigned char gray = 100;
    Bresenham(bw_pic, width, height, (Point){265, 0}, (Point){393, height-1}, gray);
    Bresenham(bw_pic, width, height, (Point){416, 0}, (Point){0, 650}, gray);
    Bresenham(bw_pic, width, height, (Point){540, 0}, (Point){419, height-1}, gray);
    Bresenham(bw_pic, width, height, (Point){658, 0}, (Point){1114, 646}, gray);

    grayscale_to_rgba(bw_pic, finish, bw_size);
    write_png("output_pic/bw_lines.png", finish, width, height);

    
    // Заливка с границами
    flood_fill_untill_barrier(bw_pic, width, height, 0, 0, gray, 0);
    flood_fill_untill_barrier(bw_pic, width, height, width-1, 0, gray, 0);
    flood_fill_untill_barrier(bw_pic, width, height, 20, height-1, gray, 0);
    flood_fill_untill_barrier(bw_pic, width, height, 318, 238, gray, 0);
    flood_fill_untill_barrier_or_water(bw_pic, width, height, 469, height-1, gray, 0);

    grayscale_to_rgba(bw_pic, finish, bw_size);
    write_png("output_pic/bw_lines_floodfilled.png", finish, width, height);


    // Считаем количество танкеров
    unsigned char* visited = (unsigned char*)calloc(width * height, sizeof(unsigned char));
    if (!visited) {
        printf("not enough memory\n");
        return -1;
    }

    unsigned char tanker_color = 200;
    int cnt_components = 0;
    for (int y = 0; y< height; y++) {
        for (int x=0; x<width; x++) {
            int idx = y*width + x;
            if (bw_pic[idx] == 255 && !visited[idx]) {
                int f = bfs_components(bw_pic, width, height, visited, x, y, 255, tanker_color);
                if (f) cnt_components++;
            }
        }
    }

    printf("%d\n", cnt_components);

    grayscale_to_rgba(bw_pic, finish, bw_size);
    write_png("output_pic/bw_tankers_found.png", finish, width, height);

    // На чб картинке отметим зеленым танкеры
    unsigned char green[4] = {94, 224, 0, 255};
    bw_mark_tanker(bw_pic, finish, bw_size, tanker_color, green);
    write_png("output_pic/bw_tankers_marked.png", finish, width, height);

    // На исходной разноцветной картинке помечаем найденные танкеры
    finish = picture;
    rgba_mark_tanker(bw_pic, finish, bw_size, tanker_color, green);
    write_png("output_pic/picture_out.png", finish, width, height);


    
    // не забыли почистить память!
    free(visited);
    free(bw_pic); 
    free(finish); 
    //free(picture); 
    
    return 0; 
}

