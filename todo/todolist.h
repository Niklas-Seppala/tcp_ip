#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#if !defined(TODO_LIST_H)
#define TODO_LIST_H

#define CMD_SIZE 3
#define CMD_ADD "ADD"
#define CMD_RMV "RMV"
#define BUFFER_OFFSET 4
#define SBUFFER_SIZE 64
#define BUFFER_SIZE 512

// struct todo_task
// {
//     char author[SBUFFER_SIZE];
//     char content[BUFFER_SIZE];
//     time_t add_time;
//     struct todo_task *next;
// };

void sys_err(const char *source);
void user_err(const char *source, const char *detail);

void print_port(int sock, const char* template_str, FILE *stream);
void print_sock(int sock, FILE *stream);
void print_addr_info(struct sockaddr *addr, FILE *stream);

void get_input(const char* output, char *buffer, size_t buff_len, size_t *out_len);

void str_ins(char *dest, char *src, int offset, size_t len);

int setup_client_socket(const char *host, const char *servive);
int setup_server_socket(in_addr_t ip, in_port_t port, int queue_size);

void clear();

#endif // TODO_LIST_H
