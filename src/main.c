#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h> // Terminal graphical user interface (GUI) library

#define MAX_MATRIX_SIZE 2
#define MAX_CHARS 16

// state machine state names
#define STATE_NAVIG 1
#define STATE_EDIT 2

int NUM_MTRX_ELEMENTS = MAX_MATRIX_SIZE*MAX_MATRIX_SIZE;

struct Global { // global variables
    int y_max, x_max; // nCurses screen size
    int key_ch; // pressed key
    int highlight_mtrx; // highlighted matrix
    int highlight_elem; // highlighted matrix element
    int state; // state machine state
    int *matrix_A, *matrix_B, *matrix_C; // A*B=C
} glob;

// edit field (box for user input)
int ef_start_y = 5, ef_start_x = 17;
int ef_inner_len = 2*MAX_CHARS + 1;

void bomb(char *msg);
void alloc_zeros(int **x, int size, char *error_message);
void check_screen_size();
void clip_int(int *x, int min, int max); // clips x between min and max
void multiply_matrices(int *A, int *B, int *C);
void draw_multiplication();
int input_is_valid(char *input);

void state_navig(int key_ch);
void state_edit(int key_ch);

void (*state_funcs[])(int key_ch) = {NULL, state_navig, state_edit};

//______________________________________________________________________________
/*
                    ##     ##    ###    #### ##    ##
                    ###   ###   ## ##    ##  ###   ##
                    #### ####  ##   ##   ##  ####  ##
                    ## ### ## ##     ##  ##  ## ## ##
                    ##     ## #########  ##  ##  ####
                    ##     ## ##     ##  ##  ##   ###
                    ##     ## ##     ## #### ##    ##
*/
//______________________________________________________________________________

int main(void) {
    initscr();
    
    check_screen_size();
    
    int key_ch;
    
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    ESCDELAY = 20;
    
    move(glob.y_max-1, glob.x_max-1);
    
    int size = sizeof(int)*pow(MAX_MATRIX_SIZE, 2);
    
    alloc_zeros(&glob.matrix_A, size, "couldn't allocate memory for matrix A");
    alloc_zeros(&glob.matrix_B, size, "couldn't allocate memory for matrix B");
    alloc_zeros(&glob.matrix_C, size, "couldn't allocate memory for matrix C");
    
    glob.matrix_A[0] = 2;
    glob.matrix_A[1] = 0;
    glob.matrix_A[2] = 0;
    glob.matrix_A[3] = -2;
   
    glob.matrix_B[0] = 0;
    glob.matrix_B[1] = 3;
    glob.matrix_B[2] = 3;
    glob.matrix_B[3] = 0;
    
    glob.highlight_elem = 0;
    glob.state = STATE_NAVIG;
    
    // ----------------------------------------------------------------
    // Edit field
    mvaddstr(1, 4, "This nCurses program multiplies two integer matrices.");
    mvaddstr(2, 4, "Use arrow keys to navigate. Press enter to edit.");
    
    move(ef_start_y, 4); addstr("Edit field: ");
    
    attron(A_ALTCHARSET);
        move(ef_start_y - 1, ef_start_x - 1); printw("%c", 108); // ┌
        move(ef_start_y    , ef_start_x - 1); printw("%c", 120); // │
        move(ef_start_y + 1, ef_start_x - 1); printw("%c", 109); // └
        for (int i = 0; i < ef_inner_len; i++) {
            move(ef_start_y - 1, ef_start_x + i); printw("%c", 113); // ─
            move(ef_start_y + 1, ef_start_x + i); printw("%c", 113); // ─
        }
        move(ef_start_y - 1, ef_start_x + ef_inner_len); printw("%c", 107); // ┐
        move(ef_start_y    , ef_start_x + ef_inner_len); printw("%c", 120); // │
        move(ef_start_y + 1, ef_start_x + ef_inner_len); printw("%c", 106); // ┘
    attroff(A_ALTCHARSET);
    
    // ---------------------------------------------------------------
    
    draw_multiplication();
    
    // ----------------------------------------------------------------
    
    void (*state_func)(int key_ch) = state_funcs[glob.state];
    
    move(ef_start_y, ef_start_x);
    do {
        key_ch = getch();
        if (key_ch == 410) check_screen_size();
        
        // clear edit field:
        move(ef_start_y, ef_start_x);
        for (int i = 0; i < ef_inner_len; i++) addstr(" ");
        move(ef_start_y, ef_start_x);
        
        state_func(key_ch);
        state_func = state_funcs[glob.state];
        
        napms(30);
    } while( key_ch != 27);
    
    endwin();
    free(glob.matrix_A);
    free(glob.matrix_B);
    free(glob.matrix_C);
    
    return 0;
}

//______________________________________________________________________________
/*
 ######## ##     ## ##    ##  ######  ######## ####  #######  ##    ##  ######
 ##       ##     ## ###   ## ##    ##    ##     ##  ##     ## ###   ## ##    ##
 ##       ##     ## ####  ## ##          ##     ##  ##     ## ####  ## ##
 ######   ##     ## ## ## ## ##          ##     ##  ##     ## ## ## ##  ######
 ##       ##     ## ##  #### ##          ##     ##  ##     ## ##  ####       ##
 ##       ##     ## ##   ### ##    ##    ##     ##  ##     ## ##   ### ##    ##
 ##        #######  ##    ##  ######     ##    ####  #######  ##    ##  ######
*/
//______________________________________________________________________________

void bomb(char *msg) {
    endwin(); puts(msg); exit(1);
}

void alloc_zeros(int **x, int size, char *error_message) {
    int *mtrx = malloc(size); if(x == NULL) bomb(error_message);
    memset(mtrx, 0, size);
    *x = mtrx;
}

void check_screen_size() {
    getmaxyx(stdscr, glob.y_max, glob.x_max);
    if((glob.y_max < 24) || (glob.x_max < 80)) bomb("Screen size must be at least 24 rows 80 columns");
}

void clip_int(int *x, int min, int max) {
    if (*x > max) *x = max;
    else if (*x < min) *x = min;
}

int input_is_valid(char *input) {
    // checks if user input is a valid integer
    int len, n;
    
    len = strlen(input);
    n = (int) input[0];
    
    // First char should be either - or a digit,
    // if there's only 1 char it should be a digit
    if (len > 1) { if (!(((47 < n) && (n < 58)) || (n == 45))) return 0; }
    else if (!((47 < n) && (n < 58))) return 0;
    
    // Rest of the chars should all be digits
    for (int i = 0; i < len - 1; i++) {
        n = input[i + 1];
        if (!((47 < n) && (n < 58))) return 0;
    }
    return 1;
}

//______________________________________________________________________________

void draw_matrix(
        int *coord_y, int *coord_x, int **in_mtrx,
        int highlight_attr_style, int n_highlight
    ) {
    char str_buffer[MAX_CHARS];
    char i2s_buffer[MAX_MATRIX_SIZE][MAX_CHARS];
    int maxlen = 1, temp, n, i_len, attr_style;
    
    attron(A_ALTCHARSET);
        move(*coord_y, *coord_x); printw("%c", 108);                        // ┌
        for (int i = 1; i < 2*MAX_MATRIX_SIZE; i++) {
            move(*coord_y + i, *coord_x); printw("%c", 120);                // │
        }
        move(*coord_y + 2*MAX_MATRIX_SIZE, *coord_x); printw("%c", 109);    // └
    attroff(A_ALTCHARSET);
    *coord_x += 2;
    // ---------------------------------------------------------
    for (int j = 0; j < MAX_MATRIX_SIZE; j++) {
        maxlen = 1;
        for (int i = 0; i < MAX_MATRIX_SIZE; i++) {
            n = i*MAX_MATRIX_SIZE + j;
            sprintf(str_buffer, "%d", (*in_mtrx)[n]);
            i_len = strlen(str_buffer);
            if (i_len > maxlen) maxlen = i_len;
        }
        for (int i = 0; i < MAX_MATRIX_SIZE; i++) {
            n = i*MAX_MATRIX_SIZE + j;
            sprintf(str_buffer, "%d", (*in_mtrx)[n]);
            attr_style = (n == n_highlight) ? highlight_attr_style : A_NORMAL;
            attron(attr_style);
            mvaddstr(
                *coord_y + 2*i + 1,
                *coord_x + maxlen - strlen(str_buffer),
                str_buffer);
            attroff(attr_style);
        }
        *coord_x += maxlen + 1;
    }
    // ---------------------------------------------------------
    attron(A_ALTCHARSET);
        move(*coord_y, *coord_x); printw("%c", 107);                        // ┐
        for (int i = 1; i < 2*MAX_MATRIX_SIZE; i++) {
            move(*coord_y + i, *coord_x); printw("%c", 120);                // │
        }
        move(*coord_y + 2*MAX_MATRIX_SIZE, *coord_x); printw("%c", 106);    // ┘
    attroff(A_ALTCHARSET); (*coord_x)++;
}

void multiply_matrices(int *A, int *B, int *C) {
    int dot_ij, i, j;
    for (int n = 0; n < NUM_MTRX_ELEMENTS; n++) {
        i = n/MAX_MATRIX_SIZE;
        j = n%MAX_MATRIX_SIZE;
        dot_ij = 0;
        for (int k = 0; k < MAX_MATRIX_SIZE; k++)
            dot_ij += A[i*MAX_MATRIX_SIZE + k]*B[k*MAX_MATRIX_SIZE + j];
        C[n] = dot_ij;
    }
}

void draw_multiplication() {
    int coord_y = 7, coord_x = 4;
    int attr_style_used, attr_style_state;
    
    // clear
    for (int i = 0; i < 1 + 2*MAX_MATRIX_SIZE; i++)
        for (int j = 0; j < 64; j++) mvaddstr(coord_y + i, coord_x + j, " ");


    attr_style_state = (glob.state == STATE_EDIT) ?
        (A_BOLD | A_UNDERLINE) : (A_REVERSE | A_DIM);
    
    multiply_matrices(glob.matrix_A, glob.matrix_B, glob.matrix_C);
    
    if (glob.highlight_mtrx == 0) {
        draw_matrix(&coord_y, &coord_x, &glob.matrix_A,
            attr_style_state, glob.highlight_elem);
        draw_matrix(&coord_y, &coord_x, &glob.matrix_B,
            attr_style_state, NUM_MTRX_ELEMENTS);
    } else {
        draw_matrix(&coord_y, &coord_x, &glob.matrix_A,
            attr_style_state, NUM_MTRX_ELEMENTS);
        draw_matrix(&coord_y, &coord_x, &glob.matrix_B,
            attr_style_state, glob.highlight_elem);
    }
    mvaddstr(coord_y + 2, coord_x + 1, "="); coord_x += 3;
    draw_matrix(&coord_y, &coord_x, &glob.matrix_C,
        attr_style_state, NUM_MTRX_ELEMENTS);
}

//______________________________________________________________________________

void state_navig(int key_ch) {
    int y, x, i, j; getyx(stdscr, y, x);
    switch(key_ch) {
        case KEY_RIGHT:
            i = glob.highlight_elem/MAX_MATRIX_SIZE;
            j = glob.highlight_elem%MAX_MATRIX_SIZE + 1;
            if (glob.highlight_mtrx == 0) {
                if (j >= MAX_MATRIX_SIZE) { // then jump to second matrix
                    j = 0;
                    glob.highlight_mtrx = 1;
                }
            } else {
                clip_int(&j, 0, MAX_MATRIX_SIZE - 1);
            }
            glob.highlight_elem = i*MAX_MATRIX_SIZE + j;
            draw_multiplication();
            break;
        case KEY_LEFT:
            i = glob.highlight_elem/MAX_MATRIX_SIZE;
            j = glob.highlight_elem%MAX_MATRIX_SIZE - 1;
            if (glob.highlight_mtrx == 1) {
                if (j < 0) { // then jump to first matrix
                    j = MAX_MATRIX_SIZE - 1;
                    glob.highlight_mtrx = 0;
                }
            } else {
                clip_int(&j, 0, MAX_MATRIX_SIZE - 1);
            }
            glob.highlight_elem = i*MAX_MATRIX_SIZE + j;
            draw_multiplication();
            break;
        case KEY_UP:
            i = glob.highlight_elem/MAX_MATRIX_SIZE - 1;
            j = glob.highlight_elem%MAX_MATRIX_SIZE;
            clip_int(&i, 0, MAX_MATRIX_SIZE - 1);
            glob.highlight_elem = i*MAX_MATRIX_SIZE + j;
            draw_multiplication();
            break;
        case KEY_DOWN:
            i = glob.highlight_elem/MAX_MATRIX_SIZE + 1;
            j = glob.highlight_elem%MAX_MATRIX_SIZE;
            clip_int(&i, 0, MAX_MATRIX_SIZE - 1);
            glob.highlight_elem = i*MAX_MATRIX_SIZE + j;
            draw_multiplication();
            break;
        case 10: // Enter
            glob.state = STATE_EDIT;
            draw_multiplication();
            break;
    }
    move(y, x);
}

void state_edit(int key_ch) {
    int y, x; getyx(stdscr, y, x);
    char input_str[MAX_CHARS];
    
    move(glob.y_max-1, 0);
    for (int i = 0; i < 16; i++) addstr(" ");
    
    move(ef_start_y, ef_start_x);
    nodelay(stdscr, FALSE);
        getnstr(input_str, MAX_CHARS-1);
    nodelay(stdscr, TRUE);
    
    if (input_is_valid(input_str)) {
        if (glob.highlight_mtrx == 0)
            glob.matrix_A[glob.highlight_elem] = atoi(input_str);
        else if (glob.highlight_mtrx == 1)
            glob.matrix_B[glob.highlight_elem] = atoi(input_str);
    } else {
        mvaddstr(glob.y_max - 1, 0, "invalid input");
    }
    
    glob.state = STATE_NAVIG;
    draw_multiplication();
    
    move(y, x);
}
