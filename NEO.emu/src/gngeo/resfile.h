#ifndef RESFILE_H
#define RESFILE_H

//#include "SDL.h"
#include "roms.h"

ROM_DEF *res_load_drv(const char *name);
//SDL_Surface *res_load_bmp(char *bmp);
void *res_load_data(const char *name);
//SDL_Surface *res_load_stbi(char *bmp);

#endif
