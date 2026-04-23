#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chat_ui.h"

int chat_ui_init(void)
{
    if (initscr() == NULL) {
        fprintf(stderr, "Failed to initialize ncurses\n");
        return -1;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    timeout(100);

    return 0;
}

void chat_ui_shutdown(void)
{
    if (stdscr != NULL) {
        endwin();
    }
}

int chat_ui_prompt_name(char *name, size_t size, pid_t default_pid)
{
    erase();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "Chat client");
    mvprintw(3, 2, "Enter your name: ");
    mvprintw(5, 2, "Press Enter to continue");
    move(3, 20);
    refresh();

    echo();
    curs_set(1);
    timeout(-1);
    if (getnstr(name, (int)size - 1) == ERR) {
        noecho();
        timeout(100);
        return -1;
    }

    noecho();
    timeout(100);

    name[strcspn(name, "\n")] = '\0';
    if (name[0] == '\0') {
        snprintf(name, size, "user_%ld", (long)default_pid);
    }

    return 0;
}

void chat_ui_draw(const char *name,
                  const char *input,
                  const char messages[][CHAT_UI_LINE_LENGTH],
                  int message_count,
                  const char users[][MAX_NAME],
                  int user_count)
{
    int rows;
    int cols;
    int top_height;
    int users_width;
    int messages_width;
    WINDOW *messages_win;
    WINDOW *users_win;
    WINDOW *input_win;

    getmaxyx(stdscr, rows, cols);
    if (rows < 8 || cols < 50) {
        erase();
        mvprintw(0, 0, "Terminal window is too small for ncurses chat");
        mvprintw(1, 0, "Resize the terminal and try again");
        refresh();
        return;
    }

    top_height = rows - 4;
    users_width = cols / 4;
    if (users_width < 18) {
        users_width = 18;
    }
    if (users_width > 30) {
        users_width = 30;
    }
    messages_width = cols - users_width;

    messages_win = newwin(top_height, messages_width, 0, 0);
    users_win = newwin(top_height, users_width, 0, messages_width);
    input_win = newwin(4, cols, top_height, 0);

    if (messages_win == NULL || users_win == NULL || input_win == NULL) {
        if (messages_win != NULL) {
            delwin(messages_win);
        }
        if (users_win != NULL) {
            delwin(users_win);
        }
        if (input_win != NULL) {
            delwin(input_win);
        }
        return;
    }

    werase(messages_win);
    werase(users_win);
    werase(input_win);
    box(messages_win, 0, 0);
    box(users_win, 0, 0);
    box(input_win, 0, 0);

    mvwprintw(messages_win, 0, 2, " Messages ");
    mvwprintw(users_win, 0, 2, " Users ");
    mvwprintw(input_win, 0, 2, " Input ");
    mvwprintw(input_win, 1, 2, "%s> %s", name, input);
    mvwprintw(input_win, 2, 2, "Enter - send, /exit - quit");

    for (int i = 0; i < message_count && i < top_height - 2; ++i) {
        mvwprintw(messages_win, 1 + i, 2, "%.*s", messages_width - 4, messages[i]);
    }

    for (int i = 0; i < user_count && i < top_height - 2; ++i) {
        mvwprintw(users_win, 1 + i, 2, "%.*s", users_width - 4, users[i]);
    }

    wrefresh(messages_win);
    wrefresh(users_win);
    wrefresh(input_win);
    move(top_height + 1, (int)strlen(name) + 4 + (int)strlen(input));
    refresh();

    delwin(messages_win);
    delwin(users_win);
    delwin(input_win);
}
