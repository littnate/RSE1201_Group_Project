#ifndef UTIL_INPUT_H
#define UTIL_INPUT_H

#include <stdbool.h>
#include <stddef.h>

bool util_read_line(char *buf, size_t buf_size);

bool util_prompt_yes_no(const char *prompt);

bool util_prompt_yes_no_default(const char *prompt,
                                bool default_yes,
                                bool *out);

bool util_prompt_long_default(const char *prompt,
                              long min_val,
                              long default_val,
                              long *out);

int util_prompt_int_range(const char *prompt, int min_val, int max_val);

bool util_prompt_long(const char *prompt, long *out);

char util_read_command_char(void);

#endif
