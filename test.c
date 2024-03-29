/*
 *  tslib/src/ts_test.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_test.c,v 1.6 2004/10/19 22:01:27 dlowder Exp $
 *
 * Basic test program for touchscreen library.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "tslib.h"
#include "fbutils.h"

static int palette [] =
{
	0x000000, 0xffe080, 0xffffff, 0xe0c0a0, 0x304050, 0x80b8c0
};
#define NR_COLORS (sizeof (palette) / sizeof (palette [0]))

struct ts_button {
	int x, y, w, h;
	char *text;
	int flags;
#define BUTTON_ACTIVE 0x00000001
};

/* [inactive] border fill text [active] border fill text */
static int button_palette [6] =
{
	1, 4, 2,
	1, 5, 0
};

#define NR_BUTTONS 17
static struct ts_button buttons [NR_BUTTONS];

static void sig(int sig)
{
	close_framebuffer();
	fflush(stderr);
	printf("signal %d caught\n", sig);
	fflush(stdout);
	exit(1);
}

static void button_draw (struct ts_button *button)
{
	int s = (button->flags & BUTTON_ACTIVE) ? 3 : 0;
	rect (button->x, button->y, button->x + button->w - 1,
	      button->y + button->h - 1, button_palette [s]);
	fillrect (button->x + 1, button->y + 1,
		  button->x + button->w - 2,
		  button->y + button->h - 2, button_palette [s + 1]);
	put_string_center (button->x + button->w / 2,
			   button->y + button->h / 2,
			   button->text, button_palette [s + 2]);
}

static int button_handle (struct ts_button *button, struct ts_sample *samp)
{
	int inside = (samp->x >= button->x) && (samp->y >= button->y) &&
		(samp->x < button->x + button->w) &&
		(samp->y < button->y + button->h);

	if (samp->pressure > 0) {
		if (inside) {
			if (!(button->flags & BUTTON_ACTIVE)) {
				button->flags |= BUTTON_ACTIVE;
				button_draw (button);
			}
		} else if (button->flags & BUTTON_ACTIVE) {
			button->flags &= ~BUTTON_ACTIVE;
			button_draw (button);
		}
	} else if (button->flags & BUTTON_ACTIVE) {
		button->flags &= ~BUTTON_ACTIVE;
		button_draw (button);
                return 1;
	}

        return 0;
}

static void refresh_screen ()
{
	int i;

	fillrect (0, 0, xres - 1, yres - 1, 0);
	/*put_string_center (xres/2, yres/4,   "TSLIB test program", 1);
	put_string_center (xres/2, yres/4+20,"Touch screen to move crosshair", 2);*/

	for (i = 0; i < NR_BUTTONS; i++)
		button_draw (&buttons [i]);
}

int main()
{
	struct tsdev *ts;
	int x, y,add1=0,add2=0,flagc=0;
	unsigned int i;
	unsigned int mode = 0;
	char context[17][17]=["7","8","9","+","4","5","6","-","1","2","3","*","AC","0","=","/","0"]
	char *tsdevice=NULL;

	signal(SIGSEGV, sig);
	signal(SIGINT, sig);
	signal(SIGTERM, sig);

	if ((tsdevice = getenv("TSLIB_TSDEVICE")) == NULL) {
#ifdef USE_INPUT_API
		tsdevice = strdup ("/dev/input/event0");
#else
		tsdevice = strdup ("/dev/touchscreen/ucb1x00");
#endif /* USE_INPUT_API */
        }

	ts = ts_open (tsdevice, 0);

	if (!ts) {
		perror (tsdevice);
		exit(1);
	}

	if (ts_config(ts)) {
		perror("ts_config");
		exit(1);
	}

	if (open_framebuffer()) {
		close_framebuffer();
		exit(1);
	}

	x = xres/2;
	y = yres/2;

	for (i = 0; i < NR_COLORS; i++)
		setcolor (i, palette [i]);

	/* Initialize buttons */
	memset (&buttons, 0, sizeof (buttons));
	for (i=0;i<16;i++){	
		buttons [i].w = xres/8;
		buttons [i].h = yres/6;
		buttons [i].x = xres/4*(i%4)+20;
		buttons [i].y = yres/4*(i/4+1)+10;
		buttons [i].text = context[i];
	}
	buttons[16].w=xres-80;
	buttons[16].h=yres/6;
	buttons[16].x=20;
	buttons[16].y=10;
	buttons[16].text=context[16];
	refresh_screen ();

	while (1) {
		struct ts_sample samp;
		int ret;

		/* Show the cross 
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);

		ret = ts_read(ts, &samp, 1);

		Hide it 
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);

		if (ret < 0) {
			perror("ts_read");
			close_framebuffer();
			exit(1);
		}*/

		if (ret != 1)
			continue;

		for (i = 0; i < NR_BUTTONS; i++)
			if (button_handle (&buttons [i], &samp))
				switch (i) {
				case 0:
					if (flagc==0){
						add1=add1*10+7;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+7;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen();
					break;
				case 1:
					if (flagc==0){
						add1=add1*10+8;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+8;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 2:
					if (flagc==0){
						add1=add1*10+9;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+9;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 3:
					if (flagc==0){
						flagc=1;
					}
					else if(flagc==1){
						add1=add1+add2;		
						sprintf(buttons[16].text,"%d\n",add1);		
						flagc=1;
					}
					else if(flagc==2){
						add1=add1-add2;		
						sprintf(buttons[16].text,"%d\n",add1);	
						flagc=1;
					}
					else if(flagc==3){
						add1=add1*add2;		
						sprintf(buttons[16].text,"%d\n",add1);	
						flagc=1;
					}
					else if(flagc==4){
						add1=add1/add2;		
						sprintf(buttons[16].text,"%d\n",add1);
						flagc=1;
					}
					refresh_screen ();
					break;
				case 4:
					if (flagc==0){
						add1=add1*10+4;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+4;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 5:
					if (flagc==0){
						add1=add1*10+5;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+5;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 6:
					if (flagc==0){
						add1=add1*10+6;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+6;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 7:
					if (flagc==0){
						flagc=2;
					}
					else if(flagc==1){
						add1=add1+add2;		
						sprintf(buttons[16].text,"%d\n",add1);	
						flagc=2;
					}
					else if(flagc==2){
						add1=add1-add2;		
						sprintf(buttons[16].text,"%d\n",add1);		
						flagc=2;
					}
					else if(flagc==3){
						add1=add1*add2;		
						sprintf(buttons[16].text,"%d\n",add1);			
						flagc=2;
					}
					else if(flagc==4){
						add1=add1/add2;		
						sprintf(buttons[16].text,"%d\n",add1);	
						flagc=2;
					}
					refresh_screen ();
					break;
				case 8:
					if (flagc==0){
						add1=add1*10+1;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+1;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 9:
					if (flagc==0){
						add1=add1*10+2;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+2;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 10:
					if (flagc==0){
						add1=add1*10+3;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10+3;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 11:
					if (flagc==0){
						flagc=3;
					}
					else if(flagc==1){
						add1=add1+add2;		
						sprintf(buttons[16].text,"%d\n",add1);		
						flagc=3;
					}
					else if(flagc==2){
						add1=add1-add2;		
						sprintf(buttons[16].text,"%d\n",add1);		
						flagc=3;
					}
					else if(flagc==3){
						add1=add1*add2;		
						sprintf(buttons[16].text,"%d\n",add1);	
						flagc=3;
					}
					else if(flagc==4){
						add1=add1/add2;		
						sprintf(buttons[16].text,"%d\n",add1);	
						flagc=3;
					}
					refresh_screen ();
					break;
				case 12:
					flagc=0;
					buttons[16]=context[16];
					refresh_screen();
					break;
				case 13:
					if (flagc==0){
						add1=add1*10;
						sprintf(buttons[16].text,"%d\n",add1);
					}
					else{
						add2=add2*10;		
						sprintf(buttons[16].text,"%d\n",add2);			
					}
					refresh_screen ();
					break;
				case 14:
					if (flagc==0){
					}
					else if(flagc==1){
						add1=add1+add2;		
						sprintf(buttons[16].text,"%d\n",add1);		
						flagc=0;
					}
					else if(flagc==2){
						add1=add1-add2;		
						sprintf(buttons[16].text,"%d\n",add1);			
						flagc=0;
					}
					else if(flagc==3){
						add1=add1*add2;		
						sprintf(buttons[16].text,"%d\n",add1);			
						flagc=0;
					}
					else if(flagc==4){
						add1=add1/add2;		
						sprintf(buttons[16].text,"%d\n",add1);		
						flagc=0;
					}
					refresh_screen ();
					break;
				case 15:
					if (flagc==0){
						flagc=4;
					}
					else if(flagc==1){
						add1=add1+add2;		
						sprintf(buttons[16].text,"%d\n",add1);	
						flagc=4;
					}
					else if(flagc==2){
						add1=add1-add2;		
						sprintf(buttons[16].text,"%d\n",add1);			
						flagc=4;
					}
					else if(flagc==3){
						add1=add1*add2;		
						sprintf(buttons[16].text,"%d\n",add1);
						flagc=4;
					}
					else if(flagc==4){
						add1=add1/add2;		
						sprintf(buttons[16].text,"%d\n",add1);
						flagc=4;
					}
					refresh_screen ();
					break;
				}

		/*printf("%ld.%06ld: %6d %6d %6d\n", samp.tv.tv_sec, samp.tv.tv_usec,
			samp.x, samp.y, samp.pressure);

		if (samp.pressure > 0) {
			if (mode == 0x80000001)
				line (x, y, samp.x, samp.y, 2);
			x = samp.x;
			y = samp.y;
			mode |= 0x80000000;
		} else
			mode &= ~0x80000000;*/
	}
	close_framebuffer();
}


