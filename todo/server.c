#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

#include "todolist.h"

int accept_connection(int server_sock)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock < 0)
    {
        sys_err("accept()", FATAL);
    }

    // fputs("Handling client ", stdout);
    // print_addr_info((struct sockaddr *)&client_addr, stdout);

    return client_sock;
}

void handle_client(int client_sock, char *message)
{
    char current;
    ssize_t read_bytes;
    while ((read_bytes = recv(client_sock, &current, 1, 0)) > 0)
    {
        if (read_bytes == -1)
            sys_err("recv()", FATAL);
        *message++ = current;
        if (current != '\0')
            break;
    }
}

int main(int argc, char **argv)
{
    char *port_str = argc == 2 ? argv[1] : SERVICE_NAME;

    printf("%s\n", "Starting the server...");
    int server_sock = setup_server_socket(INADDR_LOOPBACK, atoi(port_str));
    if (server_sock < 0)
        user_err("setup_server_socket()", "failed to create a socket!", FATAL);

    char buffer[BUFFER_SIZE];
    while (FOREVER)
    {
        int client_sock = accept_connection(server_sock);
        printf("client sock: %d\n", client_sock);
        handle_client(client_sock, buffer);
        printf("%s\n", buffer);
    }
}