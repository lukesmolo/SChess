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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "SChess.h"


/*depth has to be an even value for considering opponent move, since decisionMinMax already makes a move*/
#define WHITE_DEPTH 2
#define BLACK_DEPTH 2

struct piece *board[ROWS][COLUMNS];
struct piece *last_move_board[ROWS][COLUMNS];
struct piece *tmp_last_move_board[ROWS][COLUMNS];
struct piece w_chess[CHESS_NUMBER];
struct piece b_chess[CHESS_NUMBER];

struct piece *last_moved = NULL;
struct timespec start, stop;
int human_moves_counter = 0;

/*variables for different game modes*/
int START_COLOR = WHITE_CHESS;
int HUMAN_PLAYER = 1;
int PRESS_ENTER = 0;


/*take time when move starts and ends*/
void
set_time(int what) {
	long BILLION = 1000000000L;
	int accum;
	/*move starts*/
	if(what == 0) {
		if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
			perror( "clock gettime" );
			exit( EXIT_FAILURE );
		}
	} else {
		/*move ends*/
		if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
			perror( "clock gettime" );
			exit( EXIT_FAILURE );
		}
		accum = ( stop.tv_sec - start.tv_sec )
			+ ( stop.tv_nsec - start.tv_nsec )
			/ BILLION;
		printf( "Move required %ds\n", accum );
	}
}

/*read a file where a previous match has been stored and resume the game*/
void
resume_match(char *match, FILE *report) {
	FILE *f = NULL;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char * split_line;
	char input_move[4];
	int int_input_move[4];
	int allowed_move = 0;
	int pos[2];

	f = fopen(match, "r");
	if (f == NULL) {
		printf("File %s not found\n", match);
		exit(0);
	}

	while ((read = getline(&line, &len, f)) != -1) {

		fprintf(report, "%s", line);
		fflush(report);
		split_line = strtok (line,":\t");
		while (split_line!= NULL)
		{
			if(strlen(split_line ) == 5 && strcmp(split_line, "WHITE") != 0 && strcmp(split_line, "BLACK") != 0) {
				memset(int_input_move, 0, 4);
				strncpy(input_move, split_line, 4);

				printf ("%s\n", input_move);
				int_input_move[0] = input_move[0]%65;
				int_input_move[1] = (int)strtol(&input_move[1], NULL, 10)-1;
				int_input_move[2] = input_move[2]%65;
				int_input_move[3] = (int)strtol(&input_move[3], NULL, 10)-1;
				pos[0] = int_input_move[3];
				pos[1] = int_input_move[2];

				allowed_move = is_legal_human_move(int_input_move);
				if(!allowed_move) {
					printf("Found %s illegal move in the file %s\n", input_move, match);
					exit(0);
				} else {

					make_move(board[int_input_move[1]][int_input_move[0]], pos);
				}
				printf ("%s\n", input_move);
			}
			split_line= strtok (NULL, ":\t");
		}


	}
	fclose(f);

}

/*print the board or the fake board with colors*/
void
print_board(struct piece *fake_board[][COLUMNS]) {
	int i = 0;
	int j = 0;

	if(fake_board == NULL) {
		for(i = 7; i >= 0; i--) {
			printf(YELLOW"%d\t"WHITE, i+1);
			for(j = 0; j < 8; j++) {
				if(board[i][j] == NULL) {
					printf("%s\t", "-");
				} else if(board[i][j]->color == WHITE_CHESS) {
					printf(VIOLET"%d\t"WHITE, board[i][j]->type);
				} else if(board[i][j]->color == BLACK_CHESS) {
					printf(BLUE"%d\t"WHITE, board[i][j]->type);
				}
			}
			printf("\n");

		}
		printf("\n");
		for(j = -1; j < 8; j++) {
			if(j == -1)
				printf("\t");
			else
				printf(YELLOW"%s\t"WHITE, LETTERS[j]);
		}

		printf("\n");
	} else {
		for(i = 7; i >= 0; i--) {
			for(j = 0; j < 8; j++) {
				if(fake_board[i][j] == NULL) {
					printf("%s\t", "-");
				} else if(fake_board[i][j]->color == WHITE_CHESS) {
					printf(VIOLET"%d\t"WHITE, fake_board[i][j]->type);
				} else if(fake_board[i][j]->color == BLACK_CHESS) {
					printf(BLUE"%d\t"WHITE, fake_board[i][j]->type);
				}
			}
			printf("\n");

		}

	}

	fflush(stdout);	

}

/*create chess*/
void
create_chess() {
	int i = 0;

	for(i = 0; i < CHESS_NUMBER; i++) {
		w_chess[i].color = WHITE_CHESS;
		w_chess[i].n_moves = 0;
		w_chess[i].n_fake_moves = 0;
		w_chess[i].type = PAWN;
		w_chess[i].pos[0] = -1;
		w_chess[i].pos[1] = -1;
		w_chess[i].dead = 0;
		b_chess[i].color = BLACK_CHESS;
		b_chess[i].n_moves = 0;
		b_chess[i].n_fake_moves = 0;
		b_chess[i].type = PAWN;
		b_chess[i].pos[0] = -1;
		b_chess[i].pos[1] = -1;
		b_chess[i].dead = 0;
	}
	w_chess[CHESS_KNIGHT].type = KNIGHT;
	b_chess[CHESS_KNIGHT].type = KNIGHT;
	w_chess[CHESS_KING].type = KING;
	b_chess[CHESS_KING].type = KING;

}


/*set the initial positions of all pieces*/
void
initialize_board() {

	int i = 0;
	int j = 0;

	for(i = 0; i < ROWS; i++) {
		for(j = 0; j < COLUMNS; j++) {
			board[i][j] = NULL;
		}
	}

	for(i = 0; i < 10; i++) {
		w_chess[i].dead = 1;
		b_chess[i].dead = 1;

	}
	for(i = 0; i < 8; i++) {
		w_chess[i].pos[0] = 1;
		w_chess[i].pos[1] = i;
		w_chess[i].dead = 0;
		b_chess[i].pos[0] = 6;
		b_chess[i].pos[1] = i;
		b_chess[i].dead = 0;
		board[1][i] = &(w_chess[i]);
		board[6][i] = &(b_chess[i]);
	}


	w_chess[CHESS_KING].pos[0] = 0;
	w_chess[CHESS_KING].pos[1] = E;
	w_chess[CHESS_KING].dead = 0;
	b_chess[CHESS_KING].pos[0] = 7;
	b_chess[CHESS_KING].pos[1] = E;
	b_chess[CHESS_KING].dead = 0;
	w_chess[CHESS_KNIGHT].pos[0] = 0;
	w_chess[CHESS_KNIGHT].pos[1] = G;
	w_chess[CHESS_KNIGHT].dead = 0;
	b_chess[CHESS_KNIGHT].pos[0] = 7;
	b_chess[CHESS_KNIGHT].pos[1] = G;
	b_chess[CHESS_KNIGHT].dead = 0;

	board[0][E] = &(w_chess[CHESS_KING]);
	board[7][E] = &(b_chess[CHESS_KING]);
	board[0][G] = &(w_chess[CHESS_KNIGHT]);
	board[7][G] = &(b_chess[CHESS_KNIGHT]);
}


/*return legal moves for a piece basing on its color, its starting position and its type*/
void
return_legal_moves(struct piece *fake_board[][COLUMNS], int *all_moves, int *from, int type, int color){
	const int *moves_array;
	int move_max = -1;
	int move = -1;
	int tmp_pos[2];
	memset(all_moves, -1, sizeof(int)*TOTAL_MOVES_NUMBER);


	/*set the pointer to the right array of moves in the header*/
	switch(type) {
		case PAWN:
			moves_array = &(PAWN_MOVES[0]);
			move_max = PAWN_N_MOVES;
			break;
		case KNIGHT:
			moves_array = &(KNIGHT_MOVES[0]);
			move_max = KNIGHT_N_MOVES;
			break;
		case KING:
			moves_array = &(KING_MOVES[0]);
			move_max = KING_N_MOVES;
			break;
		default:
			break;
	}
	for(move = 0; move < move_max; move++) {
		tmp_pos[0] = from[0];
		tmp_pos[1] = from[1];
		moves(tmp_pos, type, color, *(moves_array+move));

		/*for each move just check if it is a legal one*/
		if(is_legal_fake_move(fake_board, from, tmp_pos, type, color, *(moves_array+move))) {
			all_moves[move] = *(moves_array+move);
		}

	}
}

/*put coordinates of move requested in pos*/
void
moves(int *pos, int type, int color, int what) {

	switch(type){
		case PAWN:
			switch(what) {
				case FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					break;
				case PAWN_FORWARD:
					pos[0] = pos[0]-4*(color)+2;
					break;
				case LEFT_FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					pos[1] = pos[1]+2*(color)-1;
					break;
				case RIGHT_FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					pos[1] = pos[1]-2*(color)+1;
					break;
				default:
					break;
			}
			break;
		case KNIGHT:
			switch(what) {
				case LEFT_FORWARD:
					pos[0] = pos[0]-4*(color)+2;
					pos[1] = pos[1]+2*(color)-1;
					break;
				case RIGHT_FORWARD:
					pos[0] = pos[0]-4*(color)+2;
					pos[1] = pos[1]-2*(color)+1;
					break;
				case LEFT_BACKWARD:
					pos[0] = pos[0]+4*(color)-2;
					pos[1] = pos[1]+2*(color)-1;
					break;
				case RIGHT_BACKWARD:
					pos[0] = pos[0]+4*(color)-2;
					pos[1] = pos[1]-2*(color)+1;
					break;
				case KNIGHT_LEFT_FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					pos[1] = pos[1]+4*(color)-2;
					break;
				case KNIGHT_RIGHT_FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					pos[1] = pos[1]-4*(color)+2;
					break;
				case KNIGHT_LEFT_BACKWARD:
					pos[0] = pos[0]+2*(color)-1;
					pos[1] = pos[1]+4*(color)-2;
					break;
				case KNIGHT_RIGHT_BACKWARD:
					pos[0] = pos[0]+2*(color)-1;
					pos[1] = pos[1]-4*(color)+2;
					break;
				default:
					break;
			}
			break;
		case KING:
			switch(what) {
				case LEFT:
					pos[1] = pos[1]+2*(color)-1;
					break;
				case RIGHT:
					pos[1] = pos[1]-2*(color)+1;
					break;
				case FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					break;
				case BACKWARD:
					pos[0] = pos[0]+2*(color)-1;
					break;
				case LEFT_FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					pos[1] = pos[1]+2*(color)-1;
					break;
				case RIGHT_FORWARD:
					pos[0] = pos[0]-2*(color)+1;
					pos[1] = pos[1]-2*(color)+1;
					break;
				case LEFT_BACKWARD:
					pos[0] = pos[0]+2*(color)-1;
					pos[1] = pos[1]+2*(color)-1;
					break;
				case RIGHT_BACKWARD:
					pos[0] = pos[0]+2*(color)-1;
					pos[1] = pos[1]-2*(color)+1;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

/*check if a move is legal, basing on the position of a piece, its type and its color*/
int
is_legal_fake_move(struct piece *fake_board[][COLUMNS], int *from, int *to, int type, int color, int what) {
	int ret = 1;
	int row = to[0];
	int col = to[1];
	int tmp_row = -1;

	/*for all moves check if they are inside the board and if destination is free or containing an opponent piece to be eaten*/
	if(to[0] > ROWS-1 || to[1] > COLUMNS-1 || to[0] < 0 || to[1] < 0) {
		ret = 0;
	} else if(to[0] == from[0] && to[1] == from[1]) {
		ret = 0; //no real move
	} else {
		switch(type) {
			case PAWN:

				switch(what) {
					case FORWARD:
						if(fake_board[row][col] != NULL)
							ret = 0;
						break;
					case PAWN_FORWARD:
						/*check if pawn starts moving from its original position*/
						if(fake_board[row][col] != NULL  || from[0] != color*5+1) {
							ret = 0;
							/*check if there is a piece on the next row*/
						} else if(fake_board[from[0]-2*color+1][from[1]] != NULL ) {
							ret = 0;
						}
						break;
					case LEFT_FORWARD:
					case RIGHT_FORWARD:
						tmp_row = row+2*color-1;
						/*check if the move is an en passant capture*/	
						if((fake_board[row][col] == NULL  || fake_board[row][col]->color == color) && (tmp_row < 0 || tmp_row > ROWS-1 || row != (-2*color+5) || fake_board[tmp_row][col] == NULL || fake_board[tmp_row][col]->color == color || fake_board[tmp_row][col]->type != PAWN || fake_board[tmp_row][col]->n_fake_moves != 1|| last_moved != fake_board[tmp_row][col])) {
							ret = 0;
						}
						break;
					default:
						break;
				}

				break;
			case KNIGHT:
			case KING:
				if((fake_board[row][col] != NULL && fake_board[row][col]->color == color)/* || is_under_attack(fake_board, row, col, color)*/) {
					ret = 0;
				}
				break;

			default:
				break;
		}
	}


	return ret;
}

int
is_legal_human_move(int *int_input_move) {
	int allowed_move = 0;
	int from[2];
	int to[2];
	int tmp_to[2];
	int i = 0;
	int type;
	int color;
	int legal_moves[TOTAL_MOVES_NUMBER];
	from[0] = int_input_move[1];
	from[1] = int_input_move[0];
	to[0] = int_input_move[3];
	to[1] = int_input_move[2];
	if(int_input_move[0] < 0 || int_input_move[0] > COLUMNS-1 || int_input_move[1] < 0 || int_input_move[1] > ROWS-1 || board[int_input_move[1]][int_input_move[0]] == NULL) {
		return 0;
	}
	else if(int_input_move[2] < 0 || int_input_move[2] > COLUMNS-1 || int_input_move[3] < 0 || int_input_move[3] > ROWS-1) {
		return 0;

	}

	type = board[from[0]][from[1]]->type;
	color = board[from[0]][from[1]]->color;
	return_legal_moves(board, legal_moves, from, type, color);
	for(i = 0; i < TOTAL_MOVES_NUMBER; i++) {
		tmp_to[0] = from[0];
		tmp_to[1] = from[1];
		if(legal_moves[i] != -1) {
			moves(tmp_to, type , color, legal_moves[i]);
			if(to[0] == tmp_to[0] && to[1] == tmp_to[1]) {
				allowed_move = 1;
				break;
			}
		}
	}
	return allowed_move;
}



/*return if a piece is under attack*/
int
is_under_attack(struct piece *fake_board[][COLUMNS], int row, int col, int color) {
	int move;
	int ret = 0;
	int pos[2];
	struct piece *tmp_fake_piece = NULL;
	const int *moves_array;
	const int tmp_pawn_moves[] = {LEFT_FORWARD, RIGHT_FORWARD};


	/*pawn's attacks*/
	moves_array = &(tmp_pawn_moves[0]);
	for(move = 0; move < 2; move++) {
		pos[0] = row;
		pos[1] = col;
		moves(pos, PAWN, color, *(moves_array+move));
		if(pos[0] < ROWS && pos[0] >= 0 && pos[1] < COLUMNS && pos[1] >= 0) {
			if(fake_board[pos[0]][pos[1]] != NULL) {
				tmp_fake_piece = fake_board[pos[0]][pos[1]];
				if(tmp_fake_piece->type == PAWN && tmp_fake_piece->color == !color)
					return 1;


			}
		}

	}


	/*knight's attacks*/
	moves_array = &(KNIGHT_MOVES[0]);
	for(move = 0; move < KNIGHT_N_MOVES; move++) {
		pos[0] = row;
		pos[1] = col;
		moves(pos, KNIGHT, color, *(moves_array+move));
		if(pos[0] < ROWS && pos[0] >= 0 && pos[1] < COLUMNS && pos[1] >= 0) {
			if(fake_board[pos[0]][pos[1]] != NULL) {
				tmp_fake_piece = fake_board[pos[0]][pos[1]];
				if(tmp_fake_piece->type == KNIGHT && tmp_fake_piece->color == !color)
					return 1;
			}
		}
	}

	/*king's attacks*/
	moves_array = &(KING_MOVES[0]);
	for(move = 0; move < KING_N_MOVES; move++) {
		pos[0] = row;
		pos[1] = col;
		moves(pos, KING, color, *(moves_array+move));
		if(pos[0] < ROWS && pos[0] >= 0 && pos[1] < COLUMNS && pos[1] >= 0) {
			if(fake_board[pos[0]][pos[1]] != NULL) {
				tmp_fake_piece = fake_board[pos[0]][pos[1]];
				if(tmp_fake_piece->type == KING && tmp_fake_piece->color == !color)
					return 1;
			}
		}

	}


	return ret;


}

/*return in which particular state the board is*/
int
return_board_status(struct piece *fake_board[][COLUMNS], int color) {
	int check = 0;
	int ret = 1;
	int i, j;
	struct piece *tmp_fake_board[ROWS][COLUMNS];
	struct piece *tmp_fake_piece = NULL;
	struct piece *not_tmp_fake_piece = NULL;
	int legal_moves[TOTAL_MOVES_NUMBER];
	int from[2];
	int not_from[2];
	int to[2];
	reset_fake_board(fake_board, tmp_fake_board);
	/*find kings*/
	for(i = 0; i < ROWS; i++) {
		if(tmp_fake_piece != NULL && not_tmp_fake_piece != NULL)
			break;
		for(j = 0; j < COLUMNS; j++) {
			if(tmp_fake_board[i][j] != NULL && tmp_fake_board[i][j]->color == !color && tmp_fake_board[i][j]->type == KING) {
				not_tmp_fake_piece = tmp_fake_board[i][j];
				not_from[0] = i;
				not_from[1] = j;
			}
			if(tmp_fake_board[i][j] != NULL && tmp_fake_board[i][j]->color == color && tmp_fake_board[i][j]->type == KING) {
				tmp_fake_piece = tmp_fake_board[i][j];
				from[0] = i;
				from[1] = j;
			}

		}
	}


	if(not_tmp_fake_piece == NULL || tmp_fake_piece == NULL) {
		return NOTHING;
	}
	/*check if opponent king is under check*/
	if(is_under_attack(tmp_fake_board, not_from[0], not_from[1], !color))
		check = 1;


	/*check if opponent king is under chackmate*/
	if(check) {
		return_legal_moves(tmp_fake_board, legal_moves, not_from, not_tmp_fake_piece->type, not_tmp_fake_piece->color);
		for(i = 0; i < TOTAL_MOVES_NUMBER; i++) {

			reset_fake_board(fake_board, tmp_fake_board);
			if(legal_moves[i] != -1) {
				to[0] = not_from[0];
				to[1] = not_from[1];

				moves(to, not_tmp_fake_piece->type, not_tmp_fake_piece->color, legal_moves[i]);
				make_fake_move(tmp_fake_board, not_tmp_fake_piece, not_from, to);
				if(!is_under_attack(tmp_fake_board, to[0], to[1], !color) ) {
					ret = 0;
					break;
				}

			}
		}
	} else {
		ret = 0;
	}

	reset_fake_board(fake_board, tmp_fake_board);
	if(ret == 1 && check) {
		ret = CHECKMATE;

	} else if(check) {
		return CHECK;
	} else {
		check = 0;
		ret = 1;
		/*check if king is under chackmate*/
		if(is_under_attack(tmp_fake_board, from[0], from[1], color)) {
			check = 1;
		}

		/*check if there is a stalemate situation*/
		return_legal_moves(tmp_fake_board, legal_moves, from, tmp_fake_piece->type, tmp_fake_piece->color);
		for(i = 0; i < TOTAL_MOVES_NUMBER; i++) {

			//reset_fake_board(tmp_fake_board, fake_board);
			reset_fake_board(fake_board, tmp_fake_board);
			if(legal_moves[i] != -1) {
				to[0] = from[0];
				to[1] = from[1];

				moves(to, tmp_fake_piece->type, tmp_fake_piece->color, legal_moves[i]);
				make_fake_move(tmp_fake_board, tmp_fake_piece, from, to);
				if(!is_under_attack(tmp_fake_board, to[0], to[1], color) ) {
					ret = 0;
					break;
				}

			}
		}
		if(ret == 1 && !check) {
			ret = STALEMATE;
		} else if(ret == 1 && check) {
			ret = NOT_CHECKMATE;
		} else {
			ret = NOTHING;
		}


	}


	return ret;

}

/*basing on what return_board_status returns, return a value meaning the end of the game*/
int
is_the_end(struct piece *fake_board[][COLUMNS], int color) {
	int ret = PLAYING;
	int tmp = -1;
	int i = 0;

	/*fake_board case*/
	if(fake_board != NULL) {

		tmp = return_board_status(fake_board, color);
		if(tmp == STALEMATE) {
			ret = DRAW;

		} else if(tmp == CHECKMATE) {
			if(color == START_COLOR)
				ret = COLOR_WIN;
			else
				ret = NOT_COLOR_WIN;

		} else if(tmp == NOT_CHECKMATE) {
			if(color != START_COLOR)
				ret = COLOR_WIN;
			else
				ret = NOT_COLOR_WIN;

		} else {

			for(i = 0; i < COLUMNS; i++) {
				/*white promotion case*/
				if(fake_board[ROWS-1][i] != NULL && fake_board[ROWS-1][i]->type == PAWN && fake_board[ROWS-1][i]->color == WHITE_CHESS && !is_under_attack(fake_board, ROWS-1, i, WHITE_CHESS)) {
					if(START_COLOR == WHITE_CHESS)
						ret = COLOR_WIN;
					else
						ret = NOT_COLOR_WIN;


					break;

				}
				/*black promotion case*/
				if(fake_board[0][i] != NULL && fake_board[0][i]->type == PAWN && fake_board[0][i]->color == BLACK_CHESS && !is_under_attack(fake_board, 0, i, BLACK_CHESS)) {
					if(START_COLOR == BLACK_CHESS)
						ret = COLOR_WIN;
					else
						ret = NOT_COLOR_WIN;

					break;
				}
			}
		}


		/*board case*/
	} else {
		tmp = return_board_status(board, color);
		if(tmp == CHECK) {
			printf(RED"CHECK\n"WHITE);
		} else if(tmp == STALEMATE) {
			printf(RED"STALEMATE\n"WHITE);
			ret = DRAW;
		} else if(tmp == CHECKMATE) {

			printf(RED"CHECKMATE\n"WHITE);
			if(color == START_COLOR)
				ret = COLOR_WIN;
			else
				ret = NOT_COLOR_WIN;

		} else if(tmp == NOT_CHECKMATE) {

			printf(RED"CHECKMATE\n"WHITE);
			if(color == START_COLOR)
				ret = NOT_COLOR_WIN;
			else
				ret = COLOR_WIN;


		} else {

			for(i = 0; i < COLUMNS; i++) {
				/*white promotion case*/
				if(board[ROWS-1][i] != NULL && board[ROWS-1][i]->type == PAWN && board[ROWS-1][i]->color == WHITE_CHESS && !is_under_attack(board, ROWS-1, i, WHITE_CHESS)) {
					if(START_COLOR == WHITE_CHESS)
						ret = COLOR_WIN;
					else
						ret = NOT_COLOR_WIN;

					break;

				}
				/*black promotion case*/
				if(board[0][i] != NULL && board[0][i]->type == PAWN && board[0][i]->color == BLACK_CHESS && !is_under_attack(board, 0, i, BLACK_CHESS)) {
					if(START_COLOR == BLACK_CHESS)
						ret = COLOR_WIN;
					else
						ret = NOT_COLOR_WIN;

					break;
				}
			}
			/*case ends detected by evaluate function*/
			if(ret == PLAYING) {
				tmp = evaluate(board);
				if(tmp == EVALUATED_DRAW)
					ret = DRAW;
				else if(tmp == PLUS_INFINITY) {
					if(color == START_COLOR)
						ret = COLOR_WIN;
					else
						ret = NOT_COLOR_WIN;

				} else if(tmp == MINUS_INFINITY) {
					if(color == START_COLOR)
						ret = NOT_COLOR_WIN;
					else
						ret = COLOR_WIN;

				}
			}
		}


	}

	return ret;

}

/*reset the fake board*/
void
reset_fake_board(struct piece *original_fake_board[][COLUMNS], struct piece *fake_board[][COLUMNS]) {
	memcpy(fake_board, original_fake_board, sizeof(struct piece*)*ROWS*COLUMNS);
}

void
undo_last_move(struct piece *original_board[][COLUMNS], struct piece *fake_board[][COLUMNS]) {
	int i = 0;
	int j = 0;
	int color;

	for(i = 0; i < ROWS; i++) {
		for(j = 0; j < COLUMNS; j++) {

			if(original_board[i][j] == NULL) {
				fake_board[i][j] = NULL;
			} else {
				color = original_board[i][j]->color;
				fake_board[i][j] = original_board[i][j];
				fake_board[i][j]->dead = 0;
				fake_board[i][j]->pos[0]= i;
				fake_board[i][j]->pos[1]= j;
				/*if the piece was at the beginning, reset its number of moves*/
				if(i == 5*color+1) {
					fake_board[i][j]->n_moves = 0;
					fake_board[i][j]->n_fake_moves = 0;
				}
			}
		}
	}
}

/*kill a piece on the board*/
void
kill(struct piece *p) {

	board[p->pos[0]][p->pos[1]] = NULL;
	p->dead = 1;
	p->pos[0] = -1;
	p->pos[1] = -1;
}

/*make a real move on the board*/
void
make_move(struct piece *p, int *to){

	int x_old_pos = p->pos[0];
	int y_old_pos = p->pos[1];
	int color = p->color;

	/*update last_move_board with the previous board accepted*/
	if(color != START_COLOR && HUMAN_PLAYER) {
		reset_fake_board(tmp_last_move_board, last_move_board);
	}

	if(board[to[0]][to[1]] != NULL) {
		kill(board[to[0]][to[1]]);

		/*detect en passant captures*/
	} else if(p->type == PAWN && board[to[0]+2*color-1][to[1]] != NULL && to[1]-2*color+1 == y_old_pos && to[0] == x_old_pos-2*color+1) { /*LEFT en passant*/
		kill(board[to[0]+2*color-1][to[1]]);
	}  else if(p->type == PAWN && board[to[0]+2*color-1][to[1]] != NULL && to[1]+2*color-1 == y_old_pos && to[0] == x_old_pos-2*color+1) {/*RIGHT en passant*/
		kill(board[to[0]+2*color-1][to[1]]);
	}

	board[x_old_pos][y_old_pos] = NULL;


	/*set p as last piece moved*/
	last_moved = p;
	p->pos[0] = to[0];
	p->pos[1] = to[1];
	p->n_moves++;
	p->n_fake_moves = p->n_moves;
	board[to[0]][to[1]] = p;
	/*store board for being restored*/
	if(color == START_COLOR && HUMAN_PLAYER) {
		reset_fake_board(board, tmp_last_move_board);
	}

}

/*make a fake move on a fake board, i.e., simulate a move*/
void
make_fake_move(struct piece *fake_board[][COLUMNS], struct piece *p, int *from, int *to){
	int x_old_pos = from[0];
	int y_old_pos = from[1];
	int color = p->color;
	if(fake_board[to[0]][to[1]] != NULL) {

		fake_board[to[0]][to[1]] = NULL;

		/*detect en passant captures*/
	} else if(p->type == PAWN && fake_board[to[0]+2*color-1][to[1]] != NULL  && to[1]-2*color+1 == from[1] && to[0] == from[0]-2*color+1) { /*LEFT en passant*/
		fake_board[to[0]+2*color-1][to[1]] = NULL;

	} else if(p->type == PAWN && fake_board[to[0]+2*color-1][to[1]] != NULL  && to[1]+2*color-1 == from[1] && to[0] == from[0]-2*color+1) { /*RIGHT en passant*/
		fake_board[to[0]+2*color-1][to[1]] = NULL;
	}

	/*set p as last piece moved*/
	last_moved = p;
	fake_board[x_old_pos][y_old_pos] = NULL;
	fake_board[to[0]][to[1]] = p;
	p->n_fake_moves++;

}

void
make_random_move(int color, char *input_move) {

	printf("Random move\n");
	int index_p = rand()%10;
	struct piece *p = NULL;
	int pos[2];
	int move;
	const int *moves_array = NULL;
	do {
		do{
			index_p = rand()%CHESS_NUMBER;
			if(color == WHITE_CHESS) {
				p = &(w_chess[index_p]);	
			} else {
				p = &(b_chess[index_p]);	
			}
		} while(p->dead == 1); /*it cannot take dead pieces*/


		switch(p->type) {
			case PAWN:
				move = rand()%PAWN_N_MOVES;
				moves_array = &(PAWN_MOVES[0]);
				//printf("move from array: %s\n", PAWN_MOVES_STRINGS[move]);
				break;
			case KNIGHT:
				move = rand()%KNIGHT_N_MOVES;
				moves_array = &(KNIGHT_MOVES[0]);
				//printf("move from array: %s\n", KNIGHT_MOVES_STRINGS[move]);
				break;
			case KING:
				move = rand()%KING_N_MOVES;
				moves_array = &(KING_MOVES[0]);
				//printf("move from array: %s\n", KING_MOVES_STRINGS[move]);
				break;
			default:
				break;
		}
		pos[0] = p->pos[0];
		pos[1] = p->pos[1];
		input_move[0] = pos[1]+65;
		sprintf(&input_move[1], "%d", pos[0]+1);
		moves(pos, p->type, p->color, *(moves_array+move));
		input_move[2] = pos[1]+65;
		sprintf(&input_move[3], "%d", pos[0]+1);

	} while(!is_legal_fake_move(board, p->pos, pos, p->type, p->color, *(moves_array+move)));

	make_move(p, pos);


}


/*make an evaluation of the fake board basing on EVAL formula in the header*/
int
evaluate(struct piece *fake_board[][COLUMNS]) {
	int k, k1, n, n1, p, p1, d, d1, s, s1, i, i1, m, m1;
	int tmp[COLUMNS];
	int tmp1[COLUMNS];
	int legal_moves[TOTAL_MOVES_NUMBER];


	k = 0; k1 = 0; /*king*/
	n = 0; n1 = 0; /*knights*/
	p = 0; p1 = 0; /*pawns*/
	d = 0; d1 = 0; /*doubled pawns*/
	s = 0; s1 = 0; /*backward pawns*/
	i = 0; i1 = 0; /*isolated pawns*/
	m = 0; m1 = 0; /*mobility*/
	int col = 0;
	int t = 0;
	int row = 0;
	int move = -1;
	int move_max = -1;
	int res = NOT_ALLOWED;
	struct piece *tmp_fake_piece = NULL;
	int tmp_n_paws = -1;
	int is_there_pawn = -1;
	int from[2];
	memset(tmp, 0, sizeof(int)*COLUMNS);
	memset(tmp1, 0, sizeof(int)*COLUMNS);

	/*for every piece of the board ..*/
	for(row = 0; row < ROWS; row++) {
		for(col = 0; col < COLUMNS; col++) {

			if(fake_board[row][col] != NULL) {
				from[0] = row;
				from[1] = col;
				move_max = -1;
				tmp_fake_piece = fake_board[row][col];

				return_legal_moves(fake_board, legal_moves, from, tmp_fake_piece->type, tmp_fake_piece->color);
				switch(tmp_fake_piece->color) {
					case WHITE_CHESS:
						switch(tmp_fake_piece->type) {
							case PAWN:
								p++;
								tmp[col]++;
								if(row > 1) { /*if pawn's row is greater than 2..*/
									is_there_pawn = 0;
									tmp_n_paws = -1;
									for(t = 1; t < ROWS-2; t++) {


										if(col > 0 && col < COLUMNS-1) { /*if col is neither the first or the last..*/
											if((fake_board[t][col-1] != NULL &&  fake_board[t][col-1]->type == PAWN && fake_board[t][col-1]->color == WHITE_CHESS) || (fake_board[t][col+1] != NULL  &&  fake_board[t][col+1]->type == PAWN && fake_board[t][col+1]->color == WHITE_CHESS)) {
												is_there_pawn = 1;
											}

										} else if(col == 0) { /*if it is in the first col..*/
											if(fake_board[t][col+1] != NULL  &&  fake_board[t][col+1]->type == PAWN && fake_board[t][col+1]->color == WHITE_CHESS) {
												is_there_pawn = 1;
											}

										} else if(col == COLUMNS-1) { /*if it is in the last col..*/
											if(fake_board[t][col-1] != NULL  &&  fake_board[t][col-1]->type == PAWN && fake_board[t][col-1]->color == WHITE_CHESS) {
												is_there_pawn = 1;

											}
										}
										if(is_there_pawn && t <= row) {
											tmp_n_paws = -1;
											break;
										} else if(t > row && is_there_pawn && tmp_n_paws == -1) {
											tmp_n_paws = 0;
											break;
										} else if(t == ROWS-3 && tmp_n_paws == -1){
											tmp_n_paws = 1;
										}
									}
									if(tmp_n_paws == 0) {
										s++; /*backward pawn*/
									} else if(tmp_n_paws == 1) {
										i++; /*isolated pawn*/
									}
								}
								move_max = 4;


								break;
							case KNIGHT:
								n++;
								move_max = 8;
								break;
							case KING:
								k++;
								move_max = 8;
								break;

							default:
								break;
						}


						for(move = 0; move < move_max; move++) {
							if(legal_moves[move] > -1){
								m++;
							}
						}
						break;
					case BLACK_CHESS:	
						switch(tmp_fake_piece->type) {
							case PAWN:
								p1++;
								tmp1[col]++;
								if(row < ROWS-2) { /*if pawn's row is less than 6..*/
									is_there_pawn = 0;
									tmp_n_paws = -1;

									for(t = ROWS-2; t > 0; t--) {
										if(col > 0 && col < COLUMNS-1) { /*if col is not neither the first or the last..*/
											if((fake_board[t][col-1] != NULL &&  fake_board[t][col-1]->type == PAWN && fake_board[t][col-1]->color == BLACK_CHESS) || (fake_board[t][col+1] != NULL  &&  fake_board[t][col+1]->type == PAWN && fake_board[t][col+1]->color == BLACK_CHESS)) {
												is_there_pawn = 1;
											}

										} else if(col == 0) { /*if it is in the first col..*/
											if(fake_board[t][col+1] != NULL  &&  fake_board[t][col+1]->type == PAWN && fake_board[t][col+1]->color == BLACK_CHESS) {
												is_there_pawn = 1;
											}

										} else if(col == COLUMNS-1) { /*if it is in the last col..*/
											if(fake_board[t][col-1] != NULL  &&  fake_board[t][col-1]->type == PAWN && fake_board[t][col-1]->color == BLACK_CHESS) {
												is_there_pawn = 1;

											}
										}
										if(is_there_pawn && t >= row) {
											tmp_n_paws = -1;
											break;
										} else if(t < row && is_there_pawn && tmp_n_paws == -1) {
											tmp_n_paws = 0;
											break;
										} else if(t == 1 && tmp_n_paws == -1){
											tmp_n_paws = 1;
										}
									}

									if(tmp_n_paws == 0) {
										s1++; /*backward pawn*/
									} else if(tmp_n_paws == 1) {
										i1++; /*isolated pawn*/
									}
								}
								move_max = 4;
								break;
							case KNIGHT:
								move_max = 8;

								n1++;
								break;
							case KING:
								k1++;
								move_max = 8;

								break;

							default:
								break;
						}
						for(move = 0; move < move_max; move++) {
							if(legal_moves[move] > -1){
								m1++;
							}
						}
						break;
					default:
						break;
				}
			}
		}
	}
	for(col = 0; col < COLUMNS; col++) {
		if(tmp[col] > 1) {
			d += tmp[col];
		}
		if(tmp1[col] > 1) {
			d1 += tmp1[col];
		}
	}
	/*consider king's death as defeat*/
	if((k == 0 && n == 0 && p == 0) || k == 0) {
		res = MINUS_INFINITY;
	} else if((k1 == 0 && n1 == 0 && p1 == 0) || k1 == 0) {
		res = PLUS_INFINITY;
		/*stalemate case*/
	} else if(n == 0 && p == 0 && k != 0 && n1 == 0 && p1 == 0 && k1 != 0) {
		res = EVALUATED_DRAW;
	} else {
		res = EVAL_FORMULA;
	}
	/*if black is max, consider negative evaluation*/
	if(START_COLOR == WHITE_CHESS) {
		return res;
	} else {
		return (-1)*res;
	}

}



/*start taking a decision, before calling minmax. Actually, the first step of minmax.
  Return the best move*/
struct best_move*
decision_min_max(struct best_move *b_move, struct piece *fake_board[][COLUMNS], int color) {

	int i,j;
	int move;
	int rand_val = -1;
	int alpha = MINUS_INFINITY;
	int beta = PLUS_INFINITY;
	int fake_p_pos[2];
	int legal_moves[TOTAL_MOVES_NUMBER];
	int res = MINUS_INFINITY;
	int max = MINUS_INFINITY-1;
	struct piece *tmp_fake_board[ROWS][COLUMNS];
	struct piece *tmp_fake_piece;
	int to[2];
	struct piece *tmp_last_moved = last_moved;
	int from[2];
	int best_depth = PLUS_INFINITY;
	int tmp_best_depth = best_depth;
	reset_fake_board(fake_board, tmp_fake_board);
	fake_p_pos[0] = -1;
	fake_p_pos[1] = -1;
	b_move->evaluation = NOT_ALLOWED;
	int MAX = 1;

	/*for each possible move, try to emulate it and return the its evaluation. Then take the best one*/
	for(i = 0; i < ROWS; i++) {
		for(j = 0; j < COLUMNS; j++) {

			reset_fake_board(fake_board, tmp_fake_board);

			if(tmp_fake_board[i][j] != NULL && tmp_fake_board[i][j]->color == color) { /*if there is a piece that can be moved*/
				from[0] = i;
				from[1] = j;

				tmp_fake_piece = tmp_fake_board[i][j];
				last_moved = tmp_last_moved;

				return_legal_moves(tmp_fake_board, legal_moves, from, tmp_fake_piece->type, tmp_fake_piece->color);

				for(move = 0; move < TOTAL_MOVES_NUMBER; move++) {

					best_depth = PLUS_INFINITY;
					if(legal_moves[move] > -1) {

						reset_fake_board(fake_board, tmp_fake_board);
						last_moved = tmp_last_moved;
						tmp_fake_piece = tmp_fake_board[i][j];
						to[0] = i;
						to[1] = j;
						fake_p_pos[0] = i;
						fake_p_pos[1] = j;
						if(tmp_fake_piece->type == PAWN)
							tmp_fake_piece->n_fake_moves = tmp_fake_piece->n_moves;

						moves(to, tmp_fake_piece->type, tmp_fake_piece->color, legal_moves[move]);
						make_fake_move(tmp_fake_board, tmp_fake_piece, from, to);

						/*call minmax with different depths according to the color*/
						if(color == WHITE_CHESS)
							res = minmax_value(tmp_fake_board, !color, !MAX, WHITE_DEPTH, &best_depth, alpha, beta);
						else

							res = minmax_value(tmp_fake_board, !color, !MAX, BLACK_DEPTH, &best_depth, alpha, beta);

						last_moved = tmp_last_moved;
						/*the first best depth, i.e. winning depth, is taken*/
						if(res == PLUS_INFINITY && tmp_best_depth == PLUS_INFINITY) {
							tmp_best_depth = best_depth;

						}

						/*the first accetable result is taken*/
						if(max == (MINUS_INFINITY-1) && res != (MINUS_INFINITY-1)) {
							max = res;
							b_move->move = legal_moves[move];
							b_move->fake_p_pos[0] = fake_p_pos[0];
							b_move->fake_p_pos[1] = fake_p_pos[1];
							b_move->evaluation = max;

							/*take a better move*/
						} else if(res > max && res != NOT_ALLOWED) {

							max = res;
							b_move->move = legal_moves[move];
							b_move->fake_p_pos[0] = fake_p_pos[0];
							b_move->fake_p_pos[1] = fake_p_pos[1];
							b_move->evaluation = max;
						} else if(res == max) {
							if(res == PLUS_INFINITY && tmp_best_depth > best_depth) { /*case better final move found*/

								b_move->move = legal_moves[move];
								b_move->fake_p_pos[0] = fake_p_pos[0];
								b_move->fake_p_pos[1] = fake_p_pos[1];
								b_move->evaluation = max;
								tmp_best_depth = best_depth;
							}

							else { /*case better not final state found*/
								/*take this move with some probability*/
								rand_val = rand()%2;
								if(rand_val) {
									b_move->move = legal_moves[move];
									b_move->fake_p_pos[0] = fake_p_pos[0];
									b_move->fake_p_pos[1] = fake_p_pos[1];
									b_move->evaluation = max;

								}

							}
						}
					}
				}
			}
		}
	}


	printf("best depth: %d\n", tmp_best_depth);
	printf("MAX found is %d\n", max);
	return b_move;
}

/*minmax function. Recursively try all possible moves and then return the best evaluation or a final state evaluation*/
int
minmax_value(struct piece *fake_board[][COLUMNS], int color, int MAX, int depth, int *best_depth, int alpha, int beta) {

	int res = NOT_ALLOWED;
	int i, j;
	int tmp_n_fake_moves = -1;
	int move;
	int min, max;
	int to[2];
	int from[2];
	int legal_moves[TOTAL_MOVES_NUMBER];
	int is_the_end_res = -1;
	int MAX_DEPTH = -1;
	struct piece *tmp_fake_board[ROWS][COLUMNS];
	struct piece *tmp_fake_piece;
	struct piece *tmp_last_moved = last_moved;

	if(START_COLOR == WHITE_CHESS){
		MAX_DEPTH = WHITE_DEPTH;
	} else {
		MAX_DEPTH = BLACK_DEPTH;
	}

	/*check if the board is in a final state, and possibly return who wins*/
	is_the_end_res = is_the_end(fake_board, color);
	if(is_the_end_res == DRAW) {
		res = 0;
		return res;
	} else if(MAX && is_the_end_res != PLAYING) {

		if(is_the_end_res == COLOR_WIN) {
			res = PLUS_INFINITY;

			if((MAX_DEPTH-depth) < *best_depth) { /*found a better winning move*/
				*best_depth = MAX_DEPTH-depth;
			}
		} else if(is_the_end_res == NOT_COLOR_WIN) {
			res = MINUS_INFINITY;
		}
		return res;
	} else if(!MAX && is_the_end_res != PLAYING) {
		if(is_the_end_res == COLOR_WIN) {
			res = PLUS_INFINITY;

			if((MAX_DEPTH-depth) < *best_depth) { /*found a better winning move*/

				*best_depth = MAX_DEPTH-depth;
			}
		} else if(is_the_end_res == NOT_COLOR_WIN) {
			res = MINUS_INFINITY;
		}

		return res;


		/*return the board evaluation if depth is 0*/
	} else if(depth == 0) {

		res = evaluate(fake_board);
		if(res == PLUS_INFINITY) {

			if((MAX_DEPTH-depth) < *best_depth) { /*found a better winning move*/
				*best_depth = MAX_DEPTH-depth;
			}
		}
		return res;

		/*keep going with recursion and minmax*/
	} else {

		if(MAX) {
			max = MINUS_INFINITY-1;
		} else {
			min = PLUS_INFINITY+1;
		}


		for(i = 0; i < ROWS; i++) {
			for(j = 0; j < ROWS; j++) {


				reset_fake_board(fake_board, tmp_fake_board);
				if(tmp_fake_board[i][j] != NULL && tmp_fake_board[i][j]->color == color) {
					tmp_fake_piece = tmp_fake_board[i][j];
					from[0] = i;
					from[1] = j;
					tmp_n_fake_moves = -1;
					return_legal_moves(tmp_fake_board, legal_moves, from, tmp_fake_piece->type, tmp_fake_piece->color);


					for(move = 0; move < TOTAL_MOVES_NUMBER; move++) {
						if(legal_moves[move] > -1) {

							reset_fake_board(fake_board, tmp_fake_board);


							tmp_fake_piece = tmp_fake_board[i][j];
							/*restore tmp_n_fake_moves for en passant capture*/
							if(tmp_fake_piece->type == PAWN && tmp_n_fake_moves == -1) {
								tmp_n_fake_moves = tmp_fake_piece->n_fake_moves;
							} else if(tmp_fake_piece->type == PAWN){
								tmp_fake_piece->n_fake_moves = tmp_n_fake_moves;
							}
							to[0] = i;
							to[1] = j;

							moves(to, tmp_fake_piece->type, tmp_fake_piece->color, legal_moves[move]);
							make_fake_move(tmp_fake_board, tmp_fake_piece, from, to);
							res = minmax_value(tmp_fake_board, !color, !MAX, depth-1, best_depth, alpha, beta);
							last_moved = tmp_last_moved;
							if(tmp_fake_piece->type == PAWN) {
								tmp_fake_piece->n_fake_moves = tmp_n_fake_moves;
							}

							if(MAX && res != NOT_ALLOWED && (res > max || (max == MINUS_INFINITY-1))) {

								max = res;
								if(max > alpha)
									alpha = max;
								/*do not prune if it's the winning case. In that case also depth has to be taken into account*/
								if(beta == alpha && (alpha != PLUS_INFINITY || (MAX_DEPTH-depth)+1 >= *best_depth)) {
									return alpha;
								} else if(beta < alpha) {

									return alpha;
								}

							} else if(!MAX && res != NOT_ALLOWED && (res < min|| (min == PLUS_INFINITY+1))) {
								min = res;
								if(min < beta)
									beta = min;
								if (beta <= alpha) {
									return beta;

								}
							}
						}
					}
				}
			}
		}

		if(MAX) {
			return max;
		} else {
			return min;
		}
	}

}

void
game(FILE *f){
	struct piece *fake_board[ROWS][COLUMNS];

	srand(time(NULL));
	int color = START_COLOR;
	int pos[2];
	char input_move[4];
	int int_input_move[4];
	struct best_move tmp_best_move;
	struct piece *tmp_last_moved = last_moved;
	int res = -1;
	int white_random_play = 0;
	int black_random_play = 0;
	int human_turn = 0;
	int allowed_move = 1;
	int restored_board = 1;


	/*check if AI has to play randomly*/
	if(WHITE_DEPTH == 0) {
		white_random_play = 1;
	} else if(BLACK_DEPTH == 0) {
		black_random_play = 1;
	}

	/*check if it human has to start*/
	if(HUMAN_PLAYER && START_COLOR == BLACK_CHESS) {
		human_turn = 1;
	}

	print_board(NULL);
	while(1) {

		/*check if at the beginning there is a final situation. This is helpful for debugging*/
		if(last_moved == NULL) {
			res = is_the_end(NULL, color);
			last_moved = tmp_last_moved;

			if(res > 0) {
				if(res == COLOR_WIN) {
					if(START_COLOR == WHITE_CHESS) {
						printf("%s\n", RESULTS[WHITE_WIN]);
						fprintf(f, "\n%s\n", RESULTS[WHITE_WIN]);
					} else {
						printf("%s\n", RESULTS[BLACK_WIN]);
						fprintf(f, "\n%s\n", RESULTS[BLACK_WIN]);

					}
				} else if(res == NOT_COLOR_WIN) {
					if(START_COLOR == WHITE_CHESS) {
						printf("%s\n", RESULTS[BLACK_WIN]);
						fprintf(f, "\n%s\n", RESULTS[BLACK_WIN]);
					} else {
						printf("%s\n", RESULTS[WHITE_WIN]);
						fprintf(f, "\n%s\n", RESULTS[WHITE_WIN]);

					}
				} else {
					printf("%s\n", RESULTS[res]);
					fprintf(f, "\n%s\n", RESULTS[res]);
				}
				return;

			}
		}
		if(!human_turn) {

			memset(input_move, 0, 4);
			set_time(0);
			if((START_COLOR == WHITE_CHESS && white_random_play) || (START_COLOR  == BLACK_CHESS && black_random_play)) {
				make_random_move(color, input_move);
			} else {

				reset_fake_board(board, fake_board);
				printf("color: %d\n", color);
				decision_min_max(&tmp_best_move, fake_board, color);

				pos[0] = tmp_best_move.fake_p_pos[0];
				pos[1] = tmp_best_move.fake_p_pos[1];
				input_move[0] = pos[1]+65;
				sprintf(&input_move[1], "%d", pos[0]+1);

				moves(pos, board[pos[0]][pos[1]]->type, board[pos[0]][pos[1]]->color, tmp_best_move.move);

				make_move(board[tmp_best_move.fake_p_pos[0]][tmp_best_move.fake_p_pos[1]], pos);

				input_move[2] = pos[1]+65;
				sprintf(&input_move[3], "%d", pos[0]+1);
			}
			set_time(1);

		} else {
			set_time(0);
			if(human_moves_counter == 0) {
				reset_fake_board(board, tmp_last_move_board);
				reset_fake_board(board, last_move_board);

			}

			do {

				if(restored_board == 0) {
					printf("Please insert your move, '0000' to undo your last move..\n");
				} else {
					printf("Please insert your move..\n");
				}
				scanf(" %[^\n]s",input_move);
				memset(int_input_move, 0, 4);

				int_input_move[0] = input_move[0]%65;
				int_input_move[1] = (int)strtol(&input_move[1], NULL, 10)-1;
				int_input_move[2] = input_move[2]%65;
				int_input_move[3] = (int)strtol(&input_move[3], NULL, 10)-1;

				/*char 0%65 is equal to 48, int 0 - 1 is equal to -1 -> 48+48-1-1 = 94*/
				if((int_input_move[0]+int_input_move[1]+int_input_move[2]+int_input_move[3] == 94) && restored_board == 0) {
					restored_board = 1;
					allowed_move = 0;
					undo_last_move(last_move_board, board);
					reset_fake_board(last_move_board, tmp_last_move_board);
					print_board(NULL);
				} else if((int_input_move[0]+int_input_move[1]+int_input_move[2]+int_input_move[3] == 94) && restored_board == 1) {
					printf("You have already undone the last move!\n");
					allowed_move = 0;

				} else {
					pos[0] = int_input_move[3];
					pos[1] = int_input_move[2];
					allowed_move = is_legal_human_move(int_input_move);
					if(!allowed_move) {
						printf("Move not allowed!\n");
					}
				}
			} while(!allowed_move);

			make_move(board[int_input_move[1]][int_input_move[0]], pos);
			human_moves_counter++;
			restored_board = 0;
			set_time(1);
		}


		print_board(NULL);
		tmp_last_moved = last_moved;

		printf("%s\n", input_move);
		fprintf(f, "%s:\t%s\n", CHESS_COLORS[color], input_move);
		fflush(f);
		res = is_the_end(NULL, color);
		last_moved = tmp_last_moved;

		if(res > 0) {
			if(res == COLOR_WIN) {
				if(START_COLOR == WHITE_CHESS) {
					printf("%s\n", RESULTS[WHITE_WIN]);
					fprintf(f, "\n%s\n", RESULTS[WHITE_WIN]);
				} else {
					printf("%s\n", RESULTS[BLACK_WIN]);
					fprintf(f, "\n%s\n", RESULTS[BLACK_WIN]);

				}
			} else if(res == NOT_COLOR_WIN) {
				if(START_COLOR == WHITE_CHESS) {
					printf("%s\n", RESULTS[BLACK_WIN]);
					fprintf(f, "\n%s\n", RESULTS[BLACK_WIN]);
				} else {
					printf("%s\n", RESULTS[WHITE_WIN]);
					fprintf(f, "\n%s\n", RESULTS[WHITE_WIN]);

				}
			} else {
				printf("%s\n", RESULTS[res]);
				fprintf(f, "\n%s\n", RESULTS[res]);
			}
			return;
		}
		/*change turn*/
		color = !color;
		if(!HUMAN_PLAYER)
			START_COLOR = color;
		else {
			if(color != START_COLOR) {
				human_turn = 1;
			} else {
				human_turn = 0;
			}
		}
		printf("color: %s\n", COLORS[color]);
		if(PRESS_ENTER) {
			printf("Please, press enter to continue..\n");
			char c;
			do {
				c = getchar();
			} while (c != '\n' && c != EOF);
		}


	}

}



int
main(int argc, char *argv[]) {
	char filename[32];
	FILE *f = NULL;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	int r_match = 0;

	if(argc == 2) {
		r_match = 1;
	}
	create_chess();	
	initialize_board();
	memset(filename, 0, 32);
	sprintf(filename, "%d:%d:%d,%d-%d-%d.chess", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	f = fopen(filename, "w");
	if (f == NULL)
		exit(0);

	if(r_match) {
		resume_match(argv[1], f);
	}
	game(f);

	fclose(f);
	return 0;
}
