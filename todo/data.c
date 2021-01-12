#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include <stdbool.h>
#include <time.h>

/***************** TASK LINKED LIST *******************/

/**
 * @brief Check if the first item of the
 *        linked list is used.
 * 
 * @param first linked list head.
 * @return true if head is used.
 * @return false if head is not used.
 */
bool ll_first_empty(task_t *first)
{
    return first->add_time < 0;
}

/**
 * @brief Prints all the tasks in provided
 *        linked list.
 * 
 * @param head list head
 * @param stream output stream
 */
void ll_print(task_t *head, FILE *stream)
{
    char date_buf[80];
    struct tm ts;
    while (head != NULL)
    {
        ts = *localtime(&(head->add_time));
        strftime(date_buf, sizeof(date_buf), "%a %Y-%m-%d %H:%M:%S", &ts);
        fprintf(stream, "Date: %s\n", date_buf);
        fprintf(stream, "%s\n", head->author);
        fprintf(stream, "%s\n\n", head->content);
        head = head->next;
    }
}

/**
 * @brief Get the last node of the linked list.
 * 
 * @param head linked list head
 * @return task_t* pointer to last node
 */
task_t *ll_last(task_t *head)
{
    task_t *current = head;
    while(current->next != NULL)
    {
        current = current->next;
    }
    return current;
}

/**
 * @brief Links the provided node at the end of the
 *        list. If list head is NULL, it will then
 *        point to provided node.
 * 
 * @param head First item in the list
 * @param node Node to be added
 * @return int 1 if success
 */
int ll_add_node(task_t **head, task_t *node)
{
    if (*head == NULL)
    {
        *head = node;
        return 1;
    }
    node->next = NULL;
    ll_last(*head)->next = node;
    return 1;
}

/**
 * @brief Execute provided operation for each item
 *        in the list.
 * 
 * @param head head of the linked list
 * @param operation operation that gets executed for each iteration
 */
void ll_foreach(task_t *head, void (*operation)(void *node))
{
    task_t *next;
    for (task_t *node = head; node != NULL; node = next)
    {
        // Save a copy of next node address,
        // in case something happens to current node.
        next = node->next;
        operation(node);
    }
}

/******************************************************/

/**
 * @brief Deserialize user id from database row.
 * 
 * @param id result id
 * @param count db row column count
 * @param values db row column values
 * @param col_name db row column names
 * 
 * @return int SQLITE_OK if success
 */
int deserialize_usrid(void *id, int count, char** values, char **col_name)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(col_name[i], "id") == 0)
        {
            *((int *)id) = atoi(values[i]);
        }
    }
    return SQLITE_OK;
}

/**
 * @brief Deserializes task struct from sqlite
 *        database row.
 * 
 * @param task task datastruct
 * @param count db row column count
 * @param values db row column values
 * @param col_name db row column names
 * 
 * @return int SQLITE_OK if all is OK
 */
int deserialize_task(task_t *task, int count, char **values, char **col_name)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(col_name[i], "content") == 0)
        {
            strcpy(task->content, values[i]);
        }
        else if (strcmp(col_name[i], "timestamp") == 0)
        {
            task->add_time = atoi(values[i]);
        }
    }
    return SQLITE_OK;
}

/**
 * @brief Create a task struct from database row.
 * 
 * @param list result list.
 * @param count row column count
 * @param values row column values
 * @param col_names row column names
 * 
 * @return DB_ERR if failure
 * @return SQLITE_OK if success
 */
int create_task(void* list, int count, char **values, char **col_names)
{
    if (list == NULL)
    {
        fprintf(stderr, "Error at create_task(): %s",
            "result head was null");
        return DB_ERR;
    }

    task_t *last_node = ll_last((task_t *)list);
    task_t *new_task;
    // Check if the list's first item is unused, if so
    // fill the data there. Otherwise allocate memory for
    // new struct.
    if (!ll_first_empty((task_t *)list))
    {
        // Initialize important fields and link the new item to the list.
        new_task = (task_t *)malloc(sizeof(task_t));
        new_task->next = NULL;
        new_task->add_time = -1;
        last_node->next = new_task;
    }
    else
    {
        new_task = last_node;
    }
    return deserialize_task(new_task, count, values, col_names);
}

/**
 * @brief Get the user id by username.
 * 
 * @param username username string
 * @param db sqlite3 database handle
 * @param id result id
 * 
 * @return DB_SUCCESS if recieved data
 * @return DB_NO_RESULT if recieved no data
 * @return DB_ERR if operations related to database failed
 */
int get_user_id(char *username, sqlite3 *db, int* id)
{
    *id = DB_NO_RESULT;
    char *db_err_msg = NULL;
    char *sql = sqlite3_mprintf("SELECT id FROM user WHERE name = '%q'", username);
    if (sqlite3_exec(db, sql, deserialize_usrid, id, &db_err_msg) != SQLITE_OK)
    {
        // Free allocated resources and return error code.
        fprintf(stderr, "SQL error: %s\n", db_err_msg);
        sqlite3_free(db_err_msg);
        sqlite3_free(sql);
        return DB_ERR;
    }
    else if (*id == DB_NO_RESULT)
    {
        sqlite3_free(sql);
        return DB_NO_RESULT;
    }
    sqlite3_free(sql);
    return DB_SUCCESS;
}

/**
 * @brief Get the tasks by username from database.
 * 
 * @param username username string
 * @param db sqlite3 database handle
 * @param results result linked list
 * 
 * @return int DB_CONN_ERR if database handle is invalid
 * @return int DB_NO_RESULT if query produced no empty results
 * @return int DB_ERR if query execution fails
 * @return int DB_SUCCCESS if everything OK
 */
int get_tasks_by_username(char *username, sqlite3 *db, task_t *results)
{
    // Check if the provided database handle is initialized.
    if (db == NULL)
    {
        return DB_CONN_ERR;
    }

    // Get users id by username.
    int author_id;
    get_user_id(username, db, &author_id);
    if (author_id == DB_NO_RESULT)
    {
        // No id was found by provided username.
        return DB_NO_RESULT;
    }

    char *db_err_msg = NULL;
    int rc = SQLITE_OK;
    // Mark result's head as unused.
    results->add_time = -1;
    char *sql = sqlite3_mprintf(
        "SELECT content, timestamp FROM task WHERE author = '%d'",
        author_id);
    rc = sqlite3_exec(db, sql, create_task, results, &db_err_msg);
    if (rc != SQLITE_OK)
    {
        // Possible sql error,
        // free allocated memory and return error code.
        fprintf(stderr, "SQL error: %s\n", db_err_msg);
        sqlite3_free(db_err_msg);
        sqlite3_free(sql);
        return DB_ERR;
    }
    sqlite3_free(sql);

    // Set author field to results.
    for (task_t *item = results; item != NULL; item = item->next)
    {
        strcpy(item->author, username);
    }
    return DB_SUCCESS;
}

/**
 * @brief Creates and executes SQL statement.
 * 
 * @param sql SQL string
 * @param db sqlite3 database handle
 * @param int rows_affected affected row count
 * 
 * @return int DB_SUCCESS if OK
 * @return int DB_ERR if fails
 */
int insert(char *sql, sqlite3 *db, int *affected_rows)
{
    char *db_err_msg;
    if (sqlite3_exec(db, sql, 0, 0, &db_err_msg) != SQLITE_OK) 
    {
        fprintf(stderr, "SQL error: %s\n", db_err_msg);
        sqlite3_free(db_err_msg);
        return DB_ERR;
    }
    if (affected_rows != NULL)
    {
        *affected_rows = sqlite3_changes(db);
    }
    return DB_SUCCESS;
}

/**
 * @brief Updates task fields to database.
 * 
 * @param task task struct
 * @param db sqlite3 database handle
 * @param int rows_affected affected row count
 * 
 * @return int DB_CONN_ERR if database handle is invalid
 * @return int DB__ERR if query execuion fails
 * @return int DB_SUCCESS if all is OK
 */
int update_task(task_t *task, sqlite3 *db, int *rows_affected)
{
    if (db == NULL)
    {
        return DB_CONN_ERR;
    }
    int id = DB_NO_RESULT;
    if (get_user_id(task->author, db, &id) != DB_SUCCESS)
    {
        return DB_ERR;
    }
    char *db_err_msg;
    char *sql = sqlite3_mprintf(
        "UPDATE task\
         SET author='%q', content='%q', timestamp='%d'\
         WHERE id='%q'",
            task->author, task->content,
            task->add_time, id
    );
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", db_err_msg);
        sqlite3_free(db_err_msg);
        sqlite3_free(sql);
        return DB_ERR;
    }
    if (rows_affected != NULL)
    {
        *rows_affected = sqlite3_changes(db);
    }
    sqlite3_free(sql);
    return DB_SUCCESS;
}

/**
 * @brief Inserts task datastruct values to sqlite3
 *        database.
 * 
 * @param task task struct
 * @param db sqlite3 database handle
 * @param int rows_affected affected row count
 * 
 * @return int DB_ERR if fails
 * @return int DB_SUCCESS if success
 */
int insert_task(task_t *task, sqlite3 *db, int *rows_affected)
{
    char *db_err_msg;
    char *sql = sqlite3_mprintf(
        "INSERT INTO task (content, author, timestamp) \
            VALUES ('%q', '%q', '%d')",
            task->content, task->author, task->add_time
    );

    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", db_err_msg);
        sqlite3_free(db_err_msg);
        sqlite3_free(sql);
        return DB_ERR;
    }
    if (rows_affected != NULL)
    {
        *rows_affected = sqlite3_changes(db);
    }
    sqlite3_free(sql);
    return DB_SUCCESS;
}

/**
 * @brief Deletes task from database. Note: id is
 *        looked up using author field value.
 * 
 * @param task task struct
 * @param db sqlite3 database handle
 * @param rows_affected affected row count
 * 
 * @return int DB_CONN_ERR if database handle is invalid
 * @return int DB__ERR if query execuion fails
 * @return int DB_SUCCESS if all is OK
 */
int delete_task(task_t *task, sqlite3 *db, int *rows_affected)
{
    if (db == NULL)
    {
        return DB_CONN_ERR;
    }
    int id = DB_NO_RESULT;
    if (get_user_id(task->author, db, &id) != DB_SUCCESS)
    {
        return DB_ERR;
    }
    char *db_err_msg;
    char *sql = sqlite3_mprintf(
        "DELETE FROM task WHERE id = '%d'", id
    );
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", db_err_msg);
        sqlite3_free(db_err_msg);
        sqlite3_free(sql);
        return DB_ERR;
    }
    if (*rows_affected != NULL)
    {
        *rows_affected = sqlite3_changes(db);
    }
    sqlite3_free(sql);
    return DB_SUCCESS;
}

/**
 * @brief Opens connection to sqlite3 database.
 * 
 * @param db_name database name
 * @param db database handle
 * 
 * @return int DB_CONN_ERR if fails
 * @return int DB_SUCCESS if success
 */
int open_db(char *db_name, sqlite3 **db)
{
    if (sqlite3_open(db_name, db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return DB_CONN_ERR;
    }
    return DB_SUCCESS;
}
