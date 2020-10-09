#include "3D_Header.hpp"

int eye_z;
float sin_p, sin_q, cos_p, cos_q;
struct point_3d i_point[max_vertex];
struct point_3d r_point[max_vertex];
struct point_2d screen_pt[max_vertex];
int point_num;
float screen_z[700][700];
int screen_p[700][700];
int line_num;
struct line_3d i_line[max_line];
int polygon_num, hPgnCount, poly_space;
struct polygon i_polygon[max_polygon];
//float pgn_a, pgn_b, pgn_c, pgn_d;
float y_rotation, x_rotation;