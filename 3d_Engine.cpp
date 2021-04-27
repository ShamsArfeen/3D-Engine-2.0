#include "3D_Header.hpp"
#include <stdint.h>
#include <omp.h>


pthread_mutex_t lock;

using namespace std;

struct argRender {
    int pID;
    SDL_Renderer *gRenderer;
};

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
    int iCore = (int)(intptr_t)pID;
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
        pthread_create(&pID, NULL, RotateThread, (void *)(intptr_t )i);
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
    float ptz = pgn_a * (x - center_x) + pgn_b * (center_y - y);
    float denominator = ptz + pgn_c * eye_size;
    float numinator = eye_z * ptz - eye_size * pgn_d;

    if (denominator == 0) {
        return -100000;
    }
    else {

        ptz = numinator / denominator;
        if (ptz > z) return z;

        return ptz;
    }
}

void* RenderThread(void * arg) {
    int iCore = ((struct argRender*)arg)->pID;
    SDL_Renderer *gRenderer = ((struct argRender*)arg)->gRenderer;
    for (int i = iCore; i < polygon_num; i += processor_count)
    {
        render_polygon(i, gRenderer);
    }
    return 0;
}

void* WriteThread(void * pID) {
    int iCore = (int)(intptr_t)pID;
    for (int i = iCore; i < 700; i += processor_count) 
    {
        for (int j = 0; j < 700; j++)
        {
            screen_z[i][j] = -100000;
        }
    }
    return pID;
}

/*void render_surfaces_omp(SDL_Renderer *gRenderer) {
    
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    SDL_RenderClear(gRenderer);
    
    #pragma omp parallel for
    for (int i = 0; i < polygon_num; i++) {
        render_polygon(i, gRenderer);
    }

    // update screen
    SDL_RenderPresent(gRenderer);

    #pragma omp parallel for
    for (int i = 0; i < polygon_num; i++) {
        render_polygon(i, gRenderer);
    }
}*/

void render_surfaces(SDL_Renderer *gRenderer) {

    pthread_t pID[processor_count];
    struct argRender* iArg = new struct argRender[processor_count];

    for (int i = 0; i < processor_count; i++) {
        iArg[i].pID = i;
        iArg[i].gRenderer = gRenderer;
        pthread_create(&pID[i], NULL, RenderThread, (void *)&iArg[i]);
    }

    for (int i = 0; i < processor_count; i++) {
        pthread_join(pID[i], NULL);
    }
    delete iArg;

    // update screen
    SDL_RenderPresent(gRenderer);

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    SDL_RenderClear(gRenderer);

    for (int i = 0; i < processor_count; i++) {
        pthread_create(&pID[i], NULL, WriteThread, (void *)(intptr_t )i);
    }    
    
    for (int i = 0; i < processor_count; i++) {
        pthread_join(pID[i], NULL);
    }
    
}

bool render_polygon(int pgn, SDL_Renderer *g_Renderer)// int iCore)
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

    float triangle = sqrt(pgn_a * pgn_a + pgn_b * pgn_b + pgn_c * pgn_c);
    float reflection = abs(flash_x * pgn_a + flash_y * pgn_b + flash_z * pgn_c);
    float shine = abs(asin(reflection / (triangle * flash)));

    int texture = pow(16000 * shine , 0.5);
    

    int y, pt;
    float xi[3], yi[3];

    pt = i_polygon[pgn].pt[0];
    xi[0] = screen_pt[pt].x;
    y = screen_pt[pt].y;
    
    yi[0] = y + 0.5;
    
    pt = i_polygon[pgn].pt[1];
    xi[1] = screen_pt[pt].x;
    y = screen_pt[pt].y;
    
    yi[1] = y + 0.5;
    
    pt = i_polygon[pgn].pt[2];
    xi[2] = screen_pt[pt].x;
    y = screen_pt[pt].y;

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

    y = yi[2] - yi[0] + 1;
    int TRI[2][y], TRISPACE[y], j;
    float slope;
    for (int i = 0; i < y; i++) TRISPACE[i] = 0;

    if (yi[0] != yi[1]) {
        j=0; slope = (xi[0] - xi[1]) / (yi[0] - yi[1]);
        for (int i = yi[0] + 1; i < yi[1]; i++) {
            TRI[TRISPACE[j]][j] = (i - yi[1]) * slope + xi[1];
            TRISPACE[j]++; j++;
        }
    }

    if (yi[0] != yi[2]) {
        j=0; slope = (xi[0] - xi[2]) / (yi[0] - yi[2]);
        for (int i = yi[0] + 1; i < yi[2]; i++) {
            TRI[TRISPACE[j]][j] = (i - yi[2]) * slope + xi[2];
            TRISPACE[j]++; j++;
        }
    }

    if (yi[1] != yi[2]) {
        j=yi[1] - yi[0]; slope = (xi[1] - xi[2]) / (yi[1] - yi[2]);
        for (int i = yi[1] + 1; i < yi[2]; i++) {
            TRI[TRISPACE[j]][j] = (i - yi[2]) * slope + xi[2];
            TRISPACE[j]++; j++;
        }
    }

    y = yi[0];
    float temp1 = (float)eye_size / (float)(z + eye_z);
    

    for (int i = 0; i + yi[0] < yi[2]; i++) {
        int x1, x2;
        if (TRISPACE[i] != 2) goto NEXT_i;
        
        x1 = TRI[0][i]; x2 = TRI[1][i];
        if (x2 < x1) { int temp = x2; x2 = x1; x1 = temp; }

        for (int x = x1; x <= x2; x++) {
            float i_screen_z = polygon_pixel(x, y, -z, pgn_a, pgn_b, pgn_c, pgn_d);
            
            if (screen_z[x][y] == -100000 || screen_z[x][y] < i_screen_z)
            {
                screen_z[x][y] = i_screen_z;

                float ptz = i_screen_z;

                float ptx = (x - center_x) / temp1;
                float pty = (center_y - y) / temp1;
                float dis_sqrd = pow((ptx / eye_z - flash_x), 2) 
                            + pow((pty / eye_z - flash_y), 2) 
                            + pow((ptz / eye_z - flash_z), 2);

                int color = 6 * texture / dis_sqrd;
                
                pthread_mutex_lock(&lock);
                SDL_SetRenderDrawColor(g_Renderer, color, color, color, 255);
                SDL_RenderDrawPoint(g_Renderer, x, y);
                pthread_mutex_unlock(&lock);
            }
        }
    NEXT_i:
        y++;
    }
    
    return true;
}
