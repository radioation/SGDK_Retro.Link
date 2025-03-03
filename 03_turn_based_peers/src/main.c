#include <genesis.h>
#include "resources.h"
#include "network.h" 

#define INPUT_WAIT_COUNT 10

int text_cursor_x, text_cursor_y;

// Chess Piece data
// Enum to represent piece types
typedef enum {
    EMPTY = 0,
    KING = 3, 
    QUEEN = 6, 
    ROOK = 9, 
    BISHOP = 12, 
    KNIGHT = 15, 
    PAWN = 18 
} PIECE_TYPE;

typedef enum {
    NO_PLAYER = 0,
    PLAYER_ONE = 1,
    PLAYER_TWO = 2
} PLAYER;

// Structure to represent a chess piece
typedef struct {
    PIECE_TYPE type;   // Type of the piece
    PLAYER player;     // which player  
} CHESS_PIECE;

CHESS_PIECE board[8][8]; // X, Y
int piecesTileIndex = -1;
const s8 boardStartCol = 8;
const s8 boardStartRow = 2;
const s8 boardStep = 3;

void setup_pieces() {
    // clear the board
    memset(board, 0, sizeof(CHESS_PIECE) * 8 * 8); // Set all to empty

    // set  pieces up
    board[0][7] = (CHESS_PIECE){ROOK, PLAYER_ONE};   board[7][7] = (CHESS_PIECE){ROOK, PLAYER_ONE};
    board[1][7] = (CHESS_PIECE){KNIGHT, PLAYER_ONE}; board[6][7] = (CHESS_PIECE){KNIGHT, PLAYER_ONE};
    board[2][7] = (CHESS_PIECE){BISHOP, PLAYER_ONE}; board[5][7] = (CHESS_PIECE){BISHOP, PLAYER_ONE};
    board[3][7] = (CHESS_PIECE){QUEEN, PLAYER_ONE};  board[4][7] = (CHESS_PIECE){KING, PLAYER_ONE};
    for (int i = 0; i < 8; i++) {
        board[i][6] = (CHESS_PIECE){PAWN, PLAYER_ONE};
    }

    board[0][0] = (CHESS_PIECE){ROOK, PLAYER_TWO};   board[7][0] = (CHESS_PIECE){ROOK, PLAYER_TWO};
    board[1][0] = (CHESS_PIECE){KNIGHT, PLAYER_TWO}; board[6][0] = (CHESS_PIECE){KNIGHT, PLAYER_TWO};
    board[2][0] = (CHESS_PIECE){BISHOP, PLAYER_TWO}; board[5][0] = (CHESS_PIECE){BISHOP, PLAYER_TWO};
    board[3][0] = (CHESS_PIECE){QUEEN, PLAYER_TWO};  board[4][0] = (CHESS_PIECE){KING, PLAYER_TWO};
    for (int i = 0; i < 8; i++) {
        board[i][1] = (CHESS_PIECE){PAWN, PLAYER_TWO};
    }
}

void draw_pieces(){
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if( board[col][row].player > 0 ) {
                s8 yStart = 0;
                if( board[col][row].player == 2 ) {
                    yStart = 3;
                }
                VDP_setTileMapEx( BG_A, pieces_img.tilemap, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, piecesTileIndex),    
                        boardStartCol + col * boardStep,  // PLANE X Dest in tiles
                        boardStartRow + row * boardStep,  // PLANE Y Dest in tiles
                        board[col][row].type,  // REGION X start
                        yStart,  // REGION Y start
                        boardStep,  // Width
                        boardStep,  // Height
                        CPU);
            }
        }
    }
}


void move_piece( int startCol, int startRow, int endCol, int endRow ){
    board[endCol][endRow] = board[startCol][startRow];
    board[startCol][startRow] = (CHESS_PIECE){EMPTY, NO_PLAYER}; 
    draw_pieces();

    VDP_setTileMapEx( BG_A, pieces_img.tilemap, TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, piecesTileIndex),    
            boardStartCol + startCol * boardStep,  // PLANE X Dest in tiles
            boardStartRow + startRow * boardStep,  // PLANE Y Dest in tiles
            EMPTY,  // REGION X start
            0,  // REGION Y start
            boardStep,  // Width
            boardStep,  // Height
            CPU);

}

// Sprite data structures
typedef struct 
{
    Sprite *sprite;
    s8 col;     // board col
    s8 row;     // board row
    s16 pos_x;  
    s16 pos_y;
    s8 sel_col; // selected board column
    s8 sel_row; // selected board row
    s16 sel_pos_x;
    s16 sel_pos_y;
    Sprite *selected_spr;
} CURSOR;

const s8 cursorStep = 24;
const s8 cursorColStart = 64;
const s8 cursorRowStart = 16;

void cursor_init( CURSOR *cursor, Sprite *sprite, Sprite *selected_sprite ) {
    cursor->col = 4;  // board position
    cursor->row = 4;
    cursor->pos_x = cursor->col * cursorStep + cursorColStart;
    cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
    cursor->sprite =  sprite;

    cursor->sel_col = -1;  // not on board
    cursor->sel_row = -1;
    cursor->sel_pos_x = -32;
    cursor->sel_pos_y = -32;
    cursor->selected_spr = selected_sprite;
    SPR_setAnim( cursor->selected_spr, 1 );
    SPR_setVisibility( cursor->selected_spr, HIDDEN );
}

bool cursor_move( CURSOR *cursor, u16 joypad ) {
    bool didMove = FALSE;
    if( joypad & BUTTON_LEFT ) {
        cursor->col--;
        cursor->pos_x = cursor->col * cursorStep + cursorColStart;
        didMove = TRUE;
    } 
    if( joypad & BUTTON_RIGHT ) {
        cursor->col++;
        cursor->pos_x = cursor->col * cursorStep + cursorColStart;
        didMove = TRUE;
    } 
    if( joypad & BUTTON_UP ) {
        cursor->row--;
        cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
        didMove = TRUE;
    }
    if( joypad & BUTTON_DOWN ) {
        cursor->row++;
        cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
        didMove = TRUE;
    }
    return didMove;
}

void cursor_update_from_pos( CURSOR *cursor, s8 col, s8 row, s8 sel_col, s8 sel_row ) {
    cursor->col = col;
    cursor->pos_x = cursor->col * cursorStep + cursorColStart;
    cursor->row = row;
    cursor->pos_y = cursor->row * cursorStep + cursorRowStart;

    cursor->sel_col = sel_col;
    cursor->sel_pos_x = cursor->sel_col * cursorStep + cursorColStart;
    cursor->sel_row = sel_row;
    cursor->sel_pos_y = cursor->sel_row * cursorStep + cursorRowStart;
    if( cursor->sel_col >= 0 ) {
        SPR_setVisibility( cursor->selected_spr, VISIBLE );
    } else {
        SPR_setVisibility( cursor->selected_spr, HIDDEN );
    }
}

void cursor_clear_selected( CURSOR* cursor ) {
    cursor->sel_col = -1;
    cursor->sel_row = -1;
    cursor->sel_pos_x = -32;
    cursor->sel_pos_y = -32;
    SPR_setVisibility( cursor->selected_spr, HIDDEN );
    char message[40];
    strclr(message);
    sprintf( message, "X: %d y: %d sx: %d sy %d    ", cursor->col, cursor->row, cursor->sel_col, cursor->sel_row);
}

bool cursor_action( CURSOR* cursor, CHESS_PIECE brd[8][8], u8 player ) {
    if( cursor->sel_col < 0 ) {
        // no piece selected yet, check if player owns the current piece.
        if( brd[(u8)cursor->col][(u8)cursor->row].player == player ) { 
            cursor->sel_col = cursor->col;
            cursor->sel_row = cursor->row;
            cursor->sel_pos_x = cursor->sel_col * cursorStep + cursorColStart;
            cursor->sel_pos_y = cursor->sel_row * cursorStep + cursorRowStart;
            SPR_setVisibility( cursor->selected_spr, VISIBLE );
        }
    } else {
        // A piece is selected, check if cursor position is valid )
        //TODO -- (add valid move check, for now just look for empty squares)
        if( brd[(u8)cursor->col][(u8)cursor->row].type == EMPTY ) { 
            move_piece( cursor->sel_col, cursor->sel_row, cursor->col, cursor->row );
            return true;
        }
    }
    return false;
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
            waitMs(20);

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



void cursor_send_data( CURSOR* cursor, u8 type  ) {
  NET_sendByte( 128 + type ); // first bit is always on, and 4 bytesl
  NET_sendByte( 4 ); // cursor always sends 4 bytes
  NET_sendByte( cursor->col );
  NET_sendByte( cursor->row );
  NET_sendByte( cursor->sel_col );
  NET_sendByte( cursor->sel_row );
}



void read_bytes_n(u8* data, u8 length ) {
    s16 bytePos = 0;
    while( bytePos < length ) {
        // read data
        if( NET_RXReady() ) {
            data[bytePos] = NET_readByte(); // Retrieve byte from RX hardware Fifo directly
            bytePos++;
        } else {
            waitMs(5);
        }
    }
}




int main(bool hard) {

    //////////////////////////////////////////////////////////////
    // setup screen and palettes
    SYS_disableInts();
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_setBackgroundColor(16);
    SYS_enableInts();                      // enable interrupts for networking print routines.


    //////////////////////////////////////////////////////////////
    // Networking setup
    text_cursor_x = 0; // networking text cursor location
    text_cursor_y = 0; // networking text cursor location

    VDP_drawText("Detecting adapter...[  ]", text_cursor_x, text_cursor_y); text_cursor_x+=21;   
    NET_initialize(); // Detect cartridge and set boolean variable

    if(cart_present)
    {
        VDP_setTextPalette(2); // Green text
        VDP_drawText("Ok", text_cursor_x, text_cursor_y); text_cursor_x=0; text_cursor_y+=2;
        VDP_setTextPalette(0); // White text
    }
    else
    {
        VDP_setTextPalette(1); // Red text
        VDP_drawText("XX", text_cursor_x, text_cursor_y); text_cursor_x=0; text_cursor_y+=2;
        VDP_setTextPalette(0); // White text
        VDP_drawText("Adapter not present", text_cursor_x, text_cursor_y);
        while(1) { SYS_doVBlankProcess(); }
    }


    VDP_drawText("IP Address:", text_cursor_x, text_cursor_y);
    NET_printIP(text_cursor_x+12, text_cursor_y); text_cursor_y++;

    VDP_drawText("MAC:", text_cursor_x, text_cursor_y);
    NET_printMAC(text_cursor_x+5, text_cursor_y); text_cursor_y+=2;


    NET_resetAdapter();
    VDP_drawText("          (A) - Host Game", 0, 5);
    VDP_drawText("          (C) - Join Game", 0, 7);

    u8 me = 0; // 0 - not set, 1 - PLAYER_ONE, 2 - PLAYER_TWO

    //////////////////////////////////////////////////////////////
    // Networking Loop  
    u8 buttons_prev;
    while(1) 
    {
        u8 buttons = JOY_readJoypad(JOY_1);
        // MODE NOT SET, button press will determine server or client.
        if(buttons & BUTTON_A && buttons_prev == 0x00) {
            VDP_drawText("                         ", 0, 5);
            VDP_drawText("                         ", 0, 7);
            text_cursor_y = 5;
            me = PLAYER_ONE;
            // start listening
            host_game();
            break;

        }else if(buttons & BUTTON_C && buttons_prev == 0x00) {
            VDP_drawText("                         ", 0, 5);
            VDP_drawText("                         ", 0, 7);
            // try to connect to server.
            me = PLAYER_TWO;
            text_cursor_y = 5;
            NET_connect(text_cursor_x, text_cursor_y, "010.086.022.026:5364"); text_cursor_x=0; text_cursor_y++;
            break;
        }
        buttons_prev = buttons;
        SYS_doVBlankProcess();
    }
    NET_flushBuffers();
    VDP_clearPlane( BG_A, TRUE);
    VDP_clearPlane( BG_B, TRUE);

    //////////////////////////////////////////////////////////////
    // setup background
    PAL_setPalette( PAL0, board_pal.data, CPU );
    PAL_setPalette( PAL1, pieces_pal.data, CPU );
    PAL_setPalette( PAL2, cursor_pal.data, CPU );

    int ind = TILE_USER_INDEX;
    VDP_drawImageEx(BG_B, &board_img, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += board_img.tileset->numTile; // get offset of tiles
    piecesTileIndex = ind;
    // now load pieces tileset
    VDP_loadTileSet(pieces_img.tileset, piecesTileIndex, CPU);
    setup_pieces();
    draw_pieces();

    //////////////////////////////////////////////////////////////
    // sprites 
    SPR_init();
    CURSOR cursor;
    cursor_init(&cursor, 
            SPR_addSprite( &cursor_spr, cursor.pos_x, cursor.pos_y, TILE_ATTR(PAL2, FALSE, FALSE, FALSE )),
            SPR_addSprite( &cursor_spr, cursor.pos_x, cursor.pos_y, TILE_ATTR(PAL2, FALSE, FALSE, FALSE )) );


    //////////////////////////////////////////////////////////////
    // main loop.
    u8 inputWait = 0;
    u8 currentPlayer = PLAYER_ONE;

    VDP_setTextPlane( BG_A );
    VDP_drawText("PLAYER ONE MOVE", 13, 1);
    while(TRUE)
    {
        if( currentPlayer == me ) {
            // read joypad to mover cursor
            u16 joypad  = JOY_readJoypad( JOY_1 );
            if( inputWait == 0 ) {
                if( cursor_move( &cursor, joypad ) == TRUE ) {
                    inputWait = INPUT_WAIT_COUNT;
                    // send cursor data
                    cursor_send_data( &cursor, 0 );
                }
                if( joypad & BUTTON_A ) {
                    bool didMove =  cursor_action( &cursor, board, me );
                    inputWait = INPUT_WAIT_COUNT;
                    if( didMove ) {
                        // send move data.
                        cursor_send_data( &cursor, 1 );  // move piece
                        cursor_clear_selected(&cursor); 
                        if( currentPlayer == PLAYER_ONE ) {
                            currentPlayer = PLAYER_TWO;
                            VDP_drawText("TWO", 20, 1);
                        } else {
                            currentPlayer = PLAYER_ONE;
                            VDP_drawText("ONE", 20, 1);
                        }
                    } else {
                        cursor_send_data( &cursor, 0 ); // just update cursor
                    }
                } else if( joypad & BUTTON_C ) {
                    cursor_clear_selected( &cursor );
                    inputWait = INPUT_WAIT_COUNT;
                    // send cursor data
                    cursor_send_data( &cursor,0 );
                }
            } else {
                if( inputWait > 0 ) {
                    --inputWait;
                }
            }
        } else {
            // current player is not me, listen for data

            // check if readable
    VDP_drawText("L 0 ", 0, 0 );
            if( NET_RXReady() ) {
    VDP_drawText("L 1 ", 0, 1 );
                // read the header
                u8 header[2];

                read_bytes_n( header, 2 );
    VDP_drawText("L 2 ", 0, 2 );
                u8 data_type = header[0];
    char message[40];
    strclr(message);
    sprintf( message, "T: %d   ", data_type);
    VDP_drawText(message, 0, 3);
                u8 data_length = header[1];
    sprintf( message, "L: %d   ", data_length);
    VDP_drawText(message, 0, 4);
                // read the data
                u8 buffer[16]; 
                read_bytes_n( buffer, data_length );
    VDP_drawText("L 5 ", 0, 5 );
                if( data_type == 128 ) {
    VDP_drawText("L 6 ", 0, 6 );
                    // cursor update
                    cursor_update_from_pos( &cursor, (s8)buffer[0], (s8)buffer[1], (s8)buffer[2], (s8)buffer[3] );
                }else if( data_type == 129 ) {
    VDP_drawText("L 7 ", 0, 7 );
                    // board update
                    cursor_update_from_pos( &cursor, (s8)buffer[0], (s8)buffer[1], (s8)buffer[2], (s8)buffer[3] );
                    move_piece( (s8)buffer[2], (s8)buffer[3], (s8)buffer[0], (s8)buffer[1] );
                    cursor_clear_selected(&cursor); 
                    if( currentPlayer == PLAYER_ONE ) {
                        currentPlayer = PLAYER_TWO;
                        VDP_drawText("TWO", 20, 1);
                    } else {
                        currentPlayer = PLAYER_ONE;
                        VDP_drawText("ONE", 20, 1);
                    }
                } 
    VDP_drawText("L 8 ", 0, 8 );
            } 
        }

        //////////////////////////////////////////////////////////////
        // update sprites
        SPR_setPosition( cursor.sprite, cursor.pos_x, cursor.pos_y );
        SPR_setPosition( cursor.selected_spr, cursor.sel_pos_x, cursor.sel_pos_y );
        SPR_update();

        //////////////////////////////////////////////////////////////
        // SGDK Do your thing.
        SYS_doVBlankProcess();

    }
    return 0;

}
