/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  TCP/IP echo server.
 *
 *        Version:  1.0
 *        Created:  12/19/20 14:23:18
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Niklas Seppala, 
 *   Organization:  Metropolia AMK
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_CONN_COUNT 5
#define FATAL 1
#define MINOR 0
#define FOREVER 1

extern int errno;

void user_err(const char* source, const char* message, int err_flag)
{
    fputs(source, stderr);
    fputs(": ", stderr);
    fputs(message, stderr);
    fputc('\n', stderr);
    if (err_flag == FATAL)
        exit(1);
}

void sys_err(const char* message, int err_flag)
{
    perror(message);
    if (err_flag == FATAL)
        exit(1);
}

void create_socket(int* sock)
{
    *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ((*sock) < 0)
        sys_err("socket() failed", FATAL);
}

void create_address(struct sockaddr_in* addr, in_port_t port)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = port;
}

void handle_client(int client_sock)
{
    char buffer[256];
    ssize_t bytes_received = recv(client_sock, buffer, 256, 0);
    if (bytes_received < 0)
        sys_err("recv() failed", FATAL);
    
    while (bytes_received > 0)
    {
        ssize_t bytes_sent = send(client_sock, buffer, bytes_received, 0);
        if (bytes_sent < 0)
            sys_err("send() failed", FATAL);
        else if (bytes_sent != bytes_received)
            user_err("send()", "unexpected num of bytes sent.", MINOR);

        bytes_received = recv(client_sock, buffer, 256, 0);
        if (bytes_received < 0)
            sys_err("recv() failed", FATAL);
    }
    close(client_sock);
}


int main(int argc, char** argv)
{
    // Validate argument count.
    if (argc != 2)
        user_err("Parameter(s)", "<Port>", FATAL);
    
    // Parse port argument.
    long parsed_port = strtol(argv[1], NULL, 0);
    if (errno != 0)
        sys_err("Port parameter could not be parsed.", FATAL);
    in_port_t port = htons(parsed_port);
    
    // Create the socket.
    int server_sock = 0;
    create_socket(&server_sock);
   
    // Create address.
    struct sockaddr_in server_addr;
    create_address(&server_addr, port);

    // Bind socket to local address.
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        sys_err("socket binding failed", FATAL);
    
    // Set server socket to listen incoming connections.    
    if (listen(server_sock, MAX_CONN_COUNT) < 0)
        sys_err("listen() failed: ", FATAL);

    while (FOREVER)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock < 0)
            sys_err("accept() failed!", FATAL);

        char client_name[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_name, sizeof(client_name)) != NULL)
            printf("Client: %s%d\n", client_name, ntohs(client_addr.sin_port));
        else
            printf("%s", "Unable to get client address!");

        handle_client(client_sock);
    }
    return 0;
}

