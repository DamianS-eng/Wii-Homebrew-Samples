// Based off of Original Program PowerCheck
// Copyright (C) 2012-2014	JoostinOnline

#include <gccore.h>
#include <grrlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <fat.h>

#include "font_ttf.h"
#include "background_jpg.h"

#define VERSION			"1.8"
#define FONTSIZE_TITLE	28
#define FONTSIZE_SUB	20
#define FONTSIZE_REMOTE	25
#define INDENT_REMOTE	90
#define INDENT_TITLE	15	
//#define SCREENSHOT
//#define DEMO_MODE


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
	

extern void __exception_setreload(int t);

s8 HWButton = -1;
int doreload=0, dooff=0;

static const int row[] = {10, 47, 40, 80, 110, 173, 236, 299, 362, 440};
GRRLIB_ttfFont *myFont;
GRRLIB_texImg *bkg;
GRRLIB_texImg *ScreenBuf;

//Initialize variable holding Wii Remote acceleration data
vec3w_t w_accel;
//Initalize variables holding Balance Board data
wii_board_t wiibal;
expansion_t exp_bal;
float bal_x,bal_y,weightx;

//Assorted rows for GRRLIB
enum {
	TITLE,
	SUBTEXT,
	ERROR,
	EXIT,
	REMOTE1,
	REMOTE2,
	REMOTE3,
	REMOTE4,
	REMOTE5,
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

int ButtonsDown(void) {
	//get button and accelerometer status of first wiimote
	u16 w_buttonsDown = WPAD_ButtonsDown(0);
	WPAD_Accel(0, &w_accel);
	//get sensor status of balance board
	u16 buttonsDown_Balance = WPAD_ButtonsDown(4);
	//wiimote home button or any button on balance board exits app by returning non-zero
	if (w_buttonsDown == WPAD_BUTTON_HOME) return(1);
	if (buttonsDown_Balance) return(2);
	return(0);
	//return WPAD_ButtonsDown(0) + WPAD_ButtonsDown(1) + WPAD_ButtonsDown(2) + WPAD_ButtonsDown(3) + WPAD_ButtonsDown(4);
}

void checkBalance(void) {
	u32 devtype;
	WPAD_Probe(4,&devtype);
	if (devtype==WPAD_EXP_WIIBOARD) {
		WPAD_Expansion(4, &exp_bal);
		bal_x=exp_bal.wb.x;
		bal_y=exp_bal.wb.y;
	}
}

void BatteryLevel(u8 battery, int x, int y) {
//original battery check function
	char buffer[20];
	if (battery) {
		if (battery > 100) battery = 100;

		sprintf(buffer, " %i%%", battery);
		GRRLIB_PrintfTTF(x, y, myFont, buffer, FONTSIZE_REMOTE, GREEN);
	} else {
		GRRLIB_PrintfTTF(x, y, myFont, " Not connected", FONTSIZE_REMOTE, RED);
	}
}

void accelerometer(vec3w_t accel, int x, int y) {
//modified battery check to output raw accel values instead
	char buffer[20];
	if (WPAD_BatteryLevel(0)) {
		//print raw values of roll, pitch and pitch inverse.
		sprintf(buffer, " %i %i %i", accel.x, accel.y, accel.z);
		GRRLIB_PrintfTTF(x, y, myFont, buffer, FONTSIZE_REMOTE, GREEN);
	} else {
		GRRLIB_PrintfTTF(x, y, myFont, " Not connected", FONTSIZE_REMOTE, RED);
	}
}

void balance(int x, int y) {
	char buffer[20];
	if (WPAD_BatteryLevel(4)) {
		//print calculated weight in kilograms
		weightx=exp_bal.wb.tl+exp_bal.wb.tr+exp_bal.wb.bl+exp_bal.wb.br;
		//sprintf(buffer, "%f ", weightx);
		sprintf(buffer, "%d %d %d %d", (int)exp_bal.wb.tl, (int)exp_bal.wb.tr, (int)exp_bal.wb.bl, (int)exp_bal.wb.br);
		GRRLIB_PrintfTTF(x, y, myFont, buffer, FONTSIZE_REMOTE, GREEN);
	} else {
		GRRLIB_PrintfTTF(x, y, myFont, " Not connected", FONTSIZE_REMOTE, RED);
	}
}

void LoadBackground(void) {
	fatInitDefault();
	bkg = GRRLIB_LoadTextureFromFile("background.png");
	if (bkg == NULL) bkg = GRRLIB_LoadTextureJPG(background_jpg); // Default background image
	return;
}

void Deinit(void) {
//---------------------------------------------------------------------------------
	GRRLIB_FreeTTF(myFont);
	GRRLIB_FreeTexture(bkg);
	GRRLIB_FreeTexture(ScreenBuf);
	GRRLIB_Exit();
}

void Init() {
//---------------------------------------------------------------------------------
	GRRLIB_Init();
	myFont = GRRLIB_LoadTTF(font_ttf, font_ttf_size); // Truetype font
	if (WPAD_Init() == WPAD_ERR_NONEREGISTERED) {
		GRRLIB_PrintfTTF(INDENT_TITLE, row[ERROR], myFont, "Error: No remotes have been registered", FONTSIZE_TITLE, RED);
		GRRLIB_PrintfTTF(INDENT_TITLE, row[EXIT], myFont, "Exiting...", FONTSIZE_TITLE, RED);
		GRRLIB_Render();
		sleep(4);
		Deinit();
		exit(0);
	}
	#ifndef DEMO
	WPAD_SetIdleTimeout(60 * 2); // Remote will turn off after 2 minutes of inactivity.
	#endif

	SYS_SetPowerCallback(WiiPowerPressed);
	SYS_SetResetCallback(WiiResetPressed);
	
	LoadBackground();
	
	ScreenBuf = GRRLIB_CreateEmptyTexture(rmode->fbWidth, rmode->efbHeight);
	//Adjust Dataformat from Wiimotes
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	bal_x=0;bal_y=0;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	__exception_setreload(0); // In the event of a code dump, app will exit immediately
	Init();

	while(HWButton == -1) {
		WPAD_ScanPads();
		if (ButtonsDown() != 0){
			break;
		}
		checkBalance();
		// Write constants to screen
		GRRLIB_DrawImg(0, 0, bkg, 0, 1, 1, RGBA(255, 255, 255, 255));
		GRRLIB_PrintfTTF(INDENT_TITLE, row[TITLE], myFont, "PowerCheck v" VERSION " by JoostinOnline", FONTSIZE_TITLE, WHITE);
		GRRLIB_PrintfTTF(INDENT_TITLE, row[SUBTEXT], myFont, "www.HacksDen.com", FONTSIZE_SUB, WHITE);
	
		#ifdef DEMO_MODE
		BatteryLevel(100, INDENT_REMOTE, row[REMOTE1]);
		BatteryLevel(52, INDENT_REMOTE, row[REMOTE2]);
		BatteryLevel(73, INDENT_REMOTE, row[REMOTE3]);
		BatteryLevel(0, INDENT_REMOTE, row[REMOTE4]);
		BatteryLevel(88, INDENT_REMOTE, row[REMOTE5]);
		#else
//		BatteryLevel(WPAD_BatteryLevel(0) / 1.5, INDENT_REMOTE, row[REMOTE1]);
		accelerometer(w_accel, INDENT_REMOTE, row[REMOTE1]);
		BatteryLevel(WPAD_BatteryLevel(1) / 1.5, INDENT_REMOTE, row[REMOTE2]);
		BatteryLevel(WPAD_BatteryLevel(2) / 1.5, INDENT_REMOTE, row[REMOTE3]);
		BatteryLevel(WPAD_BatteryLevel(3) / 1.5, INDENT_REMOTE, row[REMOTE4]);
//		BatteryLevel(WPAD_BatteryLevel(4) / 3.0, INDENT_REMOTE, row[REMOTE5]); // Balance Board reports twice the percentage
		balance(INDENT_REMOTE, row[REMOTE5]);
		#endif
		
		GRRLIB_PrintfTTF(INDENT_TITLE, row[PRESS_ANY_BUTTON], myFont, "Press the Home button to exit", FONTSIZE_TITLE, WHITE);

		// Work-around for GRRLIB bug
		GRRLIB_Screen2Texture(0, 0, ScreenBuf, GX_FALSE);
	
		GRRLIB_DrawImg(0, 0, ScreenBuf, 0, 1, 1, RGBA(255, 255, 255, 255));

		VIDEO_WaitVSync();
		GRRLIB_Render();
	}
	Deinit();
	if (HWButton != -1) SYS_ResetSystem(HWButton, 0, 0);
	return(0);
}

/*
		if(HWButton != -1)
		{
			#ifdef SCREENSHOT
			GRRLIB_ScrShot("Screenshot.png");  //Takes a screenshot if you press POWER or RESET
			#endif
			Deinit();
			SYS_ResetSystem(HWButton, 0, 0);
		}
*/