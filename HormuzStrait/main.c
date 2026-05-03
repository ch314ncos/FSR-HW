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


unsigned char otsu_threshold(unsigned char *bw_pic, int bw_size) {
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

void contrast (unsigned char *bw_pic, int bw_size) {
    unsigned char thresh = otsu_threshold(bw_pic, bw_size);

    for (int i=0; i<bw_size; i++) {
        if (bw_pic[i]<thresh) 
            bw_pic[i] = 0;
        else 
            bw_pic[i]=255;
    }
    return;
}


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


//  Помечаем танкеры зеленым в почти чб картинке
void bw_mark_tanker(unsigned char *bw_pic, unsigned char *res, int bw_size, unsigned char tanker, unsigned char* marked)
{ 
    for(int i=0; i<bw_size; i++) 
    {   
        if (bw_pic[i] == tanker) {
            res[i*4] = marked[0];
            res[i*4+1] = marked[1];
            res[i*4+2] = marked[2];
            res[i*4+3] = marked[3];
        }
        else {
            res[i*4] = bw_pic[i];
            res[i*4+1] = bw_pic[i];
            res[i*4+2] = bw_pic[i];
            res[i*4+3] =255;
        }
    } 
    return; 
} 

// Помечаем зеленым на исходной картинке
void rgba_mark_tanker(unsigned char* tanker_mask, unsigned char* res, int bw_size, unsigned char tanker, unsigned char* marked) {
    for (int i=0; i< bw_size; i++) {
        if (tanker_mask[i] == tanker) {
            res[i*4]= marked[0];
            res[i*4+1]= marked[1];
            res[i*4+2]= marked[2];
            res[i*4+3]= marked[3];
        }
    }
}

// Заливка 
typedef struct {
    int x, y;
} Point;


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
    if (!queue) {
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

    while (head < tail) {
        Point p = queue[head++];

        for (int i = 0; i < 4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];

            // Проверка границ
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int idx = ny*width + nx;

                // Идем, если это НЕ барьер и мы здесь еще НЕ были
                // Цвет пикселя (кроме барьерного) теперь не важен для движения
                if (bw_pic[idx] != barrier_color && !visited[idx]) {
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
        for (int i = 0; i < 4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                unsigned char val = bw_pic[ny*width + nx];
                
                if (val != barrier_color && val != new_color) {
                    bw_pic[ny*width + nx] = new_color;
                    queue[tail++] = (Point){nx, ny};
                }
            }
        }
    }
    free(queue);
}



void Bresenham(unsigned char* bw_pic, int width, int height, 
               Point A, Point B, unsigned char color) {
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
        if (x >= 0 && x < width && y >= 0 && y < height) {
            bw_pic[y * width + x] = color;
        }
        if (x == B.x && y == B.y) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx)  { err += dx; y += sy; }
    }
}

// Ищем компоненты связности
int bfs_components(unsigned char* bw_pic, int width, int height, unsigned char* visited, int sx, int sy, 
        unsigned char white, unsigned char tanker_color) {
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

    while (head < tail) {
        Point p = queue[head++];

        for (int i=0; i<4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
    
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int idx = ny * width + nx;
    
                if (bw_pic[idx] == white && !visited[idx]) {
                    visited[idx] = 1;
                    queue[tail++] = (Point){nx, ny};
                }
            }
        }
    }

    

    // перекраска валидной компоненты 
    if (tail <= maxv) {
        for (int i=0; i<tail; i++){
            Point p = queue[i];
            bw_pic[p.y*width + p.x] = tanker_color;
        }
        free(queue);
        return 1;
    }

    free(queue);
    return 0;
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

    // Нарисуем на исходной разноцветной картинке найденный корабли
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

