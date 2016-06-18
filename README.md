#SChess: a simplified version of chess for CLI, with AI

####SChess is a simplified version of chess for CLI.

SChess is written in C, and it uses AI implemented by using MinMax algorithm with alpha-beta pruning and fixed depth decided at the beginning of the match.

##Rules
Rules are the same of chess, even if there are only 10 pieces for each player:
* 8 Pawns
* 1 Knight
* 1 King

Pawns are represented by **0**, knights by **1**, kings by **2**, empty cells by **-**.

Starting positions of pieces are the same as in the picture below:

<img src="/pics/initial_board.jpg" width="180">

##Goal
The main goal is to obtain a **pawn promotion** without being eaten by the next
opponent's move, otherwise you can still try to make a **checkmate**.

**Stalemate** case is implemented too, meaning a draw for players.

##Features
* chance to play against AI, choosing which color is AI
* chance to watch a match between two AI players, each one with a different depth
* chance to undo (only) the last move
* chance to restore a match from a file
* knowledge of time used by AI for finding the best move
* storing every match a file automatically created


##Some Numbers

Memory required is less than 100MB.
Using i5 CPU, 3,3 GHz, first white move:
* Depth 3: ~0s, 2774 MinMax calls
* Depth 5: ~1s, 262980 MinMax calls
* Depth 7: ~50s, 16906767 MinMax calls

##Usage
Clone the repository:
```
$ git clone --recursive git@github.com:lukesmolo/SChess.git
```
Compile the program:
```
$ gcc -o SChess SChess.c
```

Run the program:
```
$ ./SChess
```

Run the program resuming a previous match:
```
$ ./SChess 15:19:26,2016-6-18.chess
```

##License
SChess is released under the GPLv2.

##TODO
* take the longest path when AI is going to lose
* keep track in the match file of undone moves
* chance to undo more moves











