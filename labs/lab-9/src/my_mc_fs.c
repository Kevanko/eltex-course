#define _XOPEN_SOURCE 700

#include "my_mc.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

/* qsort */
static int cmp_entries(const void *a, const void *b) {
    const FileEntry *fa = (const FileEntry *)a;
    const FileEntry *fb = (const FileEntry *)b;

    if (strcmp(fa->name, "..") == 0) return -1;
    if (strcmp(fb->name, "..") == 0) return 1;
    if (fa->is_dir != fb->is_dir) return fb->is_dir - fa->is_dir;
    return strcasecmp(fa->name, fb->name);
}

void format_display_path(const char *path, char *out, size_t out_size) {
    const char *home = getenv("HOME");
    size_t home_len = home ? strlen(home) : 0;

    if (home && home_len > 0 && strncmp(path, home, home_len) == 0) {
        if (path[home_len] == '\0') {
            snprintf(out, out_size, "~");
            return;
        }
        if (path[home_len] == '/') {
            snprintf(out, out_size, "~%s", path + home_len);
            return;
        }
    }
    snprintf(out, out_size, "%s", path);
}

int join_path_safe(const char *base, const char *name, char *out, size_t out_size) {
    size_t base_len = strlen(base);
    size_t name_len = strlen(name);

    if (base_len + 1 + name_len + 1 > out_size) {
        return -1;
    }
    snprintf(out, out_size, "%s/%s", base, name);
    return 0;
}

void load_dir(Panel *p) {
    DIR *d = NULL;
    struct dirent *ent = NULL;
    struct stat st;
    char fullpath[MAX_PATH];

    p->count = 0;
    p->cursor = 0;
    p->top = 0;

    d = opendir(p->path);
    if (!d) return;

    if (strcmp(p->path, "/") != 0 && p->count < MAX_FILES) {
        FileEntry *fe = &p->files[p->count++];
        memset(fe, 0, sizeof(*fe));
        strcpy(fe->name, "..");
        if (join_path_safe(p->path, "..", fe->full_path, sizeof(fe->full_path)) == 0) {
            fe->is_dir = 1;
            stat(fe->full_path, &st);
            fe->size = st.st_size;
            fe->mtime = st.st_mtime;
        } else {
            p->count--;
        }
    }

    while ((ent = readdir(d)) != NULL && p->count < MAX_FILES) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        if (join_path_safe(p->path, ent->d_name, fullpath, sizeof(fullpath)) != 0) {
            continue;
        }
        if (stat(fullpath, &st) != 0) {
            continue;
        }

        FileEntry *fe = &p->files[p->count++];
        memset(fe, 0, sizeof(*fe));
        snprintf(fe->name, sizeof(fe->name), "%s", ent->d_name);
        snprintf(fe->full_path, sizeof(fe->full_path), "%s", fullpath);
        fe->size = st.st_size;
        fe->mtime = st.st_mtime;
        fe->is_dir = S_ISDIR(st.st_mode);
    }

    closedir(d);
    qsort(p->files, p->count, sizeof(FileEntry), cmp_entries);
}

void scroll_into_view(Panel *p) {
    int panel_h = rows - TOP_LINES - BOTTOM_LINES;
    int file_rows = panel_h - PANEL_NON_LIST_ROWS;

    if (file_rows < 1) {
        file_rows = 1;
    }

    if (p->cursor < p->top) {
        p->top = p->cursor;
    }
    if (p->cursor >= p->top + file_rows) {
        p->top = p->cursor - file_rows + 1;
    }
    if (p->top < 0) {
        p->top = 0;
    }
}

void focus_entry_by_name(Panel *p, const char *name) {
    if (name == NULL || *name == '\0') {
        return;
    }
    for (int i = 0; i < p->count; i++) {
        if (strcmp(p->files[i].name, name) == 0) {
            p->cursor = i;
            scroll_into_view(p);
            return;
        }
    }
}

void enter_dir(Panel *p, const char *path) {
    snprintf(p->path, sizeof(p->path), "%s", path);
    load_dir(p);
}
