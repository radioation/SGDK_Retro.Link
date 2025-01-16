#include <genesis.h>
#include "network.h"

int cursor_x, cursor_y;
u8 buttons, buttons_prev;
u8 mode = 0; // 0 - not set, 1 - Host, 2 - Client


void sync_host() {
    //
}

void sync_client() {
}


void host_game() {
    // Allow client to join 
    NET_allowConnections();
    VDP_drawText("           Waiting       ", 0, 5);

    // loop while waiting for a peer.
    u8 offset = 0;
    while( 1 ) {
        // Data available?
        while( !NET_RXReady() ){
            // writing some sort of text here?
            VDP_drawText(" .     ", 21+offset, 5);
            offset +=1;
            if( offset > 3 ) offset = 0;
            waitMs(16);

        }
        // look for 'C'
        u8 ret = NET_readByte();
        if( ret == 'C' ) {
            // clear text before losing out
            VDP_drawText("          Connected!     ", 0, 5);
            return;
        }
    }

}

void join_game() {
    mode = 2;
    cursor_y = 5;
    NET_connect(cursor_x, cursor_y, "010.086.022.036:5364"); cursor_x=0; cursor_y++;
}


void set_game_mode() {
    VDP_clearTextArea( 0, 0,  40, 13  );
    VDP_drawText("          (A) - Host Game", 0, 5);
    VDP_drawText("          (C) - Join Game", 0, 7);
    NET_resetAdapter();
    bool done = false;
    while(!done) // Loop for a while
    { 
        buttons = JOY_readJoypad(JOY_1);
        // MODE NOT SET, button press will determine server or client.
        if(buttons & BUTTON_A && buttons_prev == 0x00) { 
            VDP_clearTextArea( 0, 5,  40, 3 );
            cursor_y = 5;
            mode = 1;
            // start listening 
            host_game();

        }else if(buttons & BUTTON_C && buttons_prev == 0x00) { 
            VDP_clearTextArea( 0, 5,  40, 3 );
            // try to connect to server.
            join_game();

        }
    }
}





int main()
{
    ///////////////////////////////////////////////////////////
    // basic setup
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

    ///////////////////////////////////////////////////////////
    // test retro.link 
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


    VDP_drawText("IP Address:", cursor_x, cursor_y); 
    NET_printIP(cursor_x+12, cursor_y); cursor_y++;

    VDP_drawText("MAC:", cursor_x, cursor_y); 
    NET_printMAC(cursor_x+5, cursor_y); cursor_y+=2;


    waitMs(2000);


    set_game_mode();



    //------------------------------------------------------------------
    // MAIN LOOP
    //------------------------------------------------------------------
    while(1) // Loop forever 
    { 
        buttons = JOY_readJoypad(JOY_1);
        if ( mode == 1 ) { 
            // HOST mode
        } else if ( mode == 2 ) {
            // Client mode
        }

        /*
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
         */
        SYS_doVBlankProcess(); 
    }

    //------------------------------------------------------------------
    return(0);
}


