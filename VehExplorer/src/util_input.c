#include "util_input.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void trim_inplace(char *s)
{
    char *start = s;
    char *end;
    size_t len;

    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    len = strlen(s);
    if (len == 0) {
        return;
    }

    end = s + len - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

bool util_read_line(char *buf, size_t buf_size)
{
    size_t len;

    if (buf == NULL || buf_size == 0) {
        return false;
    }

    if (fgets(buf, (int)buf_size, stdin) == NULL) {
        buf[0] = '\0';
        return false;
    }

    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        /* Line longer than buffer: drain remainder so next read is clean. */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* discard */
        }
    }

    trim_inplace(buf);
    return buf[0] != '\0';
}

bool util_prompt_yes_no(const char *prompt)
{
    char line[64];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (!util_read_line(line, sizeof line)) {
            if (feof(stdin)) {
                return false;
            }
            printf("Please enter y or n.\n");
            continue;
        }

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

int util_prompt_int_range(const char *prompt, int min_val, int max_val)
{
    char line[64];
    int value;
    char extra;

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (!util_read_line(line, sizeof line)) {
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

    if (!util_read_line(line, sizeof line)) {
        return false;
    }

    if (sscanf(line, "%ld%c", &value, &extra) != 1) {
        return false;
    }

    *out = value;
    return true;
}

char util_read_command_char(void)
{
    char line[64];
    size_t i;

    if (!util_read_line(line, sizeof line)) {
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
