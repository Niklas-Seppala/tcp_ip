#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "myecho.h"

extern int errno;

/**
 * Creates local address for the server. Uses IPv4
 * and accepts any possible address.
 *
 * @param addr pointer to sockaddr_in struct
 * @param port desired port
 *
 */
void create_address(struct sockaddr_in *addr, in_port_t port)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = port;
}

/**
 * Receives clients message and echos it back.
 * After that, closes the client socket.
 *
 * @param client_sock clients socket handle.
 */
void handle_client(int client_sock)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0)
        sys_err("recv() failed", E_FATAL);

    while (bytes_received > 0)
    {
        ssize_t bytes_sent = send(client_sock, buffer, bytes_received, 0);
        if (bytes_sent < 0)
            sys_err("send() failed", E_FATAL);
        else if (bytes_sent != bytes_received)
            user_err("send()", "unexpected num of bytes sent.", E_MINOR);

        bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0)
            sys_err("recv() failed", E_FATAL);
    }
    close(client_sock);
}

void log_connection(int sock)
{
    struct sockaddr_in client;
    socklen_t client_size = sizeof(client);
    getpeername(sock, (struct sockaddr *)&client, &client_size);

    char *client_ip = inet_ntoa(client.sin_addr);
    uint16_t client_port = ntohs(client.sin_port);

    printf("Serving client: %s:%u\n", client_ip, client_port);
}

/**
 * Accepts connection and prints client
 * address to console. If all is good,
 * calls client handling function.
 *
 * @param sock server socket handle
 */
void accept_conn(int sock)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_sock = accept(sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock < 0)
        sys_err("accept() failed: ", E_FATAL);
    else
        log_connection(client_sock);
    handle_client(client_sock);
}

/**
 * Parses the port from command line arguments.
 * If arguments are insufficient, exits the program.
 * 
 * @param argc command line argument count
 * @param argv command line arguments
 *
 * @return ready to use port value.
 */
in_port_t parse_port(int argc, char **argv)
{
    if (argc != 2)
        user_err("Parameter(s)", "<Port>", E_FATAL);

    long parsed_port = strtol(argv[1], NULL, 0);
    if (errno != 0)
        sys_err("Port parameter could not be parsed", E_FATAL);
    return htons(parsed_port);
}

void main(int argc, char **argv)
{
    in_port_t port = parse_port(argc, argv);

    int server_sock = 0;
    create_socket(&server_sock);
    struct sockaddr_in server_addr;
    create_address(&server_addr, port);

    // Bind socket to local address.
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        sys_err("socket binding failed", E_FATAL);

    // Set server socket to listen incoming connections.
    if (listen(server_sock, MAX_CONN_COUNT) < 0)
        sys_err("listen() failed: ", E_FATAL);

    // Start accepting connections and handle them accordingly.
    while (FOREVER)
        accept_conn(server_sock);
}
