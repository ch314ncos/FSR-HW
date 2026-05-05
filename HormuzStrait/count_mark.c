#include <stdlib.h>
#include "functions.h"

//  Помечаем танкеры зеленым в почти чб картинке
void bw_mark_tanker(unsigned char *bw_pic, unsigned char *res, int bw_size, unsigned char tanker, unsigned char* marked)
{ 
    for(int i=0; i<bw_size; i++) 
    {   
        if (bw_pic[i] == tanker) 
        {
            res[i*4] = marked[0];
            res[i*4+1] = marked[1];
            res[i*4+2] = marked[2];
            res[i*4+3] = marked[3];
        } else {
            res[i*4] = bw_pic[i];
            res[i*4+1] = bw_pic[i];
            res[i*4+2] = bw_pic[i];
            res[i*4+3] =255;
        }
    } 
    return; 
} 

// Помечаем зеленым на исходной картинке
void rgba_mark_tanker(unsigned char* tanker_mask, unsigned char* res, int bw_size, unsigned char tanker, unsigned char* marked) 
{
    for (int i=0; i< bw_size; i++) 
    {
        if (tanker_mask[i] == tanker) 
        {
            res[i*4]= marked[0];
            res[i*4+1]= marked[1];
            res[i*4+2]= marked[2];
            res[i*4+3]= marked[3];
        }
    }
}



// Ищем компоненты связности
int bfs_components(unsigned char* bw_pic, int width, int height, unsigned char* visited, int sx, int sy, 
        unsigned char white, unsigned char tanker_color) 
{
    // Максимлаьное количество точек в комп связности, которую будем считать корабликом
    // Если точек больше, то возвращаем 0, иначе 1
    int maxv = 10; 

    Point* queue = (Point*)malloc(width*height*sizeof(Point));
    if (!queue) 
        return 0;

    int head = 0;
    int tail = 0;

    queue[tail++] = (Point){sx, sy};
    visited[sy * width + sx] = 1;

    int dx[4] = {-1, 1, 0, 0};
    int dy[4] = {0, 0, -1, 1};

    while (head < tail) 
    {
        Point p = queue[head++];

        for (int i=0; i<4; i++) 
        {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
    
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) 
            {
                int idx = ny * width + nx;
    
                if (bw_pic[idx] == white && !visited[idx]) 
                {
                    visited[idx] = 1;
                    queue[tail++] = (Point){nx, ny};
                }
            }
        }
    }

    

    // перекраска валидной компоненты 
    if (tail <= maxv) {
        for (int i=0; i<tail; i++) 
        {
            Point p = queue[i];
            bw_pic[p.y*width + p.x] = tanker_color;
        }
        free(queue);
        return 1;
    }

    free(queue);
    return 0;
}