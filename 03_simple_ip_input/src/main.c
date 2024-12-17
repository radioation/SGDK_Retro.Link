#include <genesis.h>
#include "network.h"

int cursor_x, cursor_y;
u8 buttons, buttons_prev;

char server[16] = "192.168.001.002";

void input_server_address(){
}

int main()
{
    SYS_disableInts();                      // Disable interrupts
    VDP_setScreenWidth320();                // Set screen Width
    VDP_setScreenHeight224();               // Set screen Height
    VDP_setBackgroundColor(0);              // Set background black
    VDP_setTextPlane(BG_B);                 // Use PLANE B for text rendering
    VDP_setTextPalette(0);                  // Use palette 0 for text color
    SYS_enableInts();                       // Enable interrupts (allows retro.link callback routine to print data)

    PAL_fadeOutPalette(PAL0,1,FALSE);
    VDP_setBackgroundColor(51);             // Blue background
    PAL_fadeInPalette(PAL0, palette_grey, 16, FALSE);

    cursor_x = 8;
    cursor_y = 2;
    VDP_drawText("Detecting adapter...[  ]", cursor_x, cursor_y); cursor_x+=21; 
    NET_initialize(); // Detect cartridge and set boolean variable

    if(cart_present)
    {
        VDP_setTextPalette(2); // Green text
        VDP_drawText("Ok", cursor_x, cursor_y); cursor_x=0; cursor_y+=3;
        VDP_setTextPalette(0); // White text
    }
    else
    {
        VDP_setTextPalette(1); // Red text
        VDP_drawText("XX", cursor_x, cursor_y); cursor_x=0; cursor_y+=3;
        VDP_setTextPalette(0); // White text
        VDP_drawText("Adapter not present", cursor_x, cursor_y);
        while(1) { SYS_doVBlankProcess(); }
    }


    cursor_x = 7;
    VDP_drawText("IP Address:", cursor_x, cursor_y); 
    NET_printIP(cursor_x+12, cursor_y); cursor_y+=2;

    cursor_x = 9;
    VDP_drawText("MAC:", cursor_x, cursor_y); 
    NET_printMAC(cursor_x+5, cursor_y); cursor_y+=2;


    waitMs(2000);

    NET_resetAdapter();


    //------------------------------------------------------------------
    // MAIN LOOP
    //------------------------------------------------------------------
    while(1) // Loop forever 
    { 
        SYS_doVBlankProcess(); 
    }

    //------------------------------------------------------------------
    return(0);
}


