/**
 * @file minesweeper.c
 * @author Tristan von Reyn (tristan.vonreyn@student.nmt.edu)
 * @brief Terminal-based Minesweeper game using notcurses.
 * @version 0.1
 * @date 2026-05-03
 */

#include <netinet/in.h>
#include <notcurses/notcurses.h>
#include <notcurses/nckeys.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

/**
 * @brief Reveal a board cell and its connected empty region
 *
 * @param x X value of the checked cell
 * @param y Y value of the checked cell
 * @param height Board height
 * @param width Board width
 * @param gameboard Two-dimensional board state array
 */
void reveal(int x, int y, int height, int width, int gameboard[height][width]) {
    if (gameboard[x][y] == 12 || gameboard[x][y] < 0) {
        return;
    }
    if (gameboard[x][y] == 0) {
        gameboard[x][y] = 12;
        if (x-1 >= 0) {
            reveal(x-1, y, height, width, gameboard);
        }
        if (x+1 < height) {
            reveal(x+1, y, height, width, gameboard);
        }
        if (y-1 >= 0) {
            reveal(x, y-1, height, width, gameboard);
        }
        if (y+1 < width) {
            reveal(x, y+1, height, width, gameboard);
        }
        if (x-1 >= 0 && y-1 >= 0) {
            reveal(x-1, y-1, height, width, gameboard);
        }
        if (x-1 >= 0 && y+1 < width) {
            reveal(x-1, y+1, height, width, gameboard);
        }
        if (x+1 < height && y-1 >= 0) {
            reveal(x+1, y-1, height, width, gameboard);
        }
        if (x+1 < height && y+1 < width) {
            reveal(x+1, y+1, height, width, gameboard);
        }
    } else if (gameboard[x][y] > 0 && gameboard[x][y] < 10) {
        gameboard[x][y] = -gameboard[x][y];
    }
}

/**
 * @brief Render the game board
 *
 * @param n Notcurses plane for board rendering
 * @param height Board height
 * @param width Board width
 * @param gameboard Two-dimensional board state array
 * @param game_mode Current game mode
 */
void board(struct ncplane* n, int height, int width, int gameboard[height][width], int game_mode) {
    ncplane_erase(n);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (gameboard[y][x] == 12) {
                ncplane_set_fg_rgb(n, 0x8B4513);
                ncplane_printf_yx(n, y +1, (x * 2) + 1, "■");
                if (game_mode == 2 && (y + x) % 2 == 0) {
                    ncplane_set_fg_rgb(n, 0xD2691E);
                    ncplane_printf_yx(n, y +1, (x * 2) + 1, "■");
                }
            } else if (gameboard[y][x] < 0) {
                ncplane_set_fg_rgb(n, 0xFF0000);
                ncplane_printf_yx(n, y +1, (x * 2) + 1, "%d", -gameboard[y][x]);
                if (game_mode == 2 && (y + x) % 2 == 0) {
                    ncplane_set_fg_rgb(n, 0xFF6347);
                    ncplane_printf_yx(n, y +1, (x * 2) + 1, "%d", -gameboard[y][x]);
                }
            } else if (gameboard[y][x] == 11 || gameboard[y][x] == 13) {
                ncplane_set_fg_rgb(n, 0x0000FF);
                ncplane_putstr_yx(n, y +1, (x * 2) + 1, "■");
                if (game_mode == 2 && (y + x) % 2 == 0) {
                    ncplane_set_fg_rgb(n, 0x1E90FF);
                    ncplane_putstr_yx(n, y +1, (x * 2) + 1, "■");
                }
            } else {
                ncplane_set_fg_rgb(n, 0x00FF00);
                ncplane_putstr_yx(n, y +1, (x * 2) + 1, "■");
                if (game_mode == 2 && (y+x) % 2 == 0) {
                    ncplane_set_fg_rgb(n, 0x006400);
                    ncplane_putstr_yx(n, y +1, (x * 2) + 1, "■");
                }
            }
        }
    }
}

/**
 * @brief Show the bombs and flags
 *
 * @param n Target notcurses plane for panel rendering
 * @param bombs Total bombs
 * @param flags Total flagged
 */
void count(struct ncplane* n, int bombs, int flags) {
    ncplane_erase(n);
    ncplane_set_fg_rgb(n, 0x000000);
    ncplane_printf_yx(n, 4, 1, "Bombs: %d", bombs);
    ncplane_set_fg_rgb(n, 0x0000FF);
    ncplane_printf_yx(n, 6, 1, "Flags: %d", flags);
}

/**
 * @brief Display and handle size selection menu
 *
 * @param nc Notcurses instance
 * @param stdn Standard plane for rendering
 * @param height Pointer to height variable
 * @param width Pointer to width variable
 * @param bombs Pointer to bombs variable
 * @param size_mult Pointer to size_mult variable
 * @return-1 on error or quit
 */
int size(struct notcurses* nc, struct ncplane* stdn, int* height, int* width, int* bombs, int* size_mult) {
    ncplane_erase(stdn);
    ncplane_set_fg_rgb(stdn, 0xFFFFFF);
    ncplane_putstr_yx(stdn, 1, 2, "Select Board Size");
    ncplane_putstr_yx(stdn, 3, 2, "[1] 9x9    (10 bombs)");
    ncplane_putstr_yx(stdn, 4, 2, "[2] 16x16  (40 bombs)");
    ncplane_putstr_yx(stdn, 5, 2, "[3] 30x16  (99 bombs)");
    ncplane_putstr_yx(stdn, 6, 2, "[4] Custom");
    ncplane_putstr_yx(stdn, 8, 2, "Click 1-4");
    notcurses_render(nc);
    
    struct ncinput input = {0};
    while (1) {
        if (notcurses_get_blocking(nc, &input) < 0) {
            return -1;
        }
        if (input.evtype != NCTYPE_PRESS) {
            continue;
        }
        int selection = -1;
        if (input.id >= NCKEY_BUTTON1 && input.id <= NCKEY_BUTTON3) {
            if (input.y == 3) {
                selection = 1;
            } else if (input.y == 4) {
                selection = 2;
            } else if (input.y == 5) {
                selection = 3;
            } else if (input.y == 6) {
                selection = 4;
            }
        }
        
        if (selection == 1) {
            *height = 9; *width = 9; *bombs = 10; *size_mult = 3;
            return 0;
        } else if (selection == 2) {
            *height = 16; *width = 16; *bombs = 40; *size_mult = 2;
            return 0;
        } else if (selection == 3) {
            *height = 16; *width = 30; *bombs = 99; *size_mult = 1;
            return 0;
        } else if (selection == 4) {
            notcurses_stop(nc);
            printf("Enter height: ");
            scanf("%d", height);
            printf("Enter width: ");
            scanf("%d", width);
            printf("Enter bombs: ");
            scanf("%d", bombs);
            if (*height <= 0 || *width <= 0 || *bombs <= 0 || *bombs >= *height * *width || *bombs > 1000) {
                printf("Invalid input. Please enter positive integers for height and width, and a bomb count between 1 and height*width-1.\n");
                return -1; 
            }
            nc = notcurses_init(NULL, NULL);
            notcurses_mice_enable(nc, 0x7);
            stdn = notcurses_stdplane(nc);
            return 0;
        }
    }
}

/**
 * @brief Display and handle game mode selection menu
 *
 * @param nc Notcurses instance
 * @param stdn Standard plane for rendering
 * @param game_mode Pointer to game_mode variable (output)
 * @return 0 on success, -1 on error or quit
 */
int mode(struct notcurses* nc, struct ncplane* stdn, int* game_mode) {
    ncplane_erase(stdn);
    
    ncplane_set_fg_rgb(stdn, 0xFFFFFF);
    ncplane_putstr_yx(stdn, 1, 2, "SELECT GAME MODE");
    ncplane_putstr_yx(stdn, 3, 2, "[1] Normal");
    ncplane_putstr_yx(stdn, 4, 2, "[2] Checkerboard");
    ncplane_putstr_yx(stdn, 5, 2, "[3] Liar");
    ncplane_putstr_yx(stdn, 7, 2, "Click 1-3");
    notcurses_render(nc);
    
    struct ncinput input = {0};
    while (1) {
        if (notcurses_get_blocking(nc, &input) < 0) {
            return -1;
        }
        if (input.evtype != NCTYPE_PRESS) {
            continue;
        }       
        int selection = -1;
        if (input.id >= NCKEY_BUTTON1 && input.id <= NCKEY_BUTTON3) {
            if (input.y == 3) selection = 1;
            else if (input.y == 4) selection = 2;
            else if (input.y == 5) selection = 3;
        }
        
        if (selection >= 1 && selection <= 3) {
            *game_mode = selection;
            return 0;
        }
    }
}

 
/**
 * @brief main function
 * Initializes the board and UI, then handles user input
 *
 * @return Exit status code
 */
int main(void)
{
    int height = 9;
    int width = 9;
    int bombs = 10;
    int size_mult = 1;
    int game_mode = 1;
    int correct_bombs = 0;
    int flags = 0;

    srand(time(NULL));

    struct notcurses* nc = notcurses_init(NULL, NULL);
    notcurses_mice_enable(nc, 0x7);
    struct ncplane* stdn = notcurses_stdplane(nc);
    if (size(nc, stdn, &height, &width, &bombs, &size_mult) < 0) {
        notcurses_stop(nc);
        return 0;
    }
    if (mode(nc, stdn, &game_mode) < 0) {
        notcurses_stop(nc);
        return 0;
    }
    ncplane_erase(stdn);
    notcurses_render(nc);
    
    int gameboard[height][width];
    
    struct ncplane_options opts = {
        .y = 0,
        .x = 0,
        .rows = height + 1,
        .cols = width * 2,
    };
    struct ncplane* board_plane = ncplane_create(stdn, &opts);
    struct ncplane_options panel_opts = {
        .y = 0,
        .x = width * 2 + 1,
        .rows = height + 1,
        .cols = 20,
    };
    struct ncplane* panel_plane = ncplane_create(stdn, &panel_opts);

    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            gameboard[i][j] = 0;
        }
    }
    for (int i = 0; i < bombs; i++) {
        int x = rand() % height;
        int y = rand() % width;
        if (gameboard[x][y] == 10) {
            i--;
        } else {
            gameboard[x][y] = 10;
        }
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (gameboard[i][j] != 10) {
                if (i-1 >= 0 && gameboard[i-1][j] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i-1+j) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (i+1 < height && gameboard[i+1][j] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i+1+j) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (j-1 >= 0 && gameboard[i][j-1] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i+j-1) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (j+1 < width && gameboard[i][j+1] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i+j+1) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (i-1 >= 0 && j-1 >= 0 && gameboard[i-1][j-1] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i-1+j-1) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (i-1 >= 0 && j+1 < width && gameboard[i-1][j+1] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i-1+j+1) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (i+1 < height && j-1 >= 0 && gameboard[i+1][j-1] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i+1+j-1) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (i+1 < height && j+1 < width && gameboard[i+1][j+1] == 10) {
                    gameboard[i][j]++;
                    if (game_mode == 2 && (i+1+j+1) % 2 == 0) {
                        gameboard[i][j]++;
                    }
                }
                if (game_mode == 3 && gameboard[i][j] > 0 && gameboard[i][j] < 10) {
                    if (rand() % 2 == 0) {
                        gameboard[i][j]++;
                    } else {
                        gameboard[i][j]--;
                    }
                    if (gameboard[i][j] < 0) {
                        gameboard[i][j] = 0;
                    }
                    if (gameboard[i][j] > 8) {
                        gameboard[i][j] = 8;
                    }
                }
            }
        }
    }

    board(board_plane, height, width, gameboard, game_mode);
    count(panel_plane, bombs, flags);
    notcurses_render(nc);
    struct ncinput input = {0};
    while (correct_bombs < bombs) {
        if (notcurses_get_blocking(nc, &input) < 0) {
            break;
        }
        notcurses_render(nc);
        if (input.evtype != NCTYPE_PRESS) {
            continue;
        }
        int is_mouse_click = (input.id >= NCKEY_BUTTON1 && input.id <= NCKEY_BUTTON3) && (input.x >= 0 && input.y >= 0);

        if (!is_mouse_click) {
            continue;
        }
        if (input.id == NCKEY_BUTTON1) {
            int y;
            int x;
            
            if (is_mouse_click) {

                y = input.y - 1;
                x = (input.x - 1) / 2;
            }
            if (x < 0 || x >= width || y < 0 || y >= height) {
                continue;
            }
            
            if (gameboard[y][x] == 12 || gameboard[y][x] < 0) {
                continue;
            }
            if (gameboard[y][x] == 10) {
                ncplane_erase(board_plane);
                ncplane_set_fg_rgb(board_plane, 0xFF0000);
                ncplane_putstr_yx(board_plane, 2, 2, "GAME OVER!");
                ncplane_putstr_yx(board_plane, 4, 2, "You hit a bomb!");
                ncplane_putstr_yx(board_plane, 6, 2, "Press Q to quit");
                notcurses_render(nc);
                while (1) {
                    struct ncinput quit_input = {0};
                    if (notcurses_get_blocking(nc, &quit_input) < 0) break;
                    if (quit_input.id == 'q' || quit_input.id == 'Q') {
                        notcurses_stop(nc);
                        return 0;
                    }
                }
            } else if (gameboard[y][x] == 0) {
                reveal(y, x, height, width, gameboard);
            } else if (gameboard[y][x] > 0 && gameboard[y][x] < 10) {
                gameboard[y][x] = -gameboard[y][x];
            }

            board(board_plane, height, width, gameboard, game_mode);
            notcurses_render(nc);
        } else if (input.id == NCKEY_BUTTON3) {
            int y, x;
            if (is_mouse_click) {
                y = input.y - 1;
                x = (input.x - 1) / 2;
            }
            if (x < 0 || x >= width || y < 0 || y >= height) {
                continue;
            }
            if (gameboard[y][x] == 11) {
                gameboard[y][x] = 10;
                correct_bombs--;
                flags--;
            } else if (gameboard[y][x] == 13) {
                gameboard[y][x] = 0;
                correct_bombs--;
                flags--;
            } else if (gameboard[y][x] == 10) {
                gameboard[y][x] = 11;
                correct_bombs++;
                flags++;
            } else if (gameboard[y][x] >= 0 && gameboard[y][x] < 10) {
                gameboard[y][x] = 13;
                flags++;
            } else {
                continue;
            }
            board(board_plane, height, width, gameboard, game_mode);
            count(panel_plane, bombs, flags);
            notcurses_render(nc);
        }
    }
    int total_cells = height * width;
    int revealed_cells = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (gameboard[i][j] < 0 || gameboard[i][j] == 12) {
                revealed_cells++;
            }
        }
    }

    if (correct_bombs == bombs) {
        ncplane_erase(board_plane);
        ncplane_set_fg_rgb(board_plane, 0x00FF00); 
        ncplane_putstr_yx(board_plane, 2, 2, "CONGRATULATIONS!");
        ncplane_putstr_yx(board_plane, 4, 2, "You won the game!");
        ncplane_putstr_yx(board_plane, 6, 2, "Press Q to quit");
        notcurses_render(nc);
        while (1) {
            struct ncinput quit_input = {0};
            if (notcurses_get_blocking(nc, &quit_input) < 0) break;
            if (quit_input.id == 'q' || quit_input.id == 'Q') {
                break;
            }
        }
    }

    notcurses_stop(nc);
}










