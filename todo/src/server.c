#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#include "data.h"
#include "utils.h"
#include "server.h"
#include "network.h"

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

void read_msg(int client_sock, char *buffer, size_t buff_len)
{
    ssize_t n;
    while ((n = read(client_sock, buffer, buff_len)) > 0)
    {
        buffer[n] = 0;
    }
    close(client_sock);
}

bool parse_cmd(char *bytes, int *cmd)
{
    const char res = '|';
    char *cmd_str = strtok(bytes, &res);
    if (cmd_str == NULL)
        return false;

    if (strncmp(CMD_ADD, cmd_str, CMD_SIZE) == 0)
        *cmd = ADD;
    else if (strncmp(CMD_RMV, cmd_str, CMD_SIZE) == 0)
        *cmd = RMV;
    else
        *cmd = INV;

    return (bool)cmd;
}

void validate_args(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "%s <PORT> <DB NAME> (<--resetdb>)\n", *argv);
        user_err("validate_args()", "Invalid amount of arguments",
            FATAL);
    }
}

bool parse_next(char *bytes, char *dest, size_t max_len)
{
    const char res = '|';
    char * content = strtok(bytes, &res);
    size_t len = strlen(content);
    if (content == NULL || len > max_len -1)
        return false;
    strcpy(dest, content);
    return true;
}

bool parse(char* bytes, user_t *user, char *content_str, int *cmd)
{
    if (!parse_cmd(bytes, cmd))
    {
        user_err("parse_cmd()", "Could not parse user's command.",
            NON_FATAL);
        return false;
    }
    if (!parse_next(bytes, user->name, S_NETWORKBUF_SIZE))
    {
        user_err("parse_next()", "Could not parse user's name.",
            NON_FATAL);
        return false;
    }
    if (!parse_next(bytes, content_str, L_NETWORKBUF_SIZE))
    {
        user_err("parse_next()", "Could not parse message content.", 
            NON_FATAL);
        return false;
    }
    return true;
}

int detect_user(user_t *user, sqlite3 *db)
{
    // NULL default is needed to check
    // if the query produced a result.
    user_t *temp_result = NULL;
    int rc = db_select_user(user, db, temp_result);
    user = temp_result;
    return rc;
}


void create_task(task_t *task, user_t *user, char *content)
{
    strncpy(task->content, content, L_NETWORKBUF_SIZE);
    task->author = user;
    task->timestamp = time(NULL);
}

bool add_task(user_t *user, char *content, sqlite3 *db)
{
    task_t task;
    create_task(&task, user, content);
    int rows = 0;
    if (db_insert_task(&task, db, &rows) != DB_SUCCESS)
    {
        // Inserting the task failed
        return false;
    }
    return rows == 1;
}


bool handle_msg(char *buffer, char *db_name)
{
    int cmd = INV; // Default to invalid
    char *content = NULL;
    user_t user;

    // Here we parse the buffer contents to data structs
    if (!parse(buffer, &user, content, &cmd))
    {
        return false;
    }

    // Open databse connetction
    sqlite3 *db;
    if (db_open(db_name, &db) != DB_SUCCESS)
    {
        user_err("handle_msg()", "Could not open database.", NON_FATAL);
        return false;
    }

    if (detect_user(&user, db) == DB_NO_RESULT)
    {
        // TODO: Handle user not found in the database.
    }

    // Presume that user is valid, execute the command
    bool rc = false;
    switch (cmd)
    {
    case ADD:
        // Create new task and insert it to database.
        rc = add_task(&user, content, db);
    case RMV:
        // Remove task from database.
        rc = false;
    default:
        // This should never run.
        rc = false;
    }
    sqlite3_close(db);
    return rc;
}

int parse_config(int argc, char **argv)
{
    int result = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--resetdb") == 0)
        {
            result |= CONF_RESET_DB;
        }
    }
    return result;
}

void init_server(struct server_config *configs, int argc, char **argv, int *sock)
{
    configs->port = atoi(argv[1]);
    strncpy(configs->db_name, argv[2], S_NETWORKBUF_SIZE);
    configs->max_pending = 5; // TODO: cmd arg
    configs->flags = parse_config(argc, argv);
    if (configs->flags & CONF_RESET_DB)
    {
        printf("restarting database..\n");
        sqlite3 *db;
        db_open(configs->db_name, &db);
        db_init(db);
        sqlite3_close(db);
    }

    fprintf(stdout, "%s\n", "Starting the todo server...");
    *sock = setup_server_socket(INADDR_ANY,
        configs->port, configs->max_pending);
    print_port(*sock, "Listening at port: %u.\n", stdout);
}

int main(int argc, char **argv)
{
    validate_args(argc, argv);
    struct server_config server_conf;
    char buffer[L_NETWORKBUF_SIZE];
    int server_sock;
    init_server(&server_conf, argc, argv, &server_sock);
    for (;;)
    {
        int client_sock = accept_connection(server_sock);
        read_msg(client_sock, buffer, L_NETWORKBUF_SIZE);
        handle_msg(buffer, server_conf.db_name);
    }
}