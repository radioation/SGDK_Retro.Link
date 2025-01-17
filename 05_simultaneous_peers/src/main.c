#include <genesis.h>
#include "network.h"

int cursor_x, cursor_y;
u8 buttons, buttons_prev;


// **Identification**
// need to have an ID for each console that is unique
#define IM_NOBODY 0
#define IM_HOST 1
#define IM_CLIENT 2
u8 whoAmI = IM_NOBODY; 


// network stuff
char server[16] = "000.000.000.000"; 
unsigned int local_current_frame;
unsigned int server_current_frame;
uint8 local_history[32];
uint8 remote_history[32];

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
    whoAmI = IM_CLIENT;
    cursor_y = 5;
    NET_connect(cursor_x, cursor_y, "010.086.022.036:5364"); cursor_x=0; cursor_y++;
}


void setWhoAmI() {
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
            whoAmI = IM_HOST; 
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
    // Clear Buffers
    local_current_frame = 0; // local frame counter
    server_current_frame = 0; // frame counter received from server

    memset( local_history, 0, sizeof( local_history ));
    memset( remote_history, 0, sizeof( remote_history ));

    ///////////////////////////////////////////////////////////
    // Establish Comms (find/talk to other console)
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


    // get server address from SRAM or user.
    SRAM_enable();
    u8 part = SRAM_readByte(0);
    char textPart[4];
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
    // TODO: add ping here and save if successful.
    //  for now, saving always.
    SRAM_writeByte(0, atoi( server ));
    SRAM_writeByte(1, atoi( server + 4 ));
    SRAM_writeByte(2, atoi( server + 8 ));
    SRAM_writeByte(3, atoi( server + 12));

 

    // console waits for players to select host or client role.
    setWhoAmI(); 



    ///////////////////////////////////////////////////////////
    // Establish COmms
    get_latench();


    ///////////////////////////////////////////////////////////
    // Synchronize 
    





    //------------------------------------------------------------------
    // MAIN LOOP
    //------------------------------------------------------------------
    while(1) // Loop forever 
    { 
        buttons = JOY_readJoypad(JOY_1);
        if ( whoAmI == IM_HOST ) { 
            // HOST mode
        } else if ( whoAmI == IM_CLIENT ) {
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


