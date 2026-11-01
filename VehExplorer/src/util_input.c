#include "util_input.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Remove leading and trailing whitespace characters from a string in-place. */
static void trim_inplace(char *s)
{
    char *start = s;
    char *end;
    size_t len;

    /* Advance past leading spaces or tabs */
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    /* Shift remaining characters to the front of the buffer */
    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    len = strlen(s);
    if (len == 0) {
        return;
    }

    /* Replace trailing spaces/newlines at the end with null terminators */
    end = s + len - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* Read one bounded line from stdin, strip newline/spaces, and flush overflow. */
bool util_read_line(char *buf, size_t buf_size)
{
    size_t len;

    if (buf == NULL || buf_size == 0) {
        return false;
    }

    /* Read up to buf_size - 1 characters safely with fgets */
    if (fgets(buf, (int)buf_size, stdin) == NULL) {
        buf[0] = '\0';
        return false;
    }

    /* Remove trailing newline, or discard extra characters if line was too long */
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* discard leftover characters on this line */
        }
    }

    trim_inplace(buf);
    return buf[0] != '\0';
}

/* Keep prompting until the user enters 'y' or 'n'. */
bool util_prompt_yes_no(const char *prompt)
{
    char line[64];

    while (true) {
        printf("%s", prompt);
        fflush(stdout);

        if (!util_read_line(line, sizeof(line))) {
            if (feof(stdin)) {
                return false;
            }
            printf("Please enter y or n.\n");
            continue;
        }

        /* Accept single-letter y/Y or n/N */
        if (strlen(line) == 1) {
            char c = (char)tolower((unsigned char)line[0]);
            if (c == 'y') {
                return true;
            }
            if (c == 'n') {
                return false;
            }
        }

        printf("Please enter y or n.\n");
    }
}

/* Prompt for y/n, using default_yes if the user presses Enter on an empty line. */
bool util_prompt_yes_no_default(const char *prompt,
                                bool default_yes,
                                bool *out)
{
    char line[64];

    if (out == NULL) {
        return false;
    }

    while (true) {
        printf("%s", prompt);
        fflush(stdout);

        /* Empty line uses default value; EOF aborts */
        if (!util_read_line(line, sizeof(line))) {
            if (feof(stdin)) {
                return false;
            }
            *out = default_yes;
            return true;
        }

        if (strlen(line) == 1) {
            char c = (char)tolower((unsigned char)line[0]);
            if (c == 'y') {
                *out = true;
                return true;
            }
            if (c == 'n') {
                *out = false;
                return true;
            }
        }

        printf("Please enter y or n (or Enter for default).\n");
    }
}

/* Prompt for a whole number >= min_val, or accept default_val on empty Enter. */
bool util_prompt_long_default(const char *prompt,
                              long min_val,
                              long default_val,
                              long *out)
{
    char line[64];
    long value;
    char extra;

    if (out == NULL) {
        return false;
    }

    while (true) {
        printf("%s", prompt);
        fflush(stdout);

        if (!util_read_line(line, sizeof(line))) {
            if (feof(stdin)) {
                return false;
            }
            *out = default_val;
            return true;
        }

        /* Ensure the line contains a valid number with no trailing characters */
        if (sscanf(line, "%ld%c", &value, &extra) == 1 && value >= min_val) {
            *out = value;
            return true;
        }

        printf("Please enter a number >= %ld (or Enter for default).\n",
               min_val);
    }
}

/* Prompt until the user enters an integer between min_val and max_val. */
int util_prompt_int_range(const char *prompt, int min_val, int max_val)
{
    char line[64];
    int value;
    char extra;

    while (true) {
        printf("%s", prompt);
        fflush(stdout);

        if (!util_read_line(line, sizeof(line))) {
            if (feof(stdin)) {
                return min_val;
            }
            printf("Please enter a number from %d to %d.\n", min_val, max_val);
            continue;
        }

        if (sscanf(line, "%d%c", &value, &extra) == 1 &&
            value >= min_val && value <= max_val) {
            return value;
        }

        printf("Please enter a number from %d to %d.\n", min_val, max_val);
    }
}

/* Ask once for a long integer; return false if empty or not a valid integer. */
bool util_prompt_long(const char *prompt, long *out)
{
    char line[64];
    long value;
    char extra;

    if (out == NULL) {
        return false;
    }

    printf("%s", prompt);
    fflush(stdout);

    if (!util_read_line(line, sizeof(line))) {
        return false;
    }

    if (sscanf(line, "%ld%c", &value, &extra) != 1) {
        return false;
    }

    *out = value;
    return true;
}

/* Read a command line and return the first non-space character in lowercase. */
char util_read_command_char(void)
{
    char line[64];
    size_t i;

    if (!util_read_line(line, sizeof(line))) {
        return '\0';
    }

    for (i = 0; line[i] != '\0'; i++) {
        unsigned char ch = (unsigned char)line[i];
        if (!isspace(ch)) {
            return (char)tolower(ch);
        }
    }

    return '\0';
}
