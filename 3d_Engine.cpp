#include "3D_Header.hpp"

using namespace std;

void initiate()
{
    cout << "[*] Initiate MAT" << endl;
}

void clear_mat()
{
    cout << "[*] Empty MAT" << endl;
    point_num = 0;
    polygon_num = 0;
    x_rotation = 0;
    y_rotation = 0;
    eye_z = 0;
    for (int i = 0; i < 700; i++) {
        for (int j = 0; j < 700; j++) {
            screen_z[i][j] = -100000;
        }
    }
}

void* RotateThread(void * pID) {
    int iCore = (int)pID;
    for (int i = iCore; i < point_num; i += processor_count)
    {
        float ptx, pty, ptz, pti, temp;
        
        ptx = i_point[i].x;
        pty = i_point[i].y;
        ptz = i_point[i].z;

        pti = ptz * cos_p - ptx * sin_p;
        ptx = ptx * cos_p + ptz * sin_p;

        ptz = pti * cos_q + pty * sin_q;
        pty = pty * cos_q - pti * sin_q;

        r_point[i].x = ptx;
        r_point[i].y = pty;
        r_point[i].z = ptz;

        temp = (float)eye_size / (float)(ptz + eye_z);
        screen_pt[i].x = (float)(center_x + temp * ptx);
        screen_pt[i].y = (int)(center_y - temp * pty);
    }
    return pID;
}

void rotate_mat(float x, float y)
{
    pthread_t pID;
    float ini_x, ini_y;
    
    ini_x = x_rotation + x;
    ini_y = y_rotation + y;

    sin_p = sin(ini_x), cos_p = cos(ini_x);
    sin_q = sin(ini_y), cos_q = cos(ini_y);

    for (int i = 0; i < processor_count; i++){
        pthread_create(&pID, NULL, RotateThread, (void *)i);
    }
    pthread_join(pID, NULL);
}

void freeze_mat(float x, float y)
{
    x_rotation += x;
    y_rotation += y;
}

float polygon_pixel(int x, int y, float z, float pgn_a, float pgn_b, float pgn_c, float pgn_d)
{
    float line = pgn_a * (x - center_x) + pgn_b * (center_y - y);
    float denominator = line + pgn_c * eye_size;
    float numinator = eye_z * line - eye_size * pgn_d;
    if (denominator == 0) {
        return -100000;
    }
    else {
        line = numinator / denominator;
        if (line > z) return z;
        return line;
    }
}

void* RenderThread(void * pID) {
    int iCore = (int)pID;
    for (int i = iCore; i < polygon_num; i += processor_count)
    {
        render_polygon(i);
    }
    return pID;
}

void render_surfaces(SDL_Renderer *gRenderer) {
    pthread_t pID;

    for (int i = 0; i < processor_count; i++){
        pthread_create(&pID, NULL, RenderThread, (void *)i);
    }
    pthread_join(pID, NULL);

    for (int x = 0; x < 700; x++)
    {
        for (int y = 0; y < 700; y++)
        {
            int texture = screen_p[x][y];
            screen_p[x][y] = 0;
            screen_z[x][y] = -100000;
            SDL_SetRenderDrawColor(gRenderer, texture, texture, texture, 0);
            SDL_RenderDrawPoint(gRenderer, x, y);
        }
    }
    
    // update screen
    SDL_RenderPresent(gRenderer);
}

bool render_polygon(int pgn)//, SDL_Renderer *g_Renderer)
{
    //get_pgn_constants(pgn);
    float pt1x, pt1y, pt1z;
    float pt2x, pt2y, pt2z;
    float pt3x, pt3y, pt3z;

    pt1x = r_point[i_polygon[pgn].pt[0]].x;
    pt1y = r_point[i_polygon[pgn].pt[0]].y;
    pt1z = r_point[i_polygon[pgn].pt[0]].z;
    pt2x = r_point[i_polygon[pgn].pt[1]].x;
    pt2y = r_point[i_polygon[pgn].pt[1]].y;
    pt2z = r_point[i_polygon[pgn].pt[1]].z;
    pt3x = r_point[i_polygon[pgn].pt[2]].x;
    pt3y = r_point[i_polygon[pgn].pt[2]].y;
    pt3z = r_point[i_polygon[pgn].pt[2]].z;

    float z = pt3z;
    if (pt2z < z) z = pt2z;
    if (pt1z < z) z = pt1z;

    float pgn_c = pt1x * pt3y - pt3x * pt1y - pt2x * pt3y + pt3x * pt2y + pt2x * pt1y - pt1x * pt2y;
    float pgn_b = pt3x * pt1z - pt1x * pt3z + pt2x * pt3z - pt3x * pt2z - pt2x * pt1z + pt1x * pt2z;
    float pgn_a = pt3z * pt1y - pt1z * pt3y + pt2z * pt3y - pt3z * pt2y - pt2z * pt1y + pt1z * pt2y;
    float pgn_d = pt1x * pt2z * pt3y - pt2z * pt3x * pt1y - pt1z * pt2x * pt3y + pt2y * pt1z * pt3x + pt2x * pt1y * pt3z - pt1x * pt2y * pt3z;

    //float flash = sqrt(flash_x * flash_x + flash_y * flash_y + flash_z * flash_z);
    float triangle = sqrt(pgn_a * pgn_a + pgn_b * pgn_b + pgn_c * pgn_c);
    float reflection = abs(flash_x * pgn_a + flash_y * pgn_b + flash_z * pgn_c);
    float shine = abs(asin(reflection / (triangle * flash)));

    int texture = 7 * pow(16000 * shine + 1000, 0.3);
    //SDL_SetRenderDrawColor(g_Renderer, texture, texture, texture, 255);

    int n = i_polygon[pgn].n;
    int y, pt;//, min_y, max_y, st;
    float xi[3], yi[3];

    pt = i_polygon[pgn].pt[0];
    xi[0] = screen_pt[pt].x;
    y = screen_pt[pt].y;
    //min_y = y;
    //max_y = y;
    yi[0] = y + 0.5;
    
    pt = i_polygon[pgn].pt[1];
    xi[1] = screen_pt[pt].x;
    y = screen_pt[pt].y;
    //if (min_y > y) min_y = y;
    //else if (max_y < y) max_y = y;
    yi[1] = y + 0.5;
    
    pt = i_polygon[pgn].pt[2];
    xi[2] = screen_pt[pt].x;
    y = screen_pt[pt].y;
    //if (min_y > y) min_y = y;
    //else if (max_y < y) max_y = y;
    yi[2] = y + 0.5;

    for (int t = 3; t > 1; t--)
    {
        bool flag = false;
        for (int m = 1; m < t; m++)
        {
            if (yi[m-1] > yi[m])
            {
                flag = true;
                
                float tempf = yi[m];
                yi[m] = yi[m-1];
                yi[m-1] = tempf;

                int tempi = xi[m];
                xi[m] = xi[m-1];
                xi[m-1] = tempi;
            }
        }
        if (flag == false)
            goto JUMP;
    }
JUMP:

    //cout << "DEBUG 1" << endl;

    y = yi[2] - yi[0] + 1;
    int TRI[2][y], TRISPACE[y], j;
    float slope;
    for (int i = 0; i < y; i++) TRISPACE[i] = 0;

    //cout << "DEBUG 2" << endl;

    if (yi[0] != yi[1]) {
        j=0; slope = (xi[0] - xi[1]) / (yi[0] - yi[1]);
        for (int i = yi[0] + 1; i < yi[1]; i++) {
            TRI[TRISPACE[j]][j] = (i - yi[1]) * slope + xi[1];
            TRISPACE[j]++; j++;
        }
    }

    //cout << "DEBUG 3" << endl;

    if (yi[0] != yi[2]) {
        j=0; slope = (xi[0] - xi[2]) / (yi[0] - yi[2]);
        for (int i = yi[0] + 1; i < yi[2]; i++) {
            TRI[TRISPACE[j]][j] = (i - yi[2]) * slope + xi[2];
            TRISPACE[j]++; j++;
        }
    }

    //cout << "DEBUG 4" << endl;

    if (yi[1] != yi[2]) {
        j=yi[1] - yi[0]; slope = (xi[1] - xi[2]) / (yi[1] - yi[2]);
        for (int i = yi[1] + 1; i < yi[2]; i++) {
            TRI[TRISPACE[j]][j] = (i - yi[2]) * slope + xi[2];
            TRISPACE[j]++; j++;
        }
    }

    //cout << "DEBUG 5" << endl;

    //for (int i = 0; i < 3; i++)
    //    cout << "x " << xi[i] << "  y " << yi[i] << endl;
    //for (int i = 0; i < y; i++)
    //    cout << TRI[0][i] << " ---- " << TRI[1][i] << "  :~ " << TRISPACE[i] << endl;

    y = yi[0];
    for (int i = 0; i + yi[0] < yi[2]; i++) {
        int x1, x2;
        if (TRISPACE[i] != 2) goto NEXT_i;
        
        x1 = TRI[0][i]; x2 = TRI[1][i];
        if (x2 < x1) { int temp = x2; x2 = x1; x1 = temp; }
        //cout << "x1=" << x1 << " x2=" << x2 << " TRISPACE[i]=" << TRISPACE[i] << endl;
        if (x1 > 799 || x1 < 0 || x2 > 799 || x2 < 0)
            cout << "BAD TRIANGLE DAY" << endl;

        for (int x = x1; x <= x2; x++) {
            float i_screen_z = polygon_pixel(x, y, -z, pgn_a, pgn_b, pgn_c, pgn_d);
            if (screen_z[x][y] == -100000 || screen_z[x][y] < i_screen_z)
            {
                screen_z[x][y] = i_screen_z;
                screen_p[x][y] = texture;
                //screen_p[1][screen_ptr] = y;
                //screen_p[2][screen_ptr] = texture;
                //screen_ptr++;
                //SDL_SetRenderDrawColor(g_Renderer, texture, texture, texture, 255);
                //SDL_RenderDrawPoint(g_Renderer, x, y);
            }
        }
    NEXT_i:
        y++;
    }
    //cout << "DEBUG 6" << endl;
    return true;
}

void get_surface(int x, int y)
{
    cout << " ==== GETTING SURFACE < " << screen_p[x][y] << " > ==== " << endl;
}