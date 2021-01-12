#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#include "data.h"
#include "todolist.h"

static const int MAX_PENDING = 5;

int accept_connection(int server_sock)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock < 0)
    {
        sys_err("accept()");
    }

    fputs("Handling client ", stdout);
    print_addr_info((struct sockaddr *)&client_addr, stdout);
    return client_sock;
}

void handle_client(int client_sock, char *buffer, size_t buff_len)
{
    ssize_t n;
    while ((n = read(client_sock, buffer, buff_len)) > 0)
    {
        buffer[n] = 0;
    }
    close(client_sock);
}

int parse_cmd(char *recv_bytes, char *cmd)
{
    const char res = '|';
    char *usr_command = strtok(recv_bytes, &res);
    if (usr_command == NULL)
        return -1;
    memcpy(cmd, usr_command, strlen(usr_command));
    return 0;
}

int parse_bytes(char *recv_bytes, task_t *task)
{
    char usr_command[CMD_SIZE+1]; // room for null char
    if (parse_cmd(recv_bytes, usr_command) < 0)
    {
        user_err("parse_cmd()", "Could not parse user's command");
    }

    if (strcmp(usr_command, CMD_ADD) == 0)
    {
        const char res = '|';
        char *author = strtok(NULL, &res);
        if (author != NULL)
        {
            memcpy(task->author, author, strlen(author));
        }
        char *task_content = strtok(NULL, &res);
        if (task_content != NULL)
        {
            memcpy(task->content, task_content, strlen(task_content));
        }
        return 1;
    }
    else if (strcmp(usr_command, CMD_RMV) == 0)
    {
        printf("remove command received\n");
        return 1;
    }
    return -1;
}


int store_task(task_t **head, char* recv_bytes)
{
    task_t* task = malloc(sizeof(task_t));
    if (!parse_bytes(recv_bytes, task))
    {
        user_err("recv_msg()", "could not create a task");
        return;
    }
    task->add_time = time(NULL);
    return ll_add_node(head, task);
}

int main(int argc, char **argv)
{
    char buffer[BUFFER_SIZE];
    task_t *task_list = NULL;

    in_port_t port = atoi(argv[1]);

    fprintf(stdout, "%s\n", "Starting the todo server...");
    int s_sock = setup_server_socket(INADDR_ANY, port, MAX_PENDING);
    fprintf(stdout, "%s ", "Done!");
    print_port(s_sock, "Listening at port: %u.\n", stdout);

    for (;;) {
        int c_socket = accept_connection(s_sock);
        handle_client(c_socket, buffer, BUFFER_SIZE);
        store_task(&task_list, buffer);
    }
}