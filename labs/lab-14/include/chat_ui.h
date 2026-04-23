#ifndef CHAT_UI_H
#define CHAT_UI_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "chat.h"

#define CHAT_UI_LINE_LENGTH CHAT_LINE_LENGTH

int chat_ui_init(void);
void chat_ui_shutdown(void);
int chat_ui_prompt_name(char *name, size_t size, pid_t default_pid);
void chat_ui_draw(const char *name,
                  const char *input,
                  const char messages[][CHAT_UI_LINE_LENGTH],
                  int message_count,
                  const char users[][MAX_NAME],
                  int user_count);

#endif
