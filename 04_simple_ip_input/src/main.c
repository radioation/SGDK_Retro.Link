#include <genesis.h>
#include "network.h"


#include "ip_input.h"

int cursor_x, cursor_y;
u8 buttons, buttons_prev;

char server[16] = "000.000.000.000"; // default 


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

/*
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
*/

    SRAM_enable();
    u8 part = SRAM_readByte(0);
    char textPart[4];
    //memset( textPart, 0, sizeof(textPart) ):
    sprintf( server, "%03d.", part ); 
    part = SRAM_readByte(1);
    sprintf( server+4, "%03d.", part ); 
    part = SRAM_readByte(2);
    sprintf( server+8, "%03d.", part ); 
    part = SRAM_readByte(3);
    sprintf( server+12, "%03d", part ); 

    
    SPR_init();

    VDP_drawText( server, 13 , 3 );

    getIPFromUser(server);

    VDP_drawText( "Got Address", 13 ,12 );
    VDP_drawText( server, 13 , 13 );


    SRAM_writeByte(0, atoi( server ));
    SRAM_writeByte(1, atoi( server + 4 ));
    SRAM_writeByte(2, atoi( server + 8 ));
    SRAM_writeByte(3, atoi( server + 12));
    


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


