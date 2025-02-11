#include <genesis.h>
#include "resources.h"
#include "network.h"
#include "ip_input.h"

int cursor_x, cursor_y;
u8 buttons, buttons_prev;


////////////////////////////////////////////////////////////////////////////
// **Identification**
// need to have an ID for each console that is unique
#define IM_NOBODY 0
#define IM_HOST 1
#define IM_CLIENT 2
u8 whoAmI = IM_NOBODY; 

////////////////////////////////////////////////////////////////////////////
// network stuff
char server[16] = "000.000.000.000"; 
unsigned int local_current_frame;
unsigned int server_current_frame;
#define HISTORY_BUFFER_SIZE 32
u8 local_history[HISTORY_BUFFER_SIZE]; // circular buffer of local player inputs
u8 remote_history[HISTORY_BUFFER_SIZE]; // circular bufer of remote player inputs

u8 latency_checks = 8;
int vbl_counter = 0;
int frame_delay;
u16 receive_latency;

////////////////////////////////////////////////////////////////////////////
// Gameplay

#define TOP_EDGE 0
#define RIGHT_EDGE 319
#define LEFT_EDGE 0
#define BOTTOM_EDGE 223

#define MAX_SHOTS 2


// the players
struct CP_SPRITE {
    Sprite *sprite;

    s16 pos_x;
    s16 pos_y;

    s16 vel_x;
    s16 vel_y;

    s16 hitbox_x1;
    s16 hitbox_y1;
    s16 hitbox_x2;
    s16 hitbox_y2;

    bool active;
};



struct CP_SPRITE hostPlayer;
struct CP_SPRITE hostShots[MAX_SHOTS];
u16 hostJoyDir = 0;

struct CP_SPRITE clientPlayer;
struct CP_SPRITE clientShots[MAX_SHOTS];
u16 clientJoyDir = 0;




////////////////////////////////////////////////////////////////////////////
// video functions.
void VBlankHandler()
{
    //
    vbl_counter++;

}

static inline void waitForVBlankEnd() {
    __asm volatile (
            "1:\n\t"
            "move.w  0xC00004,%%d1\n\t"   // Read VDP status
            "and.b   #0x08,%%d1\n\t"      // Mask VBlank bit
            "bne.s   1b\n\t"              // If still in VBlank, loop
            :
            :
            : "d1"  // Clobber list (marks D1 as modified)
            );
}
static inline void ensureNonInterlaceVideoMode() {
    __asm volatile (
            "move.w  #0x8c81,0xC00004\n\t"
            );
}

static inline void ensureInterlaceVideoMode() {
    __asm volatile (
            "move.w  #0x8c83,0xC00004\n\t"
            );
}

////////////////////////////////////////////////////////////////////////////
// network functions.
void latency_send() {
    VDP_drawText("   Entered latency_send()", 0, 6);
    int count = 0;
    int curr_vbl_counter = 0; 
    int prev_vbl_counter = 0; 

    // number of time we are goint to sample latency
    while( count < latency_checks ) {
        // Wait until we aren't in VBLANK.
        waitForVBlankEnd();

        // ok to send?
        while( ! NET_TXReady() ); // keep waiting.

        // send latency token 
        NET_sendByte('@');

        // reset VBLANK counter
        vbl_counter = 0; 

        waitForVBlankEnd();
        // get response
        while( ! NET_RXReady() ); // is data availableR?
        curr_vbl_counter = vbl_counter; // get current vblank count 
        u8 ret = NET_readByte();
        // is latency byte?
        if( ret != '@' ) {
            continue; // NOPE! start over
        }

        // is current local VBLANK larger than previous
        if( curr_vbl_counter > prev_vbl_counter ) {
            prev_vbl_counter = curr_vbl_counter;
        } else {
            count+=1; // change latency chekc counter. 
        }
    }    
    VDP_drawText("    Middle latency_send()", 0, 6);
    // done with it. INspect result
    int delay = curr_vbl_counter >> 1;  // divide by 2
    if ( delay % 2 ) { // check even.
        delay+=1; // make it even
    }
    frame_delay = delay; // store frame delay locally

    char message[40];
    memset(message,0, sizeof(message));
    sprintf( message, "  frame_delay: %d", frame_delay );
    VDP_drawText(message, 0, 7 );

    //OK to send?
    while( ! NET_TXReady() );
    // send termination byte to other console.
    VDP_drawText("    Send term latency_send()", 0, 6);
    NET_sendByte('%');
    while( true ) { 
        while( ! NET_RXReady() );
        u8 ret = NET_readByte();
        if( ret == 'K' ) break;  // ACK Byte? 
    }

    VDP_drawText("    Send d latency_send()", 0, 6);
    // send frame delay 
    NET_sendByte( (unsigned char)(frame_delay & 0xFF) ); // send frame delay byte 
    NET_sendByte( (unsigned char)((frame_delay >>8 ) & 0xFF) ); // send frame delay byte 

}


void latency_recv() {
    VDP_drawText("   Entered latency_recv()", 0, 6);
    while(1) {
        // wait for byte
        while( ! NET_RXReady() ); // wait until available.
        u8 ret = NET_readByte(); // get byte

        // check for latency loop termination token.
        if( ret == '%' ) {
            // we done brah
            break;
        }

        // OK to send?
        while( ! NET_TXReady() );
        // echo it back
        NET_sendByte(ret);
        // loop around again.
    }

    VDP_drawText("    middle latency_recv()", 0, 6);

    while( ! NET_TXReady() );
    VDP_drawText("    send K latency_recv()", 0, 6);
    NET_sendByte('K');

    // get first byte
    while( ! NET_RXReady() );
    VDP_drawText("     GET 1 latency_recv()", 0, 6);
    u8 byte1 = NET_readByte(); 

    // get second byte
    u8 byte2 = NET_readByte(); 
    VDP_drawText("     GET 2 latency_recv()", 0, 6);
    receive_latency = (byte2 << 8 ) + byte1;
    char message[40];
    memset(message,0, sizeof(message));
    sprintf( message, "  recv_latency: %d", receive_latency );
    VDP_drawText(message, 0, 7 );

}

void get_latency() {
    // whoami?
    if( whoAmI == IM_HOST) {
        latency_send();
    } else if( whoAmI == IM_CLIENT) {
        // client recieves first
        latency_recv();
    }
}


void sync_host() {
    VDP_drawText("   Entered sync_host()", 0, 8);
    while(1) {
        // Data available?
        while( ! NET_RXReady() ); // If not keep checking for data.

        // get the byte
        u8 byte = NET_readByte(); 
        if( byte == '*' ) break;  // is it the OK byte from client?
    }
    VDP_drawText("       mid sync_host()", 0, 8);

    ensureNonInterlaceVideoMode();

    while(1) { 
        waitForVBlankEnd();  // wait until we aren't in vblank

        // get HVCounter value
        u16 hv = GET_HVCOUNTER; // get HVCOUNTER VALUE
        u8  v = hv >> 8; // shift it to point to V-Counter

        while( ! NET_TXReady() ); // check if it's OK to send.
        NET_sendByte(v); // send vcount value

        // Data available?
        while( ! NET_RXReady() ); 
        u8 byte = NET_readByte(); // Get Byte
        if( byte == '$' ) break;  // did we receive synced token? if so, we're done
    }
    VDP_drawText("      done sync_host()", 0, 8);
    NET_flushBuffers();

}

void sync_client() {
    VDP_drawText("   Entered sync_client()", 0, 8);
    // Define the addresses for the VDP registers
    volatile u16 * const vdpStatus  = (volatile u16 *)0xC00004;
    volatile u16 * const vdpVCounter = (volatile u16 *)0xC00008;
    s16  scanline = 0;
    s16 READ_VCOUNT = 93; // READ_VCOUNT         EQU $5D       ; VCOUNT CONSIDERED NORMAL READ TIME

    // Ok to send data?
    while( ! NET_TXReady() );
    NET_sendByte('*'); // send vcount value

    vbl_counter = 0; // clear VBLANK counter
    while( vbl_counter != 0 ) {} // client delay.  not equal keep checking?

    VDP_drawText("       mid sync_client()", 0, 8);
    ensureNonInterlaceVideoMode(); // ensure non interlace video mode.


    while(1) {  // client vblank sync:

        // is data available in sega UART
        while( ! NET_RXReady() );
        u8 remoteVCount = NET_readByte(); // read remote vcount value

        u16 status = *vdpStatus; // get VDP status
        if( status & 0x08 ) {  //In VBlank?
            scanline = 0;   // we are in VBLANK so zero it.
        } else {
            // NOPE, so get VCOunter 
            scanline = *vdpVCounter;
            // bigger than max scanline
            if( scanline > READ_VCOUNT ) {
                scanline = READ_VCOUNT;
            }
        }
        // OK! 
        s16 diff = scanline - remoteVCount;
        if( diff < 0 ) {
            // negative, invert value
            diff = -diff;    
        }
        // postivie here
        if( diff <1 ) {
            break; // less than one apart, we are synced
        }

        // send another vcount request
        // is it OK to send?
        while( ! NET_TXReady() );
        NET_sendByte('%'); // send out byte to get another vcount 
        ensureInterlaceVideoMode(); // SLOW DOWN
        // try again in loop
    }

    // host vblank sycned, get back to normal speed
    ensureNonInterlaceVideoMode();

    // latency value FRAME_DELAY set by other functions
    // OK to send?
    while( ! NET_TXReady() );
    NET_sendByte('$'); // send sync done token


    vbl_counter = 0; // clear VBLANK counter
    while( vbl_counter != frame_delay ) {} // client delay.  not equal keep checking?

    VDP_drawText("      EXIT sync_client()", 0, 8);
    NET_flushBuffers();

}

void synchronize() {
    if( whoAmI == IM_HOST) {
        sync_host();
    } else if( whoAmI == IM_CLIENT) {
        // client recieves first
        sync_client();
    }

}




void host_game() {
    // Allow client to join 
    NET_allowConnections();
    VDP_drawText("   Wait for client:      ", 0, 5);

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
            VDP_drawText("   Client Connected!     ", 0, 5);
            return;
        }
    }

}

bool join_game() {
    VDP_drawText("   Connect to server    ", 0, 5);
    cursor_y = 5;
    // blocks whilewaiting for network to be ready.
    char fullserver[21];
    memset(fullserver,0, sizeof(fullserver));
    sprintf( fullserver, "%s:5364", server);
    return NET_connect(cursor_x, cursor_y, fullserver); cursor_x=0; cursor_y++;
}


void setWhoAmI() {
    buttons = 0; 
    buttons_prev = 0;
    VDP_clearTextArea( 0, 0,  40, 13  );
    VDP_drawText("          (A) - Host Game", 0, 5);
    VDP_drawText("          (C) - Join Game", 0, 7);
    NET_resetAdapter();
    while(1) // loop forever
    { 
        buttons = JOY_readJoypad(JOY_1);
        // MODE NOT SET, button press will determine server or client.
        if(buttons & BUTTON_A && buttons_prev == 0x00) { 
            VDP_clearTextArea( 0, 5,  40, 3 );
            cursor_y = 5;
            whoAmI = IM_HOST; 
            // start listening 
            host_game();
            break;
        }else if(buttons & BUTTON_C && buttons_prev == 0x00) { 
            VDP_clearTextArea( 0, 5,  40, 3 );
            // try to connect to server.
            whoAmI = IM_CLIENT;
            join_game(); 
            break;

        }
        buttons_prev = buttons;
        SYS_doVBlankProcess();
    }
}


void init_network_buffers() {
    ///////////////////////////////////////////////////////////
    // Clear History
    local_current_frame = 0; // local frame counter
    server_current_frame = 0; // frame counter received from server

    memset( local_history, 0, sizeof( local_history ));
    memset( remote_history, 0, sizeof( remote_history ));
}



////////////////////////////////////////////////////////////////////////////
// game



void update() {
    u16 currDir = 0;
    u16 joyState = host_history[ host_current_frame ];
    if (joyState & BUTTON_RIGHT)
    {
        hostPlayer.vel_x = 2;
        currDir |= BUTTON_RIGHT;
    }
    else if (joyState & BUTTON_LEFT)
    {
        hostPlayer.vel_x = -2;
        currDir |= BUTTON_LEFT;
    } else {
        hostPlayer.vel_x = 0;
    }

    if (joyState & BUTTON_UP)
    {
        hostPlayer.vel_y = -2;
        currDir |= BUTTON_UP;
    }
    else if (joyState & BUTTON_DOWN)
    {
        hostPlayer.vel_y = 2;
        currDir |= BUTTON_DOWN;
    } else {
        hostPlayer.vel_y = 0;
    }
    if( currDir ) {
        hostJoyDir = currDir;
    }
    if (joyState & BUTTON_A) {
        s16 prevFrame = host_current_frame - 1;
        if( prevFrame < 0 ) {
            prevFrame = HISTORY_BUFFER_SIZE - 1;
        }
        u16 prevJoyState = host_history[ prevFrame ];
        if( ( prevJoyState & BUTTON_A) == 0 ) {
            // fire code here
            for( u8 i=0; i < MAX_SHOTS; ++i ) {
                if( hostShots[i].active == false ) {
                    // move it
                    hostShots[i].pos_x = hostPlayer.pos_x + 4;
                    hostShots[i].pos_y = hostPlayer.pos_y + 4;
                    hostShots[i].active = true;
                    if( hostJoyDir & BUTTON_RIGHT ) {
                        hostShots[i].vel_x = 4;
                    }else if( hostJoyDir & BUTTON_LEFT ) {
                        hostShots[i].vel_x = -4;
                    }
                    if( hostJoyDir & BUTTON_UP ) {
                        hostShots[i].vel_y = -4;
                    }else if( hostJoyDir & BUTTON_DOWN ) {
                        hostShots[i].vel_y = 4;
                    }
                    break;
                }
            }
        }
    }
    //Position the players
    hostPlayer.pos_x += hostPlayer.vel_x;
    hostPlayer.pos_y += hostPlayer.vel_y;

    for( u16 i=0; i < MAX_SHOTS; ++i ) {
        if( hostShots[i].active == TRUE ) {
            hostShots[i].pos_x +=  hostShots[i].vel_x;
            hostShots[i].pos_y +=  hostShots[i].vel_y;
            if(hostShots[i].pos_y  < 0 || hostShots[i].pos_y > 224
                    || hostShots[i].pos_x < 0 || hostShots[i].pos_x > 319) {
                hostShots[i].pos_x = -16;
                hostShots[i].pos_y = -10;
                hostShots[i].vel_x = 0;
                hostShots[i].vel_y = 0;
                hostShots[i].active = FALSE;
            }
            SPR_setPosition(hostShots[i].sprite,hostShots[i].pos_x,hostShots[i].pos_y);
        } else {
            SPR_setPosition( hostShots[i].sprite, -32, -22 );
        }
    }


    currDir = 0;
    joyState = client_history[ client_current_frame ];
    if (joyState & BUTTON_RIGHT)
    {
        clientPlayer.vel_x = 2;
        currDir |= BUTTON_RIGHT;
    }
    else if (joyState & BUTTON_LEFT)
    {
        clientPlayer.vel_x = -2;
        currDir |= BUTTON_LEFT;
    } else {
        clientPlayer.vel_x = 0;
    }

    if (joyState & BUTTON_UP)
    {
        clientPlayer.vel_y = -2;
        currDir |= BUTTON_UP;
    }
    else if (joyState & BUTTON_DOWN)
    {
        clientPlayer.vel_y = 2;
        currDir |= BUTTON_DOWN;
    } else {
        clientPlayer.vel_y = 0;
    }
    if( currDir ) {
        clientJoyDir = currDir;
    }

    if (joyState & BUTTON_A) {
        s16 prevFrame = client_current_frame - 1;
        if( prevFrame < 0 ) {
            prevFrame = HISTORY_BUFFER_SIZE - 1;
        }
        u16 prevJoyState = client_history[ prevFrame ];
        if( ( prevJoyState & BUTTON_A) == 0 ) {
            // fire code here
            for( u8 i=0; i < MAX_SHOTS; ++i ) {
                if( clientShots[i].active == false ) {
                    // move it
                    clientShots[i].pos_x = clientPlayer.pos_x + 4;
                    clientShots[i].pos_y = clientPlayer.pos_y + 4;
                    clientShots[i].active = true;
                    if( clientJoyDir & BUTTON_RIGHT ) {
                        clientShots[i].vel_x = 4;
                    }else if( clientJoyDir & BUTTON_LEFT ) {
                        clientShots[i].vel_x = -4;
                    }
                    if( clientJoyDir & BUTTON_UP ) {
                        clientShots[i].vel_y = -4;
                    }else if( clientJoyDir & BUTTON_DOWN ) {
                        clientShots[i].vel_y = 4;
                    }
                    break;
                }
            }
        }
    }
    clientPlayer.pos_x += clientPlayer.vel_x;
    clientPlayer.pos_y += clientPlayer.vel_y;


    for( u16 i=0; i < MAX_SHOTS; ++i ) {
        if( clientShots[i].active == TRUE ) {
            clientShots[i].pos_x +=  clientShots[i].vel_x;
            clientShots[i].pos_y +=  clientShots[i].vel_y;
            if(clientShots[i].pos_y  < 0 || clientShots[i].pos_y > 224
                    || clientShots[i].pos_x < 0 || clientShots[i].pos_x > 319) {
                clientShots[i].pos_x = -16;
                clientShots[i].pos_y = -10;
                clientShots[i].vel_x = 0;
                clientShots[i].vel_y = 0;
                clientShots[i].active = FALSE;
            }
            SPR_setPosition(clientShots[i].sprite,clientShots[i].pos_x,clientShots[i].pos_y);
        } else {
            SPR_setPosition( clientShots[i].sprite, -32, -22 );
        }
    }



    SPR_setPosition( hostPlayer.sprite, hostPlayer.pos_x, hostPlayer.pos_y );
    SPR_setPosition( clientPlayer.sprite, clientPlayer.pos_x, clientPlayer.pos_y );

}

void checkCollisions() {

}


void createShots() {
    s16 xpos = -16;
    s16 ypos = 230;
    for( u16 i=0; i < MAX_SHOTS; ++i ) {
        hostShots[i].pos_x = xpos;
        hostShots[i].pos_y = ypos;
        hostShots[i].vel_x = 0;
        hostShots[i].vel_y = 0;
        hostShots[i].active = FALSE;
        hostShots[i].hitbox_x1 = 0;
        hostShots[i].hitbox_y1 = 0;
        hostShots[i].hitbox_x2 = 8;
        hostShots[i].hitbox_y2 = 8;

        hostShots[i].sprite = SPR_addSprite( &shot, xpos, ypos, TILE_ATTR( PAL0, 0, FALSE, FALSE ));
        SPR_setAnim( hostShots[i].sprite, 0 );

        clientShots[i].pos_x = xpos;
        clientShots[i].pos_y = ypos;
        clientShots[i].vel_x = 0;
        clientShots[i].vel_y = 0;
        clientShots[i].active = FALSE;
        clientShots[i].hitbox_x1 = 0;
        clientShots[i].hitbox_y1 = 0;
        clientShots[i].hitbox_x2 = 8;
        clientShots[i].hitbox_y2 = 8;

        clientShots[i].sprite = SPR_addSprite( &shot, xpos, ypos, TILE_ATTR( PAL0, 0, FALSE, FALSE ));
        SPR_setAnim( clientShots[i].sprite, 0 );

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


    //SYS_setVIntCallback( VIntHandler );     // setup vertical interrupt
    SYS_setVBlankCallback( VBlankHandler );     // setup vertical interrupt

    SYS_enableInts();                       // Enable interrupts (allows our callback routine to print data)

    PAL_fadeOutPalette(PAL0,1,FALSE);
    VDP_setBackgroundColor(51);             // Blue background
    PAL_fadeInPalette(PAL0, palette_grey, 16, FALSE);

    cursor_x = 0;
    cursor_y = 1;



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
    SYS_doVBlankProcess();
    // clear out last input and wait a sec.
    waitMs(1000);
    JOY_update();

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
    // Establish Comms
    get_latency();

    ///////////////////////////////////////////////////////////
    // Synchronize 
    synchronize();

    ///////////////////////////////////////////////////////////
    // GAME ON

    PAL_setPalette( PAL0, shot_pal.data, CPU);

    // Sprite initializatoin
    SPR_init();

    // set intiial position for host
    hostPlayer.pos_x = 10;
    hostPlayer.pos_y = 10;
    hostPlayer.vel_x = 0;
    hostPlayer.vel_y = 0;
    hostPlayer.active = TRUE;
    hostPlayer.hitbox_x1 = 0;
    hostPlayer.hitbox_y1 = 0;
    hostPlayer.hitbox_x2 = 16;
    hostPlayer.hitbox_y2 = 16;
    hostPlayer.sprite  = SPR_addSprite( &player, hostPlayer.pos_x, hostPlayer.pos_y, TILE_ATTR( PAL0, 0, FALSE,FALSE ));
    SPR_setAnim( hostPlayer.sprite, 0 );


    // set intiial position for host
    clientPlayer.pos_x = 303;
    clientPlayer.pos_y = 198;
    clientPlayer.vel_x = 0;
    clientPlayer.vel_y = 0;
    clientPlayer.active = TRUE;
    clientPlayer.hitbox_x1 = 0;
    clientPlayer.hitbox_y1 = 0;
    clientPlayer.hitbox_x2 = 16;
    clientPlayer.hitbox_y2 = 16;
    clientPlayer.sprite  = SPR_addSprite( &player, clientPlayer.pos_x, clientPlayer.pos_y, TILE_ATTR( PAL0, 0, FALSE,FALSE ));
    SPR_setAnim( clientPlayer.sprite, 1 );

    createShots();

    SPR_update();

    //JOY_setEventHandler( &gameJoyHandler ); actually don't.
    if ( whoAmI == IM_HOST ) { 
        VDP_drawText("HOST MODE", 15, 1 );
        // HOST mode

    } else if ( whoAmI == IM_CLIENT ) {
        // Client mode
        VDP_drawText("CLIENT MODE", 14, 1 );                  
    }
    //------------------------------------------------------------------
    // MAIN LOOP
    //------------------------------------------------------------------
    init_network_buffers(); 

    while(1) // Loop forever 
    { 

        pre_tick(); // execute before every game tick


        game_tick(); // always called 


        SYS_doVBlankProcess(); 
    }

    //------------------------------------------------------------------
    return(0);
}


