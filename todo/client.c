#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "todolist.h"

int connect_to_server(const char *hostname, const char *port)
{
    printf("Connecting to server: %s...\n", hostname);
    int sock = setup_client_socket(hostname, port);
    if (sock > 0)
    {
        printf("%s\n", "Connected!");
    }
    else
    {
        printf("Failed to connect: %s!\n", hostname);
        exit(EXIT_FAILURE);
    }
    return sock;
}

void add_item(int sock)
{
    fputs("Add task to todo list: ", stdout);
    char task_buffer[BUFFER_SIZE];
    fgets(task_buffer, BUFFER_SIZE, stdin);
    size_t len = strlen(task_buffer) - 1;
    if (task_buffer[len] == '\n')
        task_buffer[len] = '\0';

    size_t task_str_size = strlen(task_buffer) + 1;
    printf("Sending item (%s) to server...\n", task_buffer);
    ssize_t sent_bytes = send(sock, task_buffer, task_str_size, 0);
    if (sent_bytes < 0)
        sys_err("send()", FATAL);
    else if (sent_bytes != task_str_size)
        user_err("send()", "sent unexpected number of bytes", FATAL);
    fputs("Sent!\n", stdout);
}

void quit()
{
    printf("%s\n", "Closing connection");
    exit(EXIT_SUCCESS);
}

void bad_cmd(char buffer[])
{
    printf("Undefined command: %s\n", buffer);
}

void user_input(char buffer[], int sock)
{
    fputs("Input : ", stdout);
    fgets(buffer, COMMAND_SIZE, stdin);
    size_t len = strlen(buffer) - 1;
    if (buffer[len] == '\n')
        buffer[len] = '\0';

    if (strcmp(buffer, "add") == 0)
        add_item(sock);
    else if (strcmp(buffer, "quit") == 0)
        quit();
    else if (strcmp(buffer, "clear") == 0)
        clear();
    else
        bad_cmd(buffer);
}

int main(int argc, char **argv)
{
    if (argc != 3)
        user_err("Params", "<HOST>", FATAL);
    const char *hostname = argv[1];
    const char *port = argv[2];

    int sock = connect_to_server(hostname, port);
    char command_buffer[COMMAND_SIZE];
    while (FOREVER)
    {
        user_input(command_buffer, sock);
    }
}