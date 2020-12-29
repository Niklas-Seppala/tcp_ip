#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

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
    while ((n = read(client_sock, buffer, buff_len)) > 0) {
        buffer[n] = 0;
    }
    close(client_sock);
}

void free_tasks(struct todo_task **head) {
    if (head == NULL) {
        return;
    }

    struct todo_task *temp, *current;
    current = *head;
    while (current != NULL)
    {
        temp = current;
        current = current->next;
        free(temp);
    }
    *head = NULL;
}

void store_task(struct todo_task **head, const char* task_str) {

    struct todo_task* task = calloc(1, sizeof(struct todo_task));
    strcpy(task->content, task_str);
    task->add_time = time(NULL);

    if (*head == NULL) {
        *head = task;
        return;
    }

    struct todo_task* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = task;
}

void print_stored_tasks(struct todo_task *head, FILE *stream) {
    while (head != NULL) {
        fprintf(stream, "\t%s -- %ld\n", head->content, head->add_time);
        head = head->next;
    }
}

int main(int argc, char **argv) {
    char buffer[BUFFER_SIZE];
    struct todo_task *task_list = NULL;

    in_port_t port = atoi(argv[1]);

    fprintf(stdout, "%s\n", "Starting the todo server...");
    int s_sock = setup_server_socket(INADDR_ANY, port, MAX_PENDING);
    fprintf(stdout, "%s ", "Done!");
    print_port(s_sock, "Listening at port: %u.\n", stdout);

    for (;;) {
        int c_socket = accept_connection(s_sock);
        handle_client(c_socket, buffer, BUFFER_SIZE);
        store_task(&task_list, buffer);
        print_stored_tasks(task_list, stdout);
    }
}