#include <genesis.h>
#include "resources.h"
#include "ip_input.h"

int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Check for negative sign
    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    // Iterate through the string, converting each digit to an integer
    while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
            i++;
        } else {
            // Stop if a non-digit character is encountered
            break;
        }
    }

    return result * sign;
}


s16 pos_x = 100;
s16 pos_y = 84;
s16 x_inc = 32;


void handleInput() {
    u16 joypad = JOY_readJoypad( JOY_1 );
}

//void updateAddressString( char* ipaddr, u8 p1, u8 p2, u8 p3, u8 p4 ) {
//
//}

bool getIPFromUser( char* ipaddr ) {

    //////////////////////////////////////////////////////////////
    // setup background
    PAL_setPalette( PAL0, background_pal.data, CPU );
    PAL_setPalette( PAL1, ip_cursor_pal.data, CPU );

    int ind = TILE_USER_INDEX;
    VDP_drawImageEx(BG_B, &background_img, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
  
    SPR_addSprite( &ip_cursor_spr, pos_x, pos_y, TILE_ATTR(PAL1, FALSE, FALSE, FALSE ));


    // TODO: parse numbers from string if non zero
    u8 intPart1 = 192;
    u8 intPart2 = 169;
    u8 intPart3 = 1;
    u8 intPart4 = 2;

    char temp_server[16];
    char textPart1[4];
    memset( textPart1, 0, sizeof(textPart1));
    char textPart2[4];
    memset( textPart2, 0, sizeof(textPart2));
    char textPart3[4];
    memset( textPart3, 0, sizeof(textPart3));
    char textPart4[4];
    memset( textPart4, 0, sizeof(textPart4));
    while(true) {
        //updateAddressString( temp_server, p1, p2, p3, p4 );
        uintToStr(intPart1, textPart1, 3 );             
        uintToStr(intPart2, textPart2, 3 );             
        uintToStr(intPart3, textPart3, 3 );             
        uintToStr(intPart4, textPart4, 3 );             
        VDP_drawText( textPart1, 13, 11 );
        VDP_drawText( textPart2, 17, 11 );
        VDP_drawText( textPart3, 21, 11 );
        VDP_drawText( textPart4, 25, 11 );

        SPR_update();    

        SYS_doVBlankProcess();

    }



}


