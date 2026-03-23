#define _XOPEN_SOURCE 700

#include "my_mc.h"

#include <stdio.h>
#include <string.h>

static void extract_last_component(const char *path, char *out, size_t out_size) {
    const char *end;
    const char *start;
    size_t len;

    if (path == NULL || out_size == 0) {
        return;
    }
    out[0] = '\0';
    if (path[0] == '\0' || strcmp(path, "/") == 0) {
        return;
    }

    len = strlen(path);
    end = path + len;
    while (end > path + 1 && *(end - 1) == '/') {
        end--;
    }
    start = end;
    while (start > path && *(start - 1) != '/') {
        start--;
    }
    len = (size_t)(end - start);
    if (len >= out_size) {
        len = out_size - 1;
    }
    memcpy(out, start, len);
    out[len] = '\0';
}

static void go_parent_keep_selection(Panel *p) {
    char child_name[MAX_NAME];
    char parent[MAX_PATH];

    extract_last_component(p->path, child_name, sizeof(child_name));
    if (join_path_safe(p->path, "..", parent, sizeof(parent)) != 0) {
        snprintf(parent, sizeof(parent), "/");
    }
    enter_dir(p, parent);
    focus_entry_by_name(p, child_name);
}

static void enter_directory_at_cursor(Panel *p) {
    FileEntry *fe;

    if (p->count <= 0) {
        return;
    }
    fe = &p->files[p->cursor];
    if (!fe->is_dir) {
        return;
    }
    if (strcmp(fe->name, "..") == 0) {
        go_parent_keep_selection(p);
        return;
    }
    enter_dir(p, fe->full_path);
}

void main_loop(void) {
    int ch;

    for (;;) {
        Panel *ap;

        getmaxyx(stdscr, rows, cols);
        redraw();
        ch = getch();

        if (ch == KEY_RESIZE) {
            continue;
        }

        ap = &panels[active];

        switch (ch) {
        case KEY_UP:
            if (ap->cursor > 0) ap->cursor--;
            scroll_into_view(ap);
            break;
        case KEY_DOWN:
            if (ap->cursor < ap->count - 1) ap->cursor++;
            scroll_into_view(ap);
            break;
        case '\n':
        case KEY_ENTER:
        case '\r':
            enter_directory_at_cursor(ap);
            break;
        case '\t':
            active = 1 - active;
            break;
        case KEY_BACKSPACE:
        case 127:
            if (cmdlen > 0) {
                cmdline[--cmdlen] = '\0';
            } else {
                go_parent_keep_selection(ap);
            }
            break;
        case KEY_F(10):
        case 'q':
        case 'Q':
            return;
        default:
            if (ch >= 32 && ch < 127 && cmdlen < (int)sizeof(cmdline) - 1) {
                cmdline[cmdlen++] = (char)ch;
                cmdline[cmdlen] = '\0';
            }
            break;
        }
    }
}
