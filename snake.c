/**
 * A snake game using ncurses
 *
 * Copyright (c) 2023 David Boehme
 *
 * SPDX-License-Identifier: MIT
 */

#include <ncurses.h>

#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#undef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#undef max
#define max(a,b) ((a)>(b) ? (a) : (b))

const char* helptext =
    "A Snake game.\n\n"
    " Use arrow keys or 'w', 'a', 's', 'd' to move.\n"
    " Use Space to pause.\n"
    " Use 'q' to quit the game.\n\n"
    "Arguments:\n"
    " -h        Print help\n"
    " -d [num]  Set difficulty (speed), 0 to 10. Default: 5";

enum direction { Up, Down, Left, Right };
enum gamestate { GameRunning, GameOver };

typedef struct opts_t {
    int difficulty;
} Options;

typedef struct game_t {
    int size_x;
    int size_y;
    int initial_snake_len;
    int snake_len;
    int snake_pos;
    int food_pos;
    enum direction move;
    long tick_msec;
    struct timespec prev_tick;
    int* field;
} Game;


Game* init_game(const Options* opts)
{
    const long difficulty_msecs[12] = {
        300, 210, 180, 150, 130, 120, 100, 80, 60, 40, 30, 20
    };

    Game* game = (Game*) malloc(sizeof(Game));

    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    game->size_x = max_x - 2;
    game->size_y = max_y - 2;
    game->initial_snake_len = game->snake_len = min(10, game->size_x);
    game->snake_pos = game->size_y/2*game->size_x + (game->size_x/2 - game->snake_len/2);
    game->food_pos = max(0, game->snake_pos - 1);
    game->move = Left;
    game->tick_msec = difficulty_msecs[max(0, min(11, opts->difficulty))];

    clock_gettime(CLOCK_MONOTONIC, &game->prev_tick);

    game->field = (int*) malloc(game->size_x * game->size_y * sizeof(int));
    memset(game->field, 0, game->size_x * game->size_y * sizeof(int));

    for (int c = game->snake_len, p = game->snake_pos; c > 0; --c, ++p)
        game->field[p] = c;

    srand((unsigned) (game->prev_tick.tv_nsec / 1000000));

    return game;
}


void free_game(Game* game)
{
    free(game->field);
    free(game);
}


int is_tick(Game* game)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);

    long msec_diff = 0;
    time_t sec_diff = now.tv_sec - game->prev_tick.tv_sec;

    if (sec_diff > 0) {
        msec_diff = ((long) sec_diff-1)*1000;
        long nsec_diff = now.tv_nsec + (999999999 - game->prev_tick.tv_nsec);
        msec_diff += nsec_diff/1000000;
    } else {
        msec_diff = (now.tv_nsec - game->prev_tick.tv_nsec)/1000000;
    }

    if (msec_diff > game->tick_msec) {
        game->prev_tick = now;
        return 1;
    }

    return 0;
}


int game_score(Game* game)
{
    return (game->snake_len - game->initial_snake_len - 1);
}


void draw_gameover(Game* game)
{
    nodelay(stdscr, FALSE);

    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);

    char buf[20];
    memset(buf, 0, 20);
    int len2 = snprintf(buf, 20, " Score: %d ", game_score(game));

    mvprintw(max_y/2-1, max_x/2-5, " GAME OVER ");
    mvprintw(max_y/2, max_x/2-len2/2, buf);

    refresh();
    getch();
}


void draw_pause()
{
    nodelay(stdscr, FALSE);

    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);

    mvprintw(max_y/2, max_x/2-6, " GAME PAUSED ");

    refresh();
    getch();

    nodelay(stdscr, TRUE);
}


void draw(Game* game)
{
    for (int p = 0; p < game->size_x * game->size_y; ++p) {
        int y = p / game->size_x;
        int x = p - (y * game->size_x);

        if (game->field[p] > 0)
            mvaddch(1+y, 1+x, '#');
        else if (p == game->food_pos)
            mvaddch(1+y, 1+x, 'o');
        else
            mvaddch(1+y, 1+x, ' ');
    }

    refresh();
}


enum gamestate step(Game* game)
{
    const int field_size = game->size_x*game->size_y;

    /* move the snake head and check if we hit a border */
    switch (game->move) {
        case Up:
            if (game->snake_pos - game->size_x < 0)
                return GameOver;
            game->snake_pos -= game->size_x;
            break;
        case Left:
            if (game->snake_pos % game->size_x == 0)
                return GameOver;
            --game->snake_pos;
            break;
        case Down:
            if (game->snake_pos + game->size_x >= field_size)
                return GameOver;
            game->snake_pos += game->size_x;
            break;
        case Right:
            if ((game->snake_pos+1) % game->size_x == 0)
                return GameOver;
            ++game->snake_pos;
            break;
    }

    /* check if we hit the snake body */
    if (game->field[game->snake_pos] > 0)
        return GameOver;

    game->field[game->snake_pos] = game->snake_len;

    /* check if we hit the food */
    if (game->snake_pos == game->food_pos) {
        ++game->snake_len;

        /* find a new position for the food */
        int r = rand() % (field_size - game->snake_len);
        for (int p = 0; p < field_size; ++p) {
            if (game->field[p] == 0)
                if (r-- == 0) {
                    game->food_pos = p;
                    break;
                }
        }
    } else {
        /* update the field (moves the snake body) */
        for (int p = 0; p < field_size; ++p)
            if (game->field[p] > 0)
                --game->field[p];
    }

    return GameRunning;
}


int run(Game* game)
{
    const struct timespec sleeptime = { 0, 5000000 }; /* 5 msec */

    for (;;) {
        int ch = getch();

        if (ch == 'q')
            break;

        if      (ch == KEY_UP    || ch == 'w')
            game->move = Up;
        else if (ch == KEY_LEFT  || ch == 'a')
            game->move = Left;
        else if (ch == KEY_DOWN  || ch == 's')
            game->move = Down;
        else if (ch == KEY_RIGHT || ch == 'd')
            game->move = Right;
        else if (ch == ' ') {
            draw_pause();
            continue;
        }

        if (is_tick(game)) {
            if (step(game) == GameOver) {
                draw_gameover(game);
                break;
            }

            draw(game);
        }

        nanosleep(&sleeptime, NULL);
    }

    return game_score(game);
}


void init_screen()
{
    initscr();             /* init ncurses*/
    noecho();              /* disable key press echo */
    cbreak();              /* no buffering */
    keypad(stdscr, TRUE);  /* return special keys (arrows etc.) */
    nodelay(stdscr, TRUE); /* return key immediately */
    curs_set(0);           /* hide the cursor */

#if 0
    start_color();
    short fg_color = 0, bg_color = 0;
    pair_content(0, &fg_color, &bg_color);
    init_pair(1, bg_color, fg_color); /* invert fg/bg color for the snake */
#endif

    border(0, 0, 0, 0, 0, 0, 0, 0);
}


Options parse_options(int argc, char* argv[])
{
    Options opts = {
        .difficulty = 5
    };

    int opt = 0;
    while ((opt = getopt(argc, argv, "d:h")) != -1) {
        switch (opt) {
        case 'd': {
            int val = atoi(optarg);
            if (val < 0 || val > 11) { /* Difficulty actually goes to 11 */
                fprintf(stderr, "Invalid difficulty '%d': Must be between 0 and 10\n", val);
                exit(EXIT_FAILURE);
            }
            opts.difficulty = val;
            break;
        }
        case 'h':
            puts(helptext);
            exit(EXIT_SUCCESS);
        }
    }

    return opts;
}


int main(int argc, char* argv[])
{
    Options opts = parse_options(argc, argv);

    init_screen();

    Game* game = init_game(&opts);
    int score = run(game);
    free_game(game);

    endwin();

    printf("Score: %d\n", score);

    return 0;
}