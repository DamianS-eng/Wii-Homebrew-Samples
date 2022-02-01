//#include <stdio.h>
//#include <string.h>
#include <gccore.h>
#include <grrlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fat.h>

//Use WinXP default background. Use WiiBuilder in devkitpro tools to generate header
#include "background_jpg.h"

#define VERSION			"1.0"
#define FONTSIZE_TITLE	28
#define FONTSIZE_SUB	20
#define FONTSIZE_REMOTE	25
#define INDENT_REMOTE	90
#define INDENT_TITLE	15	
#define SCREENSHOT_SIZE	20
//#define SCREENSHOT


// RGBA Colors
#define BLACK		0x000000FF
#define MAROON		0x800000FF
#define GREEN		0x00C108FF
#define OLIVE		0x808000FF
#define NAVY		0x000080FF
#define PURPLE		0x800080FF
#define TEAL		0x008080FF
#define GRAY		0x808080FF
#define SILVER		0xC0C0C0FF
#define RED			0xFF0000FF
#define LIME		0x00FF00FF
#define YELLOW		0xFFFF00FF
#define BLUE		0x0000FFFF
#define FUCHSIA		0xFF00FFFF
#define AQUA		0x00FFFFFF
#define WHITE		0xFFFFFFFF
//code dump	
extern void __exception_setreload(int t);

s8 HWButton = -1;
int filenum = 1;
char grrscre[SCREENSHOT_SIZE] = "sd:/grrlib_0.png";
//Spacing used for GRRLIB
//static const int row[] = {10, 47, 40, 80, 110, 173, 236, 299, 362, 440};
static const int colors[] = {WHITE, RED, AQUA, PURPLE};
int color_w1 = 0; 
int color_w2 = 2;
GRRLIB_texImg *bkg;
GRRLIB_texImg *ScreenBuf;

struct remote_data {
	WPADData *wd;
	vec3w_t w_accel;
	ir_t w_ir;
	u16 w_buttons;
	int w_color;
};
//use two wiimotes
struct remote_data wpads[2];
int P1MX, P1MY, P2MX, P2MY;

//Initialize variable holding Wii Remote acceleration data
u32 type;
//WPADData *wd_1, *wd_2;
//vec3w_t w_accel;

//Buttons
u16 buttons_w1, buttons_w2;


//Assorted rows for GRRLIB
enum {
	TITLE,
	SUBTEXT,
	ERROR,
	EXIT,
	PRESS_ANY_BUTTON
};

void WiiResetPressed()
{
	HWButton = SYS_RESTART;
}

void WiiPowerPressed()
{
	HWButton = SYS_POWEROFF;
}

void readController() {
	WPAD_ScanPads();
	if(WPAD_Probe(0, &type) == WPAD_ERR_NONE) {	
		wpads[0].wd = WPAD_Data(0);
		WPAD_IR(WPAD_CHAN_0, &wpads[0].w_ir);
		buttons_w1 = WPAD_ButtonsDown(0);
	// WiiMote IR Viewport correction of smoothed coordinates
		P1MX = wpads[0].w_ir.sx - 150;
		P1MY = wpads[0].w_ir.sy - 150;
	}
	else {
		P1MX = 100;
		P1MY = 0;
	}
	if(WPAD_Probe(1, &type) == WPAD_ERR_NONE) {	
		wpads[1].wd = WPAD_Data(1);
		WPAD_IR(WPAD_CHAN_1, &wpads[1].w_ir);
		buttons_w2 = WPAD_ButtonsDown(1);
		P2MX = wpads[1].w_ir.sx - 150;
		P2MY = wpads[1].w_ir.sy - 150;	
	}
	else {
		P2MX = 500;
		P2MY = 0;
	}
}
/*
void DrawHLine (int x1, int x2, int y, int color) {
    int i;
    y = 320 * y;
    x1 >>= 1;
    x2 >>= 1;
    for (i = x1; i <= x2; i++) {
//	Draw & Color
    }
}

void DrawVLine (int x, int y1, int y2, int color) {
    int i;
    x >>= 1;
    for (i = y1; i <= y2; i++) {
//		Draw & Color
    }
}
*/
void drawBox(int x1, int y1, int x2, int y2, int color){
	GRRLIB_Line(x1, y1, x2, y1, color);
	GRRLIB_Line(x1, y2, x2, y2, color);
	GRRLIB_Line(x1, y1, x1, y2, color);
	GRRLIB_Line(x2, y1, x2, y2, color);
}

void LoadBackground(void) {
//	fatInitDefault();
	bkg = GRRLIB_LoadTexture("background_jpg");
	return;
}

void SwitchBackground(void) {
	return;
}

void Deinit(void) {
//---------------------------------------------------------------------------------
//	GRRLIB_FreeTTF(myFont);
	GRRLIB_FreeTexture(bkg);
	GRRLIB_FreeTexture(ScreenBuf);
	GRRLIB_Exit();
}

void Init() {
//---------------------------------------------------------------------------------
	GRRLIB_Init();
	if (WPAD_Init() == WPAD_ERR_NONEREGISTERED) {
//		If the app suddenly quits on start, add two wiimotes first
//		GRRLIB_PrintfTTF(INDENT_TITLE, row[ERROR], myFont, "Wiimotes missing", FONTSIZE_TITLE, RED);
//		GRRLIB_PrintfTTF(INDENT_TITLE, row[EXIT], myFont, "Exiting...", FONTSIZE_TITLE, RED);
		GRRLIB_Render();
		sleep(4);
		Deinit();
		exit(0);
	}
	SYS_SetPowerCallback(WiiPowerPressed);
	SYS_SetResetCallback(WiiResetPressed);
	LoadBackground();
	ScreenBuf = GRRLIB_CreateEmptyTexture(rmode->fbWidth, rmode->efbHeight);
	WPAD_SetIdleTimeout(60 * 2); // Remote will turn off after 2 minutes of inactivity.
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(0, 640, 480);
	WPAD_SetVRes(1, 640, 480);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	__exception_setreload(0); // In the event of a code dump, app will exit immediately
	Init();

	while(HWButton == -1) {
		readController();
		//Quit app if first wiimote presses Home
		if (buttons_w1 & WPAD_BUTTON_HOME){
			break;
		}
		//Switch pointer color pressing Plus
		if (buttons_w1 & WPAD_BUTTON_PLUS){
			switch(color_w1){
				case 0:
				color_w1=1;
				break;
				default:
					color_w1=0;
			}
		}
		if (buttons_w2 & WPAD_BUTTON_PLUS){
			switch(color_w2){
				case 2:
				color_w2=3;
				break;
				default:
					color_w2=2;
			}
		}
        if(buttons_w1 & WPAD_BUTTON_1 || buttons_w2 & WPAD_BUTTON_1) {
            WPAD_Rumble(WPAD_CHAN_ALL, 1); // Rumble on;
			grrscre[12] = (char) filenum;
            GRRLIB_ScrShot(grrscre);
			//GRRLIB_ScrShot("sd:/grrlib.png");
			if (filenum > 8) {filenum = 0;}
			else {filenum++;}
            WPAD_Rumble(WPAD_CHAN_ALL, 0); // Rumble off
        }
		// Work-around for GRRLIB bug
//		GRRLIB_Screen2Texture(0, 0, ScreenBuf, GX_FALSE);
//		GRRLIB_DrawImg(0, 0, ScreenBuf, 0, 1, 1, RGBA(0, 0, 255, 255));
		// Draw boxes based on IR location
		GRRLIB_DrawImg(0, 0, bkg, 0, 1, 1, RGBA(255, 255, 255, 255));
		drawBox(P1MX, P1MY, P1MX + 30, P1MY + 30, colors[color_w1]);
		drawBox(P2MX, P2MY, P2MX + 30, P2MY + 30, colors[color_w2]);
		VIDEO_WaitVSync();
		GRRLIB_Render();
	}
	Deinit();
	if (HWButton != -1) SYS_ResetSystem(HWButton, 0, 0);
	return(0);
}