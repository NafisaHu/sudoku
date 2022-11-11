/*
 * sudoku.c: A simple command-line Sudoku solver in C for educational purposes.
 * 
 * Author: Ricardo Garcia Gonzalez.
 * License: Public domain code.
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define SUBDIMENSION	(3)
#define MIN_NUM		(1)
#define MAX_NUM		(9)
#define TOTAL_NUMS	(9)
#define ARRAY_SIZE	(MIN_NUM + TOTAL_NUMS)

#ifdef ASSERT
#define assert_(A) assert(A)
#else
#define assert_(A)
#endif

#define MATRIXSIZE 5000

double a[MATRIXSIZE][MATRIXSIZE];
double b[MATRIXSIZE][MATRIXSIZE];
double c[MATRIXSIZE][MATRIXSIZE];

/*
 *
 * CANDIDATES.
 *
 */

/*
 * A candidates array represents which values have already been used for a row,
 * column or square.
 */
typedef int candidates[ARRAY_SIZE];

/*
 * All candidates start as unused.
 */
void init_candidates(candidates *c)
{
	int i;

	for (i = MIN_NUM; i <= MAX_NUM; ++i)
		(*c)[i] = 0;
}

/*
 * Using a candidate number means marking it as used in the array.
 */
void use_candidate(candidates *cp, int num)
{
	assert_(cp != NULL && num >= MIN_NUM && num <= MAX_NUM);

	(*cp)[num] = 1;
}

/*
 * Restoring a candidate means marking it as unused in the array.
 */
void restore_candidate(candidates *cp, int num)
{
	assert_(cp != NULL && num >= MIN_NUM && num <= MAX_NUM);

	(*cp)[num] = 0;
}

/*
 *
 * CELLS AND BOARDS.
 *
 */

/*
 * A cell has a flag to indicate if its value has been set or not, the cell
 * value and three pointers to candidate arrays. One for the row it belongs to,
 * one for the column it belongs to and one for the square it belongs to.
 */
struct cell {
	int has_value;
	int value;

	candidates *row_candidates;
	candidates *col_candidates;
	candidates *square_candidates;
};

/*
 * A board has a number of unset cells, a matrix of cells and the candidate
 * arrays for each row, column and square in the board.
 */
struct board {
	int unset_cells;
	struct cell cells[ARRAY_SIZE][ARRAY_SIZE];

	candidates rows[ARRAY_SIZE];
	candidates columns[ARRAY_SIZE];
	candidates squares[ARRAY_SIZE];
};

/*
 * Auxiliar. Calculates the square number for the given cell. Squares are
 * numberd from top to bottom, left to right.
 */
int square(int row, int col)
{
	assert_(row >= MIN_NUM && row <= MAX_NUM &&
	       col >= MIN_NUM && col <= MAX_NUM);

	return (((row - 1) / SUBDIMENSION) * SUBDIMENSION) +
		((col - 1) / SUBDIMENSION) + 1;
}

/*
 * Every board starts empty. Cell candidate pointers are established.
 */
void init_board(struct board *b)
{
	int i;
	int j;

	assert_(b != NULL);

	b->unset_cells = TOTAL_NUMS * TOTAL_NUMS;

	for (i = MIN_NUM; i <= MAX_NUM; ++i) {
		init_candidates(b->rows + i);
		init_candidates(b->columns + i);
		init_candidates(b->squares + i);

		for (j = MIN_NUM; j <= MAX_NUM; ++j) {
			b->cells[i][j].has_value = 0;
			b->cells[i][j].value = 0;
			b->cells[i][j].row_candidates = b->rows + i;
			b->cells[i][j].col_candidates = b->columns + j;
			b->cells[i][j].square_candidates = b->squares + square(i, j);
		}
	}
}

/*
 * Finds the lowest candidate number which is free in all arrays, having a
 * value greater or equal to the "atleast" argument.
 */
int find_common_free(candidates *r, candidates *c, candidates *s, int atleast)
{
	assert_(r != NULL && c != NULL && s != NULL);

	int i;
	for (i = atleast; i <= MAX_NUM; ++i)
		if ((! (*r)[i]) && (! (*c)[i]) && (! (*s)[i]))
			return i;
	return (-1);
}

/*
 * Sets a cell value in the given board.
 */
void set_cell(struct board *b, int r, int c, int val)
{
	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM &&
	       val >= MIN_NUM && val <= MAX_NUM);

	assert_((! (*(b->cells[r][c].row_candidates))[val]) &&
	       (! (*(b->cells[r][c].col_candidates))[val]) &&
	       (! (*(b->cells[r][c].square_candidates))[val]));

	b->unset_cells -= 1;
	b->cells[r][c].has_value = 1;
	b->cells[r][c].value = val;
	use_candidate(b->cells[r][c].row_candidates, val);
	use_candidate(b->cells[r][c].col_candidates, val);
	use_candidate(b->cells[r][c].square_candidates, val);
}

/*
 * Unsets a cell value in the given board.
 */
void unset_cell(struct board *b, int r, int c, int val)
{
	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM &&
	       val >= MIN_NUM && val <= MAX_NUM);

	assert_((*(b->cells[r][c].row_candidates))[val] &&
	       (*(b->cells[r][c].col_candidates))[val] &&
	       (*(b->cells[r][c].square_candidates))[val]);

	b->unset_cells += 1;
	b->cells[r][c].has_value = 0;
	b->cells[r][c].value = 0;
	restore_candidate(b->cells[r][c].row_candidates, val);
	restore_candidate(b->cells[r][c].col_candidates, val);
	restore_candidate(b->cells[r][c].square_candidates, val);
}

/*
 * Checks if a cell value is set. Returns 1 if set, 0 otherwise.
 */
int is_set(struct board *b, int r, int c)
{
	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM);

	return (b->cells[r][c].has_value);
}

/*
 * Calculates the number following a given one circularly.
 */
int following(int num)
{
	assert_(num >= MIN_NUM && num <= MAX_NUM);

	return ((num - MIN_NUM + 1) % TOTAL_NUMS + MIN_NUM);
}

/*
 * Calculates the cell following a given one. Advances from top to bottom and
 * left to right. Returns 0 if there is no next cell, 1 otherwise and modifies
 * the arguments to point to the next cell in that case.
 */
int next_cell(int *r, int *c)
{
	assert_(r != NULL && c != NULL);

	if ((*r) == MAX_NUM && (*c) == MAX_NUM)
		return 0;

	*c = following(*c);
	if ((*c) == MIN_NUM)
		(*r) = following(*r);
	return 1;
}

/*
 * Prints the given board on screen.
 */
void print_board(struct board *b)
{
	int i;
	int j;

	assert_(b != NULL);

	for (i = MIN_NUM; i <= MAX_NUM; ++i) {
		for (j = MIN_NUM; j <= MAX_NUM; ++j)
			printf(" %d", b->cells[i][j].value);
		printf("\n");
	}
}

/*
 * Solves a board starting with the given cell. Returns 1 if the board could be
 * solved, 0 if not.
 */
int solve_board(struct board *b, int r, int c)
{
	int prev;
	int val;

	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM);

	/* Base case: board solved, print it. */
	if (b->unset_cells == 0) {
		print_board(b);
		return 1;
	}

	/* Find the next unset cell. */
	while (is_set(b, r, c) && next_cell(&r, &c))
	       ;

	/* This should never happen. */
	if (is_set(b, r, c))
		return 1;

	/* Try every possible cell value until the board can be solved. */
	prev = MIN_NUM;
	while (1) {
		val = find_common_free(b->cells[r][c].row_candidates,
				       b->cells[r][c].col_candidates,
				       b->cells[r][c].square_candidates,
				       prev);
		if (val == -1)
			break;

		set_cell(b, r, c, val);
		if (solve_board(b, r, c))
			return 1;
		unset_cell(b, r, c, val);

		prev = val+1;
	}

	return 0;
}

/*
 * Reads a board from the given file. Format: a digit represents a cell value,
 * a dot represents an empty cell. Cells should be given from top to bottom and
 * left to right. All other characters are ignored.
 *
 * Example:
 *
 *     5 3 . . 7 . . . .
 *     6 . . 1 9 5 . . .
 *     . 9 8 . . . . 6 .
 *     8 . . . 6 . . . 3
 *     4 . . 8 . 3 . . 1
 *     7 . . . 2 . . . 6
 *     . 6 . . . . 2 8 .
 *     . . . 4 1 9 . . 5
 *     . . . . 8 . . 7 9
 *
 */
void read_board(FILE *f, struct board *b)
{
	int row;
	int col;
	int c;

	assert_(f != NULL && b != NULL);

	row = MIN_NUM;
	col = MIN_NUM;

	while (! feof(f)) {
		c = fgetc(f);
		if ((isdigit(c) && c != '0') || c == '.') {
			if (c != '.')
				set_cell(b, row, col, (c - '0'));
			if (! next_cell(&row, &col))
				break;
		}
	}
}

double
fun1(double a, double b)
{
	return (1 * a * b);
}

double
fun2(double a, double b)
{
	return (2 * a * b);
}

double
fun3(double a, double b)
{
	return (3 * a * b);
}

double
fun4(double a, double b)
{
	return (4 * a * b);
}

double
fun5(double a, double b)
{
	return (5 * a * b);
}


double
fun6(double a, double b)
{
	return (6 * a * b);
}

double
fun7(double a, double b)
{
	return (7 * a * b);
}

double
fun8(double a, double b)
{
	return (8 * a * b);
}

double
fun9(double a, double b)
{
	return (9 * a * b);
}

void ReduceIPC()
{
	int rows, cols, temp;

	for (rows = 0; rows < MATRIXSIZE; rows++) {
		for (cols = 0; cols < MATRIXSIZE; cols++) {
			temp = rand() % 9;

			switch(temp) {
				case 1:
					c[rows][cols] = fun1(a[rows][cols], b[rows][cols]);
					break;
				case 2:
					c[rows][cols] = fun2(a[rows][cols], b[rows][cols]);
					break;
				case 3:
					c[rows][cols] = fun3(a[rows][cols], b[rows][cols]);
					break;
				case 4:
					c[rows][cols] = fun4(a[rows][cols], b[rows][cols]);
					break;
				case 5:
					c[rows][cols] = fun5(a[rows][cols], b[rows][cols]);
					break;
				case 6:
					c[rows][cols] = fun6(a[rows][cols], b[rows][cols]);
					break;
				case 7:
					c[rows][cols] = fun7(a[rows][cols], b[rows][cols]);
					break;
				case 8:
					c[rows][cols] = fun8(a[rows][cols], b[rows][cols]);
					break;
				case 9:
					c[rows][cols] = fun9(a[rows][cols], b[rows][cols]);
					break;
				default: break; 
			}
		}
	}
}

/*
 *
 * MAIN PROGRAM.
 *
 */

int main(int argc, char *argv[])
{
	FILE *in;
	struct board b;

	int ret, count;

	if (argc > 2) {
		fprintf(stderr, "ERROR: too many arguments\n");
		return 1;
	}
    for (count = 0; count < 6000; count++) {
	    if (argc == 2) {
		    in = fopen(argv[1], "r");
		    if (in == NULL) {
			    fprintf(stderr, "ERROR: could not open \"%s\"\n", argv[1]);
			    return 2;
		    }
	    } else {
		    in = stdin;
	    }

	    init_board(&b);

	    read_board(in, &b);
	    ret = solve_board(&b, MIN_NUM, MIN_NUM);

	    fclose(in);
    }
	if (! ret)
		fprintf(stderr, "ERROR: board could not be solved\n");

	ReduceIPC();
	return (ret?0:3);
}
