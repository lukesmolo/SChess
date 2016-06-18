/*
 * Copyright (C) 2016 Luca Sciullo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#define VIOLET "\x1B[35m"
#define GREEN "\033[0;0;32m"
#define WHITE   "\033[0m"
#define RED "\033[0;0;31m"
#define BLUE "\033[0;0;34m"
#define ORANGE "\033[0;0;33m"
#define CYAN  "\x1B[36m"
#define YELLOW  "\x1B[33m"

#define ROWS 8
#define COLUMNS 8


#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define F 5
#define G 6
#define H 7


static const char* LETTERS[]  = {"A", "B", "C", "D", "E", "F", "G", "H"};

#define WHITE_CHESS 0
#define BLACK_CHESS 1

static const char* CHESS_COLORS[]  = {"WHITE", "BLACK"};

#define CHESS_NUMBER 10
#define CHESS_KNIGHT 8
#define CHESS_KING 9

#define PAWN_N_MOVES 4
#define KNIGHT_N_MOVES 8
#define KING_N_MOVES 8

#define PLUS_INFINITY 100000
#define MINUS_INFINITY (-1)*PLUS_INFINITY
#define NOT_ALLOWED (30)*PLUS_INFINITY
#define EVALUATED_DRAW (20)*PLUS_INFINITY

#define EVAL_FORMULA (int)(2000*(k-k1)+30*(n-n1)+10*(p-p1)-5*(d-d1+s-s1+i-i1)+(m-m1))

#define TOTAL_MOVES_NUMBER KNIGHT_RIGHT_BACKWARD-FORWARD


enum { FORWARD, LEFT_FORWARD, RIGHT_FORWARD, BACKWARD, LEFT_BACKWARD, RIGHT_BACKWARD, LEFT, RIGHT, PAWN_FORWARD, KNIGHT_LEFT_FORWARD, KNIGHT_RIGHT_FORWARD, KNIGHT_LEFT_BACKWARD, KNIGHT_RIGHT_BACKWARD};

enum {
	PAWN, KNIGHT, KING
};

enum {
	PLAYING, WHITE_WIN, BLACK_WIN, DRAW, COLOR_WIN, NOT_COLOR_WIN
};
static const char* RESULTS[]  = {"PLAYING", "WHITE WIN", "BLACK WIN", "DRAW", "COLOR_WIN", "NOT_COLOR_WIN"};

enum {
	NOTHING, CHECK, CHECKMATE, NOT_CHECKMATE, STALEMATE
};

static const char* STATES[]  = {"NOTHING", "CHECK", "CHECKMATE", "NOT_CHECKMATE", "STALEMATE"};


struct piece{
	int n_moves;
	int n_fake_moves;
	int type;
	int color;
	int pos[2];
	int dead;
};

struct fake_piece{
	int pos[2];
	struct piece *p;
};

struct best_move{
	int fake_p_pos[2];
	int move;
	int evaluation;
};

static const char* COLORS[]  = {"WHITE", "BLACK"};
static const char* TOTAL_MOVES_STRINGS[] = { "FORWARD", "LEFT_FORWARD", "RIGHT_FORWARD", "BACKWARD", "LEFT_BACKWARD", "RIGHT_BACKWARD", "LEFT", "RIGHT", "PAWN_FORWARD", "KNIGHT_LEFT_FORWARD", "KNIGHT_RIGHT_FORWARD", "KNIGHT_LEFT_BACKWARD", "KNIGHT_RIGHT_BACKWARD"};

const int PAWN_MOVES[] = { LEFT_FORWARD, FORWARD, RIGHT_FORWARD, PAWN_FORWARD };
static const char* PAWN_MOVES_STRINGS[] = { "LEFT_FORWARD", "FORWARD", "RIGHT_FORWARD", "PAWN_FORWARD" };

const int KNIGHT_MOVES[] = {KNIGHT_LEFT_FORWARD, LEFT_FORWARD, RIGHT_FORWARD, KNIGHT_RIGHT_FORWARD, KNIGHT_RIGHT_BACKWARD, RIGHT_BACKWARD, LEFT_BACKWARD, KNIGHT_LEFT_BACKWARD};
static const char* KNIGHT_MOVES_STRINGS[] = {"KNIGHT_LEFT_FORWARD", "LEFT_FORWARD", "RIGHT_FORWARD", "KNIGHT_RIGHT_FORWARD", "KNIGHT_RIGHT_BACKWARD", "RIGHT_FORWARD", "LEFT_BACKWARD", "KNIGHT_LEFT_BACKWARD"};
const int KING_MOVES[] = {LEFT, LEFT_FORWARD, FORWARD, RIGHT_FORWARD, RIGHT, RIGHT_BACKWARD, BACKWARD, LEFT_BACKWARD};
static const char* KING_MOVES_STRINGS[] = {"LEFT", "LEFT_FORWARD", "FORWARD", "RIGHT_FORWARD", "RIGHT", "RIGHT_BACKWARD", "BACKWARD", "LEFT_BACKWARD"};

const int END[] = {WHITE_WIN, BLACK_WIN, DRAW};
const char* END_STRINGS[] = {"WHITE_WIN", "BLACK_WIN", "DRAW"};

void set_time(int what);
void resume_match(char *match, FILE *report);
void print_board(struct piece *fake_board[][COLUMNS]);
void create_chess();
void initialize_board();
void return_legal_moves(struct piece *fake_board[][COLUMNS], int *all_moves, int *from, int type, int color);
void moves(int *pos, int type, int color, int what);
int is_legal_fake_move(struct piece *fake_board[][COLUMNS], int *from, int *to, int type, int color, int what);
int is_legal_human_move(int *int_input_move);
int is_under_attack(struct piece *fake_board[][COLUMNS], int row, int col, int color);
int return_board_status(struct piece *fake_board[][COLUMNS], int color);
int is_the_end(struct piece *fake_board[][COLUMNS], int color);
void reset_fake_board(struct piece *original_fake_board[][COLUMNS], struct piece *fake_board[][COLUMNS]);
void undo_last_move(struct piece *original_board[][COLUMNS], struct piece *fake_board[][COLUMNS]);
void kill(struct piece *p);
void make_move(struct piece *p, int *to);
void make_fake_move(struct piece *fake_board[][COLUMNS], struct piece *p, int *from, int *to);
void make_random_move(int color, char *input_move);
int evaluate(struct piece *fake_board[][COLUMNS]);
struct best_move* decision_min_max(struct best_move *b_move, struct piece *fake_board[][COLUMNS], int color);
int minmax_value(struct piece *fake_board[][COLUMNS], int color, int MAX, int depth, int *best_depth, int alpha, int beta);
void game(FILE *f);
int main(int argc, char *argv[]);
