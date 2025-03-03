# About
This project is just testing some ideas for handling turn
based games. So while it looks like a chess game, it has 
none of the movment rules or Win-logic implemented. Right 
now the code just establishes the connection and lets players
take turns moving pieces around the board.  As a simple test, 
it assumes everything will be fine when sending and receiving
data over TCP. A real game will need to handle delays and
bad connections.

The use of 128 and 129 as the 'type' is intentional. The 
data I'm sending doesn't need the high bit, so more complex
networking code could use that bit to indicate the start
of a data message.


