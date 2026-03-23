#define _XOPEN_SOURCE 700

#include "my_mc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void fmt_size(off_t size, char *buf, int buflen) {
    snprintf(buf, buflen, "%7lld", (long long)size);
}

static void fmt_time(time_t t, char *buf, int buflen) {
    static const char *mon[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    struct tm *tm = localtime(&t);

    snprintf(buf, buflen, "%s %2d %02d:%02d",
             mon[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min);
}

void init_colors(void) {
    start_color();
    use_default_colors();

    init_pair(CP_PANEL,        COLOR_WHITE,  COLOR_BLUE);
    init_pair(CP_PANEL_BORDER, COLOR_WHITE,  COLOR_BLUE);
    init_pair(CP_PANEL_HEADER, COLOR_BLACK,  COLOR_WHITE);
    init_pair(CP_DIR,          COLOR_WHITE,  COLOR_BLUE);
    init_pair(CP_CURSOR,       COLOR_BLACK,  COLOR_CYAN);
    init_pair(CP_TOPBAR,       COLOR_BLACK,  COLOR_CYAN);
    init_pair(CP_CMDLINE,      COLOR_WHITE,  COLOR_BLACK);
    init_pair(CP_FKEY_NUM,     COLOR_WHITE,  COLOR_BLACK);
    init_pair(CP_FKEY_LABEL,   COLOR_BLACK,  COLOR_CYAN);
    init_pair(CP_INACTIVE,     COLOR_WHITE,  COLOR_BLUE);
    init_pair(CP_INACTIVE_DIR, COLOR_WHITE,  COLOR_BLUE);
    init_pair(CP_FKEY_IDLE,    COLOR_WHITE, COLOR_RED);
}

/*
 * панель: рамка, путь сверху, таблица, внизу полоска и счётчик
 */
static void draw_panel(int panel_idx) {
    Panel *p = &panels[panel_idx];
    int is_act = (panel_idx == active);
    int panel_w = cols / 2;
    int panel_x = panel_idx * panel_w;
    int panel_h = rows - TOP_LINES - BOTTOM_LINES;
    int panel_y = TOP_LINES;
    int file_rows = panel_h - PANEL_NON_LIST_ROWS;
    int inner_w = panel_w - 2;
    int date_w = 12;
    int size_w = 7;
    int name_w = inner_w - size_w - date_w - 2;
    int sep1_x;
    int sep2_x;
    int data_x = panel_x + 1;
    char disp_path[MAX_PATH];
    char path_label[MAX_PATH + 4];
    int max_path_len = panel_w - 4;
    int path_x = panel_x + 2;

    if (panel_idx == 1) panel_w = cols - panel_x;
    if (file_rows < 1) file_rows = 1;

    attrset(COLOR_PAIR(CP_PANEL_BORDER));
    mvaddch(panel_y, panel_x, ACS_ULCORNER);

    format_display_path(p->path, disp_path, sizeof(disp_path));
    snprintf(path_label, sizeof(path_label), " %s ", disp_path);

    for (int i = 0; i < panel_w - 2; i++) {
        mvaddch(panel_y, panel_x + 1 + i, ACS_HLINE);
    }

    if (max_path_len < 1) max_path_len = 1;
    if (is_act) {
        attrset(COLOR_PAIR(CP_PANEL_HEADER) | A_BOLD);
    } else {
        attrset(COLOR_PAIR(CP_PANEL_BORDER));
    }
    mvprintw(panel_y, path_x, "%.*s", max_path_len, path_label);

    attrset(COLOR_PAIR(CP_PANEL_BORDER));
    mvaddch(panel_y, panel_x + panel_w - 1, ACS_URCORNER);

    for (int r = 1; r < panel_h - 1; r++) {
        attrset(COLOR_PAIR(CP_PANEL_BORDER));
        mvaddch(panel_y + r, panel_x, ACS_VLINE);
        mvaddch(panel_y + r, panel_x + panel_w - 1, ACS_VLINE);

        attrset(COLOR_PAIR(CP_PANEL));
        for (int c = 1; c < panel_w - 1; c++) {
            mvaddch(panel_y + r, panel_x + c, ' ');
        }
    }

    attrset(COLOR_PAIR(CP_PANEL_BORDER));
    mvaddch(panel_y + panel_h - 1, panel_x, ACS_LLCORNER);
    for (int i = 1; i < panel_w - 1; i++) {
        mvaddch(panel_y + panel_h - 1, panel_x + i, ACS_HLINE);
    }
    mvaddch(panel_y + panel_h - 1, panel_x + panel_w - 1, ACS_LRCORNER);

    if (name_w < 8) name_w = 8;
    if (name_w + size_w + date_w + 2 > inner_w) {
        date_w = inner_w - name_w - size_w - 2;
        if (date_w < 4) date_w = 4;
    }
    sep1_x = data_x + name_w;
    sep2_x = sep1_x + 1 + size_w;

    {
        const char *name_hdr = "Name";
        int name_len = (int)strlen(name_hdr);
        int name_x = data_x + (name_w - name_len) / 2;
        if (name_x < data_x) {
            name_x = data_x;
        }

        attrset(COLOR_PAIR(is_act ? CP_PANEL : CP_INACTIVE) | A_BOLD);
        mvprintw(panel_y + 1, data_x, "%-*s", name_w, "");
        mvprintw(panel_y + 1, name_x, "%s", name_hdr);
    }
    attrset(COLOR_PAIR(is_act ? CP_PANEL : CP_INACTIVE) | A_BOLD);
    mvprintw(panel_y + 1, sep1_x + 1, "%*.*s", size_w, size_w, "Size");
    mvprintw(panel_y + 1, sep2_x + 1, "%-*.*s", date_w, date_w, "Changed");
    attrset(COLOR_PAIR(CP_PANEL_BORDER));
    mvaddch(panel_y + 1, sep1_x, ACS_VLINE);
    mvaddch(panel_y + 1, sep2_x, ACS_VLINE);

    for (int i = 0; i < file_rows; i++) {
        int file_idx = p->top + i;
        int row = panel_y + 2 + i;

        if (file_idx >= p->count) {
            attrset(COLOR_PAIR(CP_PANEL));
            mvprintw(row, panel_x + 1, "%-*s", inner_w, "");
            attrset(COLOR_PAIR(CP_PANEL_BORDER));
            mvaddch(row, sep1_x, ACS_VLINE);
            mvaddch(row, sep2_x, ACS_VLINE);
            continue;
        }

        FileEntry *fe = &p->files[file_idx];
        int is_cur = (file_idx == p->cursor);
        int pair = CP_PANEL;
        int attr = 0;
        char size_str[16];
        char date_str[16];
        char disp_name[MAX_NAME + 2];

        if (is_cur && is_act) {
            pair = CP_CURSOR;
        } else if (!is_act) {
            pair = fe->is_dir ? CP_INACTIVE_DIR : CP_INACTIVE;
        } else {
            pair = fe->is_dir ? CP_DIR : CP_PANEL;
        }

        attr = COLOR_PAIR(pair);
        if (fe->is_dir && !(is_cur && is_act)) attr |= A_BOLD;
        attrset(attr);

        if (strcmp(fe->name, "..") == 0) {
            snprintf(size_str, sizeof(size_str), "%s", "[..]");
        } else {
            fmt_size(fe->size, size_str, sizeof(size_str));
        }
        fmt_time(fe->mtime, date_str, sizeof(date_str));

        if (fe->is_dir && strcmp(fe->name, "..") != 0) {
            snprintf(disp_name, sizeof(disp_name), "%s/", fe->name);
        } else {
            strncpy(disp_name, fe->name, MAX_NAME);
        }

        if ((int)strlen(disp_name) > name_w) {
            disp_name[name_w - 1] = '~';
            disp_name[name_w] = '\0';
        }

        mvprintw(row, data_x, "%-*.*s", name_w, name_w, disp_name);
        mvprintw(row, sep1_x + 1, "%*.*s", size_w, size_w, size_str);
        mvprintw(row, sep2_x + 1, "%-*.*s", date_w, date_w, date_str);
        attrset(COLOR_PAIR(CP_PANEL_BORDER));
        mvaddch(row, sep1_x, ACS_VLINE);
        mvaddch(row, sep2_x, ACS_VLINE);
    }

    {
        int split_row = panel_y + panel_h - 3;
        attrset(COLOR_PAIR(CP_PANEL_BORDER));
        mvaddch(split_row, panel_x, ACS_LTEE);
        mvhline(split_row, panel_x + 1, ACS_HLINE, panel_w - 2);
        mvaddch(split_row, panel_x + panel_w - 1, ACS_RTEE);
    }

    attrset(COLOR_PAIR(is_act ? CP_PANEL : CP_INACTIVE));
    if (p->count > 0) {
        FileEntry *cur_fe = &p->files[p->cursor];
        char stat_buf[MAX_NAME + 32];
        snprintf(stat_buf, sizeof(stat_buf), " %d/%d %s", p->cursor + 1, p->count, cur_fe->name);
        mvprintw(panel_y + panel_h - 2, panel_x + 1, "%-*.*s", inner_w, inner_w, stat_buf);
    } else {
        mvprintw(panel_y + panel_h - 2, panel_x + 1, "%-*.*s", inner_w, inner_w, " 0/0");
    }
}
/* Для красоты, ничего не делает */
static void draw_topbar(void) {
    static const char *items[] = {
        "Win-L", "Catalog", "Actions", "Setup", "Win-R"
    };
    int x = 1;

    attrset(COLOR_PAIR(CP_TOPBAR) | A_BOLD);
    mvprintw(0, 0, "%-*s", cols, "");

    for (int i = 0; i < 5; i++) {
        attrset(COLOR_PAIR(CP_TOPBAR) | A_BOLD);
        mvprintw(0, x, " %s ", items[i]);
        x += (int)strlen(items[i]) + 2;
    }
}

static void draw_hintline(void) {
    int y = rows - BOTTOM_LINES;

    attrset(COLOR_PAIR(CP_CMDLINE));
    mvprintw(y, 0, "%-*s", cols, "");
    mvprintw(y, 0, "%s",
             "F10=exit Tab=panel Enter=folder Back=up");
}

static void build_cmd_prompt(char *prompt, size_t prompt_size) {
    char hostname[64] = "localhost";
    char *user = getenv("USER");

    if (!user) {
        user = "user";
    }
    gethostname(hostname, sizeof(hostname));
    snprintf(prompt, prompt_size, "[%s@%s %s]$ ", user, hostname, panels[active].path);
}

static void draw_cmdline(void) {
    int y = rows - BOTTOM_LINES + 1;
    char prompt[MAX_PATH + 160];

    attrset(COLOR_PAIR(CP_CMDLINE));
    mvprintw(y, 0, "%-*s", cols, "");

    build_cmd_prompt(prompt, sizeof(prompt));
    attrset(COLOR_PAIR(CP_CMDLINE) | A_BOLD);
    mvprintw(y, 0, "%s", prompt);
    attrset(COLOR_PAIR(CP_CMDLINE));
    printw("%.*s", cols - (int)strlen(prompt) - 1, cmdline);
}

static void draw_fkeys(void) {
    int y = rows - 1;
    static const char *labels[] = {
        "About", "Pick", "Peek", "Patch", "Clone",
        "Reloc", "MkDir", "Erase", "Extra", "Exit"
    };

    attrset(COLOR_PAIR(CP_FKEY_NUM));
    mvprintw(y, 0, "%-*s", cols, "");

    for (int i = 0; i < FKEY_COUNT; i++) {
        int x1 = (cols * i) / FKEY_COUNT;
        int x2 = (cols * (i + 1)) / FKEY_COUNT;
        int width = x2 - x1;
        int label_x = x1 + 2;
        int label_w = width - 3;
        int key_num = i + 1;
        int is_working = (key_num == 10);

        if (width <= 0) continue;

        attrset(COLOR_PAIR(CP_FKEY_NUM) | A_BOLD);
        mvprintw(y, x1, "%2d", i + 1);
        if (label_w > 0) {
            attrset(COLOR_PAIR(is_working ? CP_FKEY_LABEL : CP_FKEY_IDLE));
            mvprintw(y, label_x, "%-*.*s", label_w, label_w, labels[i]);
        }
        if (x2 - 1 >= x1) {
            attrset(COLOR_PAIR(CP_FKEY_NUM));
            mvaddch(y, x2 - 1, ' ');
        }
    }
}

static void draw_separator(void) {
    int x = cols / 2;
    int panel_h = rows - TOP_LINES - BOTTOM_LINES;

    attrset(COLOR_PAIR(CP_PANEL_BORDER));
    for (int r = 1; r < panel_h - 1; r++) {
        int y = TOP_LINES + r;
        mvaddch(y, x, ACS_VLINE);
    }
}

void redraw(void) {
    char prompt[MAX_PATH + 160];
    int cmd_y = rows - BOTTOM_LINES + 1;
    int cmd_x = 0;

    erase();
    draw_topbar();
    draw_panel(0);
    draw_panel(1);
    draw_separator();
    draw_hintline();
    draw_cmdline();
    draw_fkeys();

    build_cmd_prompt(prompt, sizeof(prompt));
    cmd_x = (int)strlen(prompt) + cmdlen;
    move(cmd_y, cmd_x < cols ? cmd_x : cols - 1);

    refresh();
}
