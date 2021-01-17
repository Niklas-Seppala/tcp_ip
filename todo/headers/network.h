#if !defined NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <stdio.h>

#define S_NETWORKBUF_SIZE 64
#define L_NETWORKBUF_SIZE 512

void print_port(int sock, const char* template_str, FILE *stream);
void print_sock(int sock, FILE *stream);
void print_addr_info(struct sockaddr *addr, FILE *stream);
void print_peer(int sock);

int setup_client_socket(const char *host, const char *servive);
int setup_server_socket(in_addr_t ip, in_port_t port, int queue_size);

#endif