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
struct args* parse_args(char** argv, int arg_count)
{
    struct args* user_args;
    user_args = (struct args*) malloc(sizeof(struct args));
   
    // Set echo and ip args.
    if (strlen(argv[2]) <= ECHO_SIZE)
    {
        user_args->echo = argv[2];
    }
    else
    {
        p_user_err("Size error", "Echo string is too long.");
        exit(1);
    }
    if (strlen(argv[1]) <= IPV4_CHAR_SIZE)
    {
        user_args->ip = argv[1];
    }
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
    {
        user_args->port = DEFAULT_PORT;
    }

    return user_args;
}


int main(int argc, char* argv[])
{
    if (argc < 3 || argc < 4)
    {
        p_user_err("Param(s)", "<Address> <Echo String> (<Port>)");
        return 1;
    }
    
    struct args* user_args = parse_args(argv, argc);
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        p_sys_err("Couldn't create a socket!");
        return 1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    // Set address
    int success = inet_pton(AF_INET, user_args->ip, &server_addr.sin_addr.s_addr);
    if (success == 0)
    {
        p_user_err("inet_pton() failed", "invalid address.");
        return 1;
    }
    else if (success < 0)
    {
        p_sys_err("inet_pton() failed");
        return 1;
    }
    // Set port
    server_addr.sin_port = user_args->port;
    
    // Start connection
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        p_sys_err("connect() failed.");
    }

    // Send message to server
    size_t echo_size = strlen(user_args->echo);
    ssize_t byte_size = send(sock, user_args->echo, echo_size, 0);
    if (byte_size < 0)
    {
        p_sys_err("send() failed.");
    }
    else if (byte_size != echo_size)
    {
        p_user_err("send()", "Sent unexpected number of bytes.");
    }
    
    // Receive echo from server
    unsigned int total_bytes_rcvd = 0;
    fputs("Received: ", stdout);
    while (total_bytes_rcvd < byte_size)
    {
        char buffer[ECHO_SIZE];
        byte_size = recv(sock, buffer, ECHO_SIZE-1, 0);
        if (byte_size < 0)
        {
            p_sys_err("recv() failed");
        }
        else if (byte_size == 0)
        {
            p_user_err("recv() failed", "Connection closed too early");
        }
        total_bytes_rcvd += byte_size;
        fputs(buffer, stdout);
    }

    fputc('\n', stdout);
    return 0;
}

