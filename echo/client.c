/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  TCP/IP client for echo server. This is 
 *                  pretty cool :3
 *
 *        Version:  1.0
 *        Created:  12/18/20 16:32:01
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Niklas Seppala, 
 *   Organization:  Metropolia AMK
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 7
#define ECHO_SIZE 256
#define IPV4_CHAR_SIZE 15

extern int errno;

void p_user_err(const char* message, const char *details)
{
    fputs(message, stderr);
    fputs(": ", stderr);
    fputs(details, stderr);
    fputc('\n', stderr);
}

void p_sys_err(const char* message)
{
    perror(message);
}


struct args 
{
    char* echo;
    char* ip;
    in_port_t port;
};

/* 
 * Creates agrument data structure from
 * command line arguments.
 */
void parse_args(char** argv, int arg_count, struct args* user_args)
{   
    // Set echo and ip args.
    if (strlen(argv[2]) <= ECHO_SIZE)
        user_args->echo = argv[2];
    else
    {
        p_user_err("Size error", "Echo string is too long.");
        exit(1);
    }

    if (strlen(argv[1]) <= IPV4_CHAR_SIZE)
        user_args->ip = argv[1];
    else
    {
        p_user_err("Size error", "Server ip is invalid");
        exit(1);
    }

    // Determines if client is using user defined port or default.    
    if (arg_count == 4)
    {
        long parsed_port = strtol(argv[3], NULL, 10);
        if (errno != 0)
        {
            p_user_err("Parsing error", "port could not be parsed");
            exit(1);
        }
        user_args->port = htons(parsed_port);
    }
    else
        user_args->port = DEFAULT_PORT;
}

void create_socket(int* sock)
{
    *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ((*sock) < 0)
    {
        p_sys_err("Could not create a socket!");
        exit(1);
    }
}

void create_address(struct sockaddr_in* server_addr, struct args* args)
{
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    int success;
    success = inet_pton(AF_INET, args->ip, &server_addr->sin_addr.s_addr);
    if (success == 0)
    {
        p_user_err("inet_pton() failed", "invalid address.");
        exit(1);
    }
    else if (success == -1)
    {
        p_sys_err("inet_pton failed");
        exit(1);
    }
    server_addr->sin_port = args->port;
}

ssize_t send_echo(const char* echo, size_t size, const int* sock)
{
    ssize_t bytes_sent = send(*sock, echo, size, 0);
    if (bytes_sent < 0)
    {
        p_sys_err("send() failed.");
        exit(1);
    }
    else if (bytes_sent != size)
    {
        p_user_err("send()", "Unexpected amount of bytes sent.");
    }
    return bytes_sent;
}

void receive_echo(const int* sock, ssize_t bytes_sent)
{
    ssize_t bytes = 0;
    unsigned int bytes_received = 0;

    fputs("response from server:\n    ", stdout);
    while (bytes_received < bytes_sent)
    {
        char buffer[ECHO_SIZE];
        bytes = recv(*sock, buffer, ECHO_SIZE-1, 0);
        if (bytes < 0)
        {
            p_sys_err("recv() failed.");
            exit(1);
        }
        else if (bytes == 0)
        {
            p_user_err("recv() failed.", "Connection closed");
            exit(1);
        }
        bytes_received += bytes;
        fputs(buffer, stdout);
    }
    fputc('\n', stdout);
}

int main(int argc, char* argv[])
{
    // Validate command line arguments and parse them to
    // args struct.
    if (argc < 3 || argc < 4)
    {
        p_user_err("Param(s)", "<Address> <Echo String> (<Port>)");
        return 1;
    }
    struct args user_args;
    parse_args(argv, argc, &user_args);
    

    printf("%s\n", user_args.ip);
    printf("%s\n", user_args.echo);
    printf("%#08x\n", user_args.port);

    // Create the socket and configure address
    //int sock = 0;
    //create_socket(&sock);
    //struct sockaddr_in server_addr;
    //create_address(&server_addr, &user_args);
    
    // Start connection
    //if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    //{
    //    p_sys_err("connect() failed.");
    //   return 1;
    //}

    // Send message to server
    //size_t echo_size = strlen(user_args.echo);
    //ssize_t bytes_sent = send_echo(user_args.echo, echo_size, &sock);

    
    // Receive echo from server
    //receive_echo(&sock, bytes_sent);
    return 0;
}

