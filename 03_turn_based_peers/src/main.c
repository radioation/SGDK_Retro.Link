#include <genesis.h>
#include "resources.h"

#define INPUT_WAIT_COUNT 10

typedef struct 
{
    Sprite *sprite;
    s16 pos_x;
    s16 pos_y;
    s16 step_x;
    s16 step_y;
} CP_SPRITE;

CP_SPRITE cursor;


int main(bool hard) {

    //////////////////////////////////////////////////////////////
    // setup screen and palettes
    VDP_setBackgroundColor(16);
    VDP_setScreenWidth320();

    PAL_setPalette( PAL0, board_pal.data, CPU );
    PAL_setPalette( PAL1, pieces_pal.data, CPU );

    //////////////////////////////////////////////////////////////
    // setup background
    int ind = TILE_USER_INDEX;
    VDP_drawImageEx(BG_B, &board_img, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += board_img.tileset->numTile;
    //VDP_drawImageEx(BG_A, &fg, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    //ind += fg.tileset->numTile;

    // foreground


    //////////////////////////////////////////////////////////////
    // sprites 
    SPR_init();
    cursor.pos_x = 64;
    cursor.pos_y = 16;
    cursor.step_x = 24;
    cursor.step_y = 24;
    cursor.sprite = SPR_addSprite( &cursor_spr, cursor.pos_x, cursor.pos_y, TILE_ATTR(PAL0, 0, FALSE, FALSE ));

    //////////////////////////////////////////////////////////////
    // main loop.
    u8 inputWait = 0;
    while(TRUE)
    {
        // read joypad to mover cursor
        u16 joypad  = JOY_readJoypad( JOY_1 );
        if( inputWait == 0 ) {
            if( joypad & BUTTON_LEFT ) {
                cursor.pos_x -= cursor.step_x;
                inputWait = INPUT_WAIT_COUNT;
            } else if( joypad & BUTTON_RIGHT ) {
                cursor.pos_x += cursor.step_x;
                inputWait = INPUT_WAIT_COUNT;
            } else if( joypad & BUTTON_UP ) {
                cursor.pos_y -= cursor.step_y;
                inputWait = INPUT_WAIT_COUNT;
            } else if( joypad & BUTTON_DOWN ) {
                cursor.pos_y += cursor.step_y;
                inputWait = INPUT_WAIT_COUNT;
            }
        } else {
            if( inputWait > 0 ) {
                --inputWait;
            }
        }



        //////////////////////////////////////////////////////////////
        // update sprites
        SPR_setPosition( cursor.sprite, cursor.pos_x, cursor.pos_y );
        SPR_update();


        //////////////////////////////////////////////////////////////
        // SGDK Do your thing.
        SYS_doVBlankProcess();

    }
    return 0;

}
