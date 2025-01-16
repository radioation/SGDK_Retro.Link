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


const s16 pos_y = 84;
const s16 base_x = 100;
s16 pos_x = 100;
s16 x_inc = 32;
u8 curr_octet = 0;
u8 ip_bytes [ 4 ] = {192,168,1,2};
Sprite * ip_cursor;
bool doSave = false;
bool done = false;

void inputCallback( u16 joy, u16 changed, u16 state ) {
    if(  changed & state & BUTTON_LEFT ) {
        if( curr_octet == 0 ) {
            curr_octet = 3;
        }else {
            curr_octet -= 1;
        }
        pos_x = base_x + x_inc * curr_octet;
    }else if(  changed & state & BUTTON_RIGHT) {
        curr_octet += 1;
        if( curr_octet > 3 ) {
            curr_octet = 0;
        }
        pos_x = base_x + x_inc * curr_octet;
    }

    if(  changed & state & BUTTON_UP ) {
        // increment 
        if( state & BUTTON_C ) {
            ip_bytes[ curr_octet ] += 10;
        } else {
            ip_bytes[ curr_octet ] += 1;
        }

    } else if(  changed & state & BUTTON_DOWN ) {
        if( state & BUTTON_C ) {
            ip_bytes[ curr_octet ] -= 10;
        } else {
            ip_bytes[ curr_octet ] -= 1;
        }
    }


    if(  changed & state & BUTTON_A ) {
       doSave = true; 
       done = true;
    }else if(  changed & state & BUTTON_B ) {
       done = true;
    }
}


bool getIPFromUser( char* ipaddr ) {

    //////////////////////////////////////////////////////////////
    // setup background
    PAL_setPalette( PAL0, background_pal.data, CPU );
    PAL_setPalette( PAL1, ip_cursor_pal.data, CPU );

    int ind = TILE_USER_INDEX;
    VDP_drawImageEx(BG_B, &background_img, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);

    ip_cursor = SPR_addSprite( &ip_cursor_spr, pos_x, pos_y, TILE_ATTR(PAL1, FALSE, FALSE, FALSE ));


    // TODO: parse numbers from string if non zero

    char temp_server[16];
    char textPart1[4];
    memset( textPart1, 0, sizeof(textPart1));
    char textPart2[4];
    memset( textPart2, 0, sizeof(textPart2));
    char textPart3[4];
    memset( textPart3, 0, sizeof(textPart3));
    char textPart4[4];
    memset( textPart4, 0, sizeof(textPart4));

    JOY_setEventHandler( &inputCallback );

    VDP_drawText( "Press A to save", 13, 16 );
    VDP_drawText( "Press B to cancel", 12, 18 );

    while(!done) {
        //updateAddressString( temp_server, p1, p2, p3, p4 );
        uintToStr(ip_bytes[0], textPart1, 3 );             
        uintToStr(ip_bytes[1], textPart2, 3 );             
        uintToStr(ip_bytes[2], textPart3, 3 );             
        uintToStr(ip_bytes[3], textPart4, 3 );             
        VDP_drawText( textPart1, 13, 11 );
        VDP_drawText( textPart2, 17, 11 );
        VDP_drawText( textPart3, 21, 11 );
        VDP_drawText( textPart4, 25, 11 );
        if( doSave == true ) {
            sprintf(ipaddr,"%s.%s.%s.%s", textPart1, textPart2, textPart3, textPart4 );
            return;
        }
        SPR_setPosition( ip_cursor, pos_x, pos_y );
        SPR_update();    

        SYS_doVBlankProcess();

    }



}


