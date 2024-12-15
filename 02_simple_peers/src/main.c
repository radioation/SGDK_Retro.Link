#include <genesis.h>
#include "network.h"

int cursor_x, cursor_y;
u8 buttons, buttons_prev;

int main()
{
    SYS_disableInts();                      // Disable interrupts
    VDP_setScreenWidth320();                // Set screen Width
    VDP_setScreenHeight224();               // Set screen Height
    VDP_setBackgroundColor(0);              // Set background black
    VDP_setTextPlane(BG_B);                 // Use PLANE B for text rendering
    VDP_setTextPalette(0);                  // Use palette 0 for text color
    SYS_enableInts();                       // Enable interrupts (allows our callback routine to print data)

    PAL_fadeOutPalette(PAL0,1,FALSE);
    VDP_setBackgroundColor(51);             // Blue background
    PAL_fadeInPalette(PAL0, palette_grey, 16, FALSE);

    cursor_x = 0;
    cursor_y = 1;

    VDP_drawText("Detecting adapter...[  ]", cursor_x, cursor_y); cursor_x+=21; 
    NET_initialize(); // Detect cartridge and set boolean variable

    if(cart_present)
    {
        VDP_setTextPalette(2); // Green text
        VDP_drawText("Ok", cursor_x, cursor_y); cursor_x=0; cursor_y+=2;
        VDP_setTextPalette(0); // White text
    }
    else
    {
        VDP_setTextPalette(1); // Red text
        VDP_drawText("XX", cursor_x, cursor_y); cursor_x=0; cursor_y+=2;
        VDP_setTextPalette(0); // White text
        VDP_drawText("Adapter not present", cursor_x, cursor_y);
        while(1) { SYS_doVBlankProcess(); }
    }

//------------------------------------------------------------------
// MAIN LOOP
//------------------------------------------------------------------

    VDP_drawText("IP Address:", cursor_x, cursor_y); 
    NET_printIP(cursor_x+12, cursor_y); cursor_y++;

    VDP_drawText("MAC:", cursor_x, cursor_y); 
    NET_printMAC(cursor_x+5, cursor_y); cursor_y+=2;

    waitMs(2000);

    NET_pingIP(cursor_x, cursor_y, 4, "10.86.22.6"); cursor_y+=6;

    waitMs(2000);



    u8 mode = 0; // 0 - not set, 1 - server, 2 - client
    while(1) // Loop forever 
    { 


	    buttons = JOY_readJoypad(JOY_1);
	    if( mode == 0 ) {

		    if(buttons & BUTTON_A && buttons_prev == 0x00) { 
			    mode = 1;
			    // start listening

		    }else if(buttons & BUTTON_C && buttons_prev == 0x00) { 
			    // try to connect to server.
			    mode = 2;
			    VDP_drawText("Rebooting adapter...", cursor_x ,cursor_y); cursor_y+=2;
			    NET_resetAdapter();

			    NET_connect(cursor_x, cursor_y, "10.86.22.6:8080"); cursor_x=0; cursor_y++;
		    }
	    } else if ( mode == 1 ) {
	    } else if ( mode == 2 ) {
		    if(buttons & BUTTON_START && buttons_prev == 0x00) { NET_sendMessage("Test 1, 2, 3\n"); }
		    if(buttons & BUTTON_A && buttons_prev == 0x00) { NET_sendMessage("Button A Pressed\n"); }
		    if(buttons & BUTTON_B && buttons_prev == 0x00) { NET_sendMessage("Button B Pressed\n"); }
		    if(buttons & BUTTON_C && buttons_prev == 0x00) { NET_sendMessage("Button C Pressed\n"); }
	    }
	    while(NET_RXReady()) // while data in hardware receive FIFO
	    {   
		    u8 byte = NET_readByte(); // Retrieve byte from RX hardware Fifo directly
		    switch(byte)
		    {
			    case 0x0A: // a line feed?
				    cursor_y++;
				    cursor_x=1;
				    break;              
			    case 0x0D: // a carridge Return?
				    cursor_x=1;
				    break; 
			    default:   // print
				    if (cursor_x >= 40) { cursor_x=0; cursor_y++; }
				    if (cursor_y >= 28) { cursor_x=0; cursor_y=0; }
				    sprintf(str, "%c", byte); // Convert
				    VDP_drawText(str, cursor_x, cursor_y); cursor_x++;
				    break;
		    }
	    }
	    buttons_prev = buttons;
	    SYS_doVBlankProcess(); 
    }

    //------------------------------------------------------------------
    return(0);
}




