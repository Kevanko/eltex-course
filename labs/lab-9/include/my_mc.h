#ifndef MY_MC_H
#define MY_MC_H

#include <ncurses.h>
#include <sys/types.h>
#include <time.h>

#define MAX_FILES     2048
#define MAX_PATH      4096
#define MAX_NAME      256
#define FKEY_COUNT    10
#define BOTTOM_LINES  3
#define TOP_LINES     1

#define PANEL_NON_LIST_ROWS 5

enum {
    CP_PANEL         = 1,
    CP_PANEL_BORDER  = 2,
    CP_PANEL_HEADER  = 3,
    CP_DIR           = 4,
    CP_CURSOR        = 5,
    CP_TOPBAR        = 6,
    CP_CMDLINE       = 7,
    CP_FKEY_NUM      = 8,
    CP_FKEY_LABEL    = 9,
    CP_INACTIVE      = 10,
    CP_INACTIVE_DIR  = 11,
    CP_FKEY_IDLE = 12, /* нижняя полоса: неактивные клавиши */
};

typedef struct {
    char name[MAX_NAME];
    char full_path[MAX_PATH];
    off_t size;
    time_t mtime;
    int is_dir;
} FileEntry;

typedef struct {
    char path[MAX_PATH];
    FileEntry files[MAX_FILES];
    int count;
    int cursor;
    int top;
} Panel;

extern Panel panels[2];
extern int active;
extern int rows;
extern int cols;
extern char cmdline[MAX_PATH];
extern int cmdlen;

void load_dir(Panel *p);
void enter_dir(Panel *p, const char *path);
void scroll_into_view(Panel *p);
void focus_entry_by_name(Panel *p, const char *name);
void init_colors(void);
void redraw(void);
void main_loop(void);
void format_display_path(const char *path, char *out, size_t out_size);
int join_path_safe(const char *base, const char *name, char *out, size_t out_size);

#endif
