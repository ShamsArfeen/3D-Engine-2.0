#include <SDL.h>
#include <SDL_image.h>

#include <math.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <thread>

const auto processor_count = std::thread::hardware_concurrency();

const int eye_size = 1500;
const int center_x = 350;
const int center_y = 350;

const int flash_x = 5;
const int flash_y = 5;
const int flash_z = -10;

const float flash = sqrt(flash_x * flash_x + flash_y * flash_y + flash_z * flash_z);

const int max_vertex = 1000000;
const int max_line = 1000000;
const int max_polygon = 1000000;

struct polygon { int n, pt[3]; };
struct point_2d { float x; int y; };
struct point_3d { float x, y, z; };
struct line_3d { int p1, p2; };

extern int eye_z;
extern float sin_p, sin_q, cos_p, cos_q;
extern struct point_3d i_point[max_vertex];
extern struct point_3d r_point[max_vertex];
extern struct point_2d screen_pt[max_vertex];
extern int point_num;
extern float screen_z[700][700];
extern int screen_p[700][700];
extern int line_num;
extern struct line_3d i_line[max_line];
extern int polygon_num, hPgnCount, poly_space;
extern struct polygon i_polygon[max_polygon];
//extern float pgn_a, pgn_b, pgn_c, pgn_d;
extern float y_rotation, x_rotation;

float polygon_pixel(int x, int y, float z, float pgn_a, float pgn_b, float pgn_c, float pgn_d);
//void get_pgn_constants(int);
bool render_polygon(int);//, SDL_Renderer *);
void initiate();
void rotate_mat(float, float);
void freeze_mat(float, float);
void render_surfaces(SDL_Renderer *);
void get_surface(int, int);
void load_pgn_model();
//void set_pixel(int x, int y);
//bool polish_surface();
void clear_mat();