#include <grrlib.h>
#include <stdlib.h>
#include <math.h>
#include <wiiuse/wpad.h>

// Include Graphics
#include "RGFX_Background_png.h"
#include "RGFX_Background_jpg.h"
#include "RGFX_Crosshair_png.h"
#include "RGFX_Font_png.h"


// Declare static functions
static void ExitGame();
// Prepare Graphics
GRRLIB_texImg *GFX_Background;
GRRLIB_texImg *GFX_Crosshair;
GRRLIB_texImg *GFX_Font;

void switchBackground(int page) {
	switch(page){
		case 1: 
			GFX_Background = GRRLIB_LoadTexturePNG(RGFX_Background_png);
			break;
		default:
			GFX_Background = GRRLIB_LoadTextureJPG(RGFX_Background_jpg);
	}
}

int main() {
    ir_t P1Mote;
	int backpage = 0;
    // Init GRRLIB & WiiUse
    GRRLIB_Init();
    u16 WinW = rmode->fbWidth;
    u16 WinH = rmode->efbHeight;
    WPAD_Init();
    WPAD_SetIdleTimeout( 60 * 10 );
    WPAD_SetDataFormat( WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR );

    // Load textures
    //GFX_Background = GRRLIB_LoadTexturePNG(RGFX_Background_png);
	switchBackground(backpage);
    GFX_Crosshair  = GRRLIB_LoadTexturePNG(RGFX_Crosshair_png);
	GFX_Font       = GRRLIB_LoadTexturePNG(RGFX_Font_png);
    GRRLIB_InitTileSet( GFX_Font, 8, 16, 32 );

//    // Set handles
    GRRLIB_SetMidHandle( GFX_Crosshair, true );

    while (true) {
        WPAD_ScanPads();
        u32 WPADKeyDown = WPAD_ButtonsDown(WPAD_CHAN_0);
        WPAD_SetVRes(WPAD_CHAN_0, WinW, WinH);
        WPAD_IR(WPAD_CHAN_0, &P1Mote);


        // WiiMote IR Viewport correction
        int P1MX = P1Mote.sx - 150;
        int P1MY = P1Mote.sy - 150;

        // Drawing Background
        GRRLIB_DrawImg( 0, 0, GFX_Background, 0, 1, 1, RGBA(255, 255, 255, 255) );

        // Draw Crosshair
        GRRLIB_DrawImg( P1MX, P1MY, GFX_Crosshair, 0, 1, 1, RGBA(255, 255, 255, 255) );

        // Draw Text
        GRRLIB_Rectangle( 28, 28, 280, 20, RGBA(0, 0, 0, 160), 1 );
        GRRLIB_Printf   ( 32, 32, GFX_Font, 0xFFFFFFFF, 1, "Point your WiiMote on the screen." );

        // Renders the Scene
        GRRLIB_Render();

        if (WPADKeyDown & WPAD_BUTTON_B) {   
			if (backpage == 1) {
				backpage = 2;
			}
			else {
				backpage = 1;
			}
			switchBackground(backpage);
       }
        if (WPADKeyDown & WPAD_BUTTON_HOME) {
            break;
        }
    }
    ExitGame();
    return 0;
}

static void ExitGame() {

    // Deinitialize GRRLIB & Video
    GRRLIB_Exit();

    // Exit application
    exit(0);
}
