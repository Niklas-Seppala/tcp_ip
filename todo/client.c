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
    int sock = setup_client_socket(hostname, port);
    return sock;
}

void append_msg(char *msg, char* item, size_t offset, size_t len, size_t* next_offset)
{
    str_ins(msg, item, offset, len);
    *next_offset = offset + len;
    msg[*next_offset] = '|';
    (*next_offset)++;
}

void create_msg(char *msg, char *task_buffer, char *username, size_t *len)
{
    size_t next_offset = 0;
    append_msg(msg, CMD_ADD, 0, CMD_SIZE, &next_offset);
    append_msg(msg, username, next_offset, strlen(username), &next_offset);
    append_msg(msg, task_buffer, next_offset, strlen(task_buffer), &next_offset);
    *len = next_offset-1; // remove last '|' from the message
}

void send_msg(const char *hostname, const char *port, char *msg, size_t msg_len)
{
    printf("Sending item \"%.*s\" to server...\n", (int)msg_len, msg);

    int sock = connect_to_server(hostname, port);
    ssize_t sent_bytes = write(sock, msg, msg_len);
    if (sent_bytes < 0)
    {
        sys_err("write()");
    }
    else if (sent_bytes != msg_len)
    {
        user_err("write()", "sent unexpected number of bytes");
    }
    printf("Sent!\n");
    close(sock);
}

void bad_cmd(const char* buffer)
{
    printf("Undefined command: %s\n", buffer);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        user_err("Params", "<HOST> <SERVICE>");
    }
    const char *hostname = argv[1];
    const char *port = argv[2];
    const int tbuffer_size = BUFFER_SIZE - BUFFER_OFFSET;
    char task_buffer[tbuffer_size];
    char cmd_buffer[SBUFFER_SIZE];
    char username[SBUFFER_SIZE];
    char msg[BUFFER_SIZE];

    get_input("Who are you? :", username, SBUFFER_SIZE, NULL);

    for(;;)
    {
        get_input("Input: ", cmd_buffer, SBUFFER_SIZE, NULL);
        if (strcmp(cmd_buffer, "add") == 0)
        {
            get_input("Add task to todo list: ", task_buffer,
                       tbuffer_size, NULL);
            size_t msg_len = 0;
            create_msg(msg, task_buffer, username, &msg_len);
            send_msg(hostname, port, msg, msg_len);
        }
        else if (strcmp(cmd_buffer, "quit") == 0) 
        {
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(cmd_buffer, "clear") == 0) 
        {
            clear();
        }
        else
        {
            bad_cmd(cmd_buffer);
        }
    }
}