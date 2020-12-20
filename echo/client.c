#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "myecho.h"

extern int errno;

/**
 * structure that holds command
 * line arguments for this client
 * program.
 */
struct args
{
    char *echo;
    char *ip;
    in_port_t port;
};

/* 
 * Creates agrument data structure from
 * command line arguments.
 *
 * @param argv command line argument vector
 * @param arg_count command line argument count
 * @param user_args pointer to user arguments
 */
void parse_args(char **argv, int arg_count, struct args *user_args)
{
    // Set echo and ip args.
    if (strlen(argv[2]) <= BUFFER_SIZE)
        user_args->echo = argv[2];
    else
        user_err("Size error", "Echo string is too long.", E_FATAL);

    if (strlen(argv[1]) <= INET_ADDRSTRLEN)
        user_args->ip = argv[1];
    else
        user_err("Size error", "Server ip is invalid", E_FATAL);

    // Determines if client is using user defined port or default.
    if (arg_count == 4)
    {
        long parsed_port = strtol(argv[3], NULL, 10);
        if (errno != 0)
            user_err("Parsing error", "port could not be parsed", E_FATAL);

        user_args->port = htons(parsed_port);
    }
    else
        user_args->port = DEFAULT_PORT;
}

/**
 * Fills the address data to sockaddr struct
 * from command line parameters. 
 */
void create_address(struct sockaddr_in *server_addr, struct args *args)
{
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;

    int success;
    success = inet_pton(AF_INET, args->ip, &server_addr->sin_addr.s_addr);
    if (success == 0)
        user_err("inet_pton() failed", "invalid address.", E_FATAL);
    else if (success == -1)
        sys_err("inet_pton failed", E_FATAL);

    server_addr->sin_port = args->port;
}

/**
 * Sends message to server.
 *
 * @param echo message string
 * @param size size of the message
 * @param sock socket handle
 *
 * @return the amount of bytes sent to server
 */
ssize_t send_echo(const char *echo, size_t size, const int *sock)
{
    ssize_t bytes_sent = send(*sock, echo, size, 0);
    if (bytes_sent < 0)
        sys_err("send() failed.", E_FATAL);
    else if (bytes_sent != size)
        user_err("send()", "Unexpected amount of bytes sent.", E_MINOR);

    return bytes_sent;
}

/**
 * Waits for echo to return from the server, then
 * prints it to console and closes the socket.
 *
 * @param sock socket handle
 * @param bytes_set the amount of bytes sent to server
 */
void receive_echo(const int *sock, ssize_t bytes_sent)
{
    ssize_t bytes = 0;
    unsigned int bytes_received = 0;

    fputs("Response from server: ", stdout);
    while (bytes_received < bytes_sent)
    {
        char buffer[BUFFER_SIZE];
        bytes = recv(*sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes < 0)
            sys_err("recv() failed.", E_FATAL);
        else if (bytes == 0)
            user_err("recv() failed.", "Connection closed", E_FATAL);

        bytes_received += bytes;
        buffer[bytes_received] = '\0'; // Terminate the string.
        fputs(buffer, stdout);
    }
    fputc('\n', stdout);
}

int main(int argc, char **argv)
{
    // Validate command line arguments and parse them to
    // args struct.
    if (argc < 3 || argc > 4)
        user_err("Param(s)", "<Address> <Echo String> (<Port>)", E_FATAL);

    struct args user_args;
    parse_args(argv, argc, &user_args);

    // Create the socket and configure address
    int sock = 0;
    create_socket(&sock);
    struct sockaddr_in server_addr;
    create_address(&server_addr, &user_args);

    // Start connection
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        sys_err("connect() failed.", E_FATAL);

    // Send message to server
    ssize_t bytes_sent = send_echo(user_args.echo, strlen(user_args.echo), &sock);

    // Receive echo from server
    receive_echo(&sock, bytes_sent);
    close(sock);
    return 0;
}
