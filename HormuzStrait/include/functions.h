typedef struct {
    int x, y;
} Point;

// png_interaction
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height);
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height);

// pic_preprocessing
void gamma_correction(unsigned char *bw_pic, int bw_size);
unsigned char otsu_threshold(unsigned char *bw_pic, int bw_size);
void contrast (unsigned char *bw_pic, int bw_size);
void flood_fill_untill_barrier(unsigned char* bw_pic, int width, int height, int sx, int sy,
                             unsigned char barrier_color,
                             unsigned char new_color);
void flood_fill_untill_barrier_or_water(unsigned char* bw_pic, int width, int height, int sx, int sy,
                             unsigned char barrier_color,
                             unsigned char new_color);
void Bresenham(unsigned char* bw_pic, int width, int height, 
               Point A, Point B, unsigned char color);

// rgba_grayscale
void grayscale_to_rgba(unsigned char* bw_pic, unsigned char* pic, int bw_size);
void rgba_to_grayscale(unsigned char* bw_pic, unsigned char* pic, int bw_size);

// count_mark
void bw_mark_tanker(unsigned char *bw_pic, unsigned char *res, int bw_size, unsigned char tanker, unsigned char* marked);
void rgba_mark_tanker(unsigned char* tanker_mask, unsigned char* res, int bw_size, unsigned char tanker, unsigned char* marked);
int bfs_components(unsigned char* bw_pic, int width, int height, unsigned char* visited, int sx, int sy, 
        unsigned char white, unsigned char tanker_color);

