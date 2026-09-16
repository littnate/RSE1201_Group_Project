#ifndef UTIL_INPUT_H
#define UTIL_INPUT_H

#include <stdbool.h>
#include <stddef.h>

/* Read one line into buf (nul-terminated, no trailing newline).
 * Returns false on EOF or empty line after trim. */
bool util_read_line(char *buf, size_t buf_size);

/* Prompt until user enters y/Y or n/N. Returns true for yes. */
bool util_prompt_yes_no(const char *prompt);

/* Prompt until user enters an integer in [min_val, max_val]. */
int util_prompt_int_range(const char *prompt, int min_val, int max_val);

/* Read one long integer from a prompted line.
 * Returns false on EOF or empty line; true and sets *out on a valid parse. */
bool util_prompt_long(const char *prompt, long *out);

/* Read one command line; return first non-space char (letters lowercased).
 * Returns 0 on EOF. */
char util_read_command_char(void);

#endif /* UTIL_INPUT_H */
