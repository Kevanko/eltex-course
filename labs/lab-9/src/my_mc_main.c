#define _XOPEN_SOURCE 700

#include "my_mc.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Panel panels[2];
int active = 0;
int rows = 0;
int cols = 0;
char cmdline[MAX_PATH] = "";
int cmdlen = 0;

int main(int argc, char *argv[]) {
    char cwd[MAX_PATH];
    const char *start_path = NULL;

    setlocale(LC_ALL, "");

    if (argc > 1) {
        start_path = argv[1];
    } else if (getcwd(cwd, sizeof(cwd)) != NULL) {
        start_path = cwd;
    } else {
        start_path = "/";
    }

    strncpy(panels[0].path, start_path, MAX_PATH - 1);
    panels[0].path[MAX_PATH - 1] = '\0';
    strncpy(panels[1].path, start_path, MAX_PATH - 1);
    panels[1].path[MAX_PATH - 1] = '\0';

    /* обе панели с одной стартовой папки */
    load_dir(&panels[0]);
    load_dir(&panels[1]);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    set_escdelay(25);

    if (has_colors()) {
        init_colors();
    }

    getmaxyx(stdscr, rows, cols);
    main_loop();
    endwin();
    return 0;
}
