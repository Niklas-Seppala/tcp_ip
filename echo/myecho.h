#ifndef MYECHO_H
#define MYECHO_H

#define DEFAULT_PORT 7
#define BUFFER_SIZE 256
#define E_FATAL 1
#define E_MINOR 2
#define FOREVER 1
#define MAX_CONN_COUNT 5

void user_err(const char* source, const char* details, int flag);
void sys_err(const char* message, int flag);
void create_socket(int* sock);

#endif
