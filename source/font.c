/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#include <stdio.h>
#include <stdarg.h>
#include <vita2d.h>
#include "font.h"

extern const unsigned char msx_font[];

void font_draw_char(int x, int y, unsigned int color, char c)
{
	unsigned char *font = (unsigned char *)(msx_font + (c - (unsigned int)' ') * 8);
	int i, j, pos_x, pos_y;
	for (i = 0; i < 8; ++i) {
		pos_y = y + i*2;
		for (j = 0; j < 8; ++j) {
			pos_x = x + j*2;
			if ((*font & (128 >> j))) {
				vita2d_draw_rectangle(pos_x, pos_y, 2, 2, color);
			}
		}
		++font;
	}
}

void font_draw_string(int x, int y, unsigned int color, const char *string)
{
	if (string == NULL) return;

	int startx = x;
	const char *s = string;

	while (*s) {
		if (*s == '\n') {
			x = startx;
			y += 16;
		} else if (*s == ' ') {
			x += 16;
		} else if(*s == '\t') {
			x += 16*4;
		} else {
			font_draw_char(x, y, color, *s);
			x += 16;
		}
		++s;
	}
}

void font_draw_stringf(int x, int y, unsigned int color, const char *s, ...)
{
	char buf[256];
	va_list argptr;
	va_start(argptr, s);
	vsnprintf(buf, sizeof(buf), s, argptr);
	va_end(argptr);
	font_draw_string(x, y, color, buf);
}

// printf replacement, only for debug purposes
//int y_printf = 0;
void gpu_printf (const char *format, ...){
	/*__gnuc_va_list arg;
	int done;

	va_start (arg, format);
	char msg[512];
	done = vsprintf (msg, format, arg);
	va_end (arg);
	int i;
	for (i = 0; i<3;i++){
		vita2d_start_drawing();
		font_draw_string(0, y_printf, RGBA8( 0xFF, 0xFF, 0xFF, 0xFF), msg);
		vita2d_end_drawing();
		vita2d_swap_buffers();
		sceDisplayWaitVblankStart();
	}
	//sceKernelDelayThread(500000);
	y_printf += 20;
	if (y_printf > 520){
		y_printf = 0;
		for (i = 0; i<3;i++){
			vita2d_start_drawing();
			vita2d_clear_screen();
			vita2d_end_drawing();
			vita2d_swap_buffers();
			sceDisplayWaitVblankStart();
		}
	}*/
}