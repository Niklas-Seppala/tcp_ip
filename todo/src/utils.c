#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "data.h"

void get_input(const char* output, char *buffer, size_t buff_len, size_t *out_len)
{
    if (output != NULL) {
        fputs(output, stdout);
    }
    fgets(buffer, buff_len, stdin);
    size_t len = strlen(buffer);
    if (buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        if (out_len != NULL) {
            *out_len = len-1;
        }
    } else if (out_len != NULL) {
        *out_len = len;
    }
}

void str_ins(char *dest, char *src, int offset, size_t len) {
    dest += offset;
    for (size_t i = 0; i < len; i++)
        *dest++ = *src++;
}


/**
 * @brief Prints user related error to console. Exits program
 * with EXIT_FAILURE.
 * 
 * @param source source function
 * @param detail description message
 */
void user_err(const char *source, const char *detail, int flag)
{
#ifdef DEBUG
    fputs(source, stderr);
    fputs(": ", stderr);
#endif
    fputs(detail, stderr);
    fputc('\n', stderr);
    if (flag == FATAL)
    {
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints syste related error to console. Exits the program
 * with EXIT_FAILURE.
 *
 * @param source description message
 */
void sys_err(const char *source)
{
    perror(source);
    exit(EXIT_FAILURE);
}


void clear()
{
#ifdef unix
    system("clear");
#elif _WIN32
    system("cls");
#endif
}
