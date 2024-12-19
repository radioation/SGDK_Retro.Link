#include <genesis.h>
#include "resources.h"

#define INPUT_WAIT_COUNT 10


// Chess Piece data
// Enum to represent piece types
typedef enum {
    KING = 0, 
    QUEEN = 3, 
    ROOK = 6, 
    BISHOP = 9, 
    KNIGHT = 12, 
    PAWN = 15, 
    EMPTY = 18
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
const u8 boardStartCol = 8;
const u8 boardStartRow = 2;
const u8 boardStep = 3;

void setup_pieces() {
    // clear the board
    CHESS_PIECE empty = {EMPTY, 0};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = empty;
        }
    }

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
                u8 yStart = 0;
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
    char message[40];
    strclr(message);
    sprintf( message, "X: %d y: %d sx: %d sy %d    ", startCol, startRow, endCol, endRow );
    VDP_drawText(message, 0, 27);
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
    s16 col;
    s16 row;
    s16 pos_x;
    s16 pos_y;
    s16 sel_col; // selected piece column
    s16 sel_row; // selected piece row
} CURSOR;

const u8 cursorStep = 24;
const u8 cursorColStart = 64;
const u8 cursorRowStart = 16;

void cursor_init( CURSOR *cursor, Sprite *sprite ) {
    cursor->col = 4;
    cursor->row = 4;
    cursor->pos_x = cursor->col * cursorStep + cursorColStart;
    cursor->pos_y = cursor->row * cursorStep + cursorRowStart;
    cursor->sprite =  sprite;
    cursor->sel_col = -1;
    cursor->sel_row = -1;
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

void cursor_clear( CURSOR* cursor ) {
    cursor->sel_col = -1;
    cursor->sel_row = -1;
    char message[40];
    strclr(message);
    sprintf( message, "X: %d y: %d sx: %d sy %d    ", cursor->col, cursor->row, cursor->sel_col, cursor->sel_row);
    VDP_drawText(message, 0, 0);
    VDP_drawText("CLEARED", 0, 1);
}

void cursor_action( CURSOR* cursor, CHESS_PIECE brd[8][8] ) {
    char message[40];
    strclr(message);
    sprintf( message, "X: %d y: %d sx: %d sy %d    ", cursor->col, cursor->row, cursor->sel_col, cursor->sel_row);
    VDP_drawText(message, 0, 0);

    if( cursor->sel_col < 0 ) {
        // no piece selected yet, check if valid piece in board
        // ( valid means current player owns the piece)
        // TODO --
        if( brd[cursor->col][cursor->row].player > 0 ) { // move anything player for now
            cursor->sel_col = cursor->col;
            cursor->sel_row = cursor->row;
        }
    } else {
        // piece is selected, check if we can move to new space.
        //TODO --

        // move to new spac
        if( brd[cursor->col][cursor->row].type == EMPTY ) { 
            // just check for empty for now
            // clear old player
            VDP_drawText("DO MOVE", 0, 1);
            move_piece( cursor->sel_col, cursor->sel_row, cursor->col, cursor->row );
            cursor_clear(cursor); 
        }
    }
}


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
    cursor_init(&cursor, SPR_addSprite( &cursor_spr, cursor.pos_x, cursor.pos_y, TILE_ATTR(PAL0, FALSE, FALSE, FALSE )) );



    //////////////////////////////////////////////////////////////
    // main loop.
    u8 inputWait = 0;
    while(TRUE)
    {
        // read joypad to mover cursor
        u16 joypad  = JOY_readJoypad( JOY_1 );
        if( inputWait == 0 ) {
            if( cursor_move( &cursor, joypad ) == TRUE ) {
                inputWait = INPUT_WAIT_COUNT;
            }


            if( joypad & BUTTON_A ) {
                cursor_action( &cursor, board );
                inputWait = INPUT_WAIT_COUNT;
            } else if( joypad & BUTTON_C ) {
                cursor_clear( &cursor );
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
