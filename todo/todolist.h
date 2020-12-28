#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#if !defined(TODO_LIST_H)
#define TODO_LIST_H

#define MAX_PENDING_CONNECTIONS 5
#define FOREVER 1
#define SERVICE_NAME "5050"
#define COMMAND_SIZE 128
#define BUFFER_SIZE 256
#define FATAL 1
#define MINOR 0

struct todo_node
{
    char *content;
    char *author;
    time_t add_time;
    time_t deadline_time;
    struct todo_node *next;
};

uint8_t word_count(const char *buffer);
void sys_err(const char *source, int flags);
void user_err(const char *source, const char *detail, int flags);
int setup_client_socket(const char *host, const char *servive);
int setup_server_socket(in_addr_t ip, in_port_t port);
void clear();
void print_addr_info(struct sockaddr *addr, FILE *stream);

#endif // TODO_LIST_H
