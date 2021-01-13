#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include <stdbool.h>
#include <time.h>

static int create_task(void* list, int count, char **values, char **col_names);
static int deserialize_user(void *id, int count, char** values, char **col_name);
static int deserialize_task(task_t *task, int count, char **values, char **col_name);
static void get_row_count(sqlite3 *db, int *affected_rows);
static void db_cleanup(char *sql, char *err_msg);
static void db_err(const char *template, const char *db_err_msg,
    const char *sql, FILE *stream);

/***************** TASK LINKED LIST *******************/

/**
 * @brief Check if the first item of the
 *        linked list is used.
 * 
 * @param first linked list head.
 * @return true if head is used.
 * @return false if head is not used.
 */
bool ll_first_empty(node_t *first)
{
    return first->value == NULL;
}

/**
 * @brief Prints all the tasks in provided
 *        linked list.
 * 
 * @param head list head
 * @param stream output stream
 */
// void ll_print(node_t *head, FILE *stream)
// {
//     char date_buf[80];
//     struct tm ts;
//     while (head != NULL)
//     {
//         ts = *localtime(&(head->add_time));
//         strftime(date_buf, sizeof(date_buf), "%a %Y-%m-%d %H:%M:%S", &ts);
//         fprintf(stream, "Date: %s\n", date_buf);
//         fprintf(stream, "%s\n", head->author);
//         fprintf(stream, "%s\n\n", head->content);
//         head = head->next;
//     }
// }

/**
 * @brief Get the last node of the linked list.
 * 
 * @param head linked list head
 * @return task_t* pointer to last node
 */
node_t *ll_last(node_t *head)
{
    while(head->next != NULL)
    {
        head = head->next;
    }
    return head;
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
void ll_add_node(node_t **head, node_t *node)
{
    if ((*head)->value == NULL)
    {
        *head = node;
    }
    else
    {
        ll_last(*head)->next = node;
    }
}

/**
 * @brief Execute provided operation for each item
 *        in the list.
 * 
 * @param head head of the linked list
 * @param operation operation that gets executed for each iteration
 */
void ll_foreach(node_t *head, void (*operation)(void *node))
{
    node_t *next;
    for (node_t *node = head; node != NULL; node = next)
    {
        next = node->next;
        operation(node);
    }
}



/******************************************************/


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
int db_select_user(user_t *user, sqlite3 *db, user_t *result)
{
    char *db_err_msg = NULL;
    char *sql;
    if (strlen(user->name) > 0)
    {   // Use username in query
        sql = sqlite3_mprintf("SELECT * FROM user WHERE name = '%q'", user->name);
    }
    else if (user->id > 0)
    {   // Use user id in query
        sql = sqlite3_mprintf("SELECT * FROM user WHERE id='%d'", user->id);
    }
    else
    {
        return DB_INVALID_PARAM;
    }
    int rc = sqlite3_exec(db, sql, deserialize_user, result, &db_err_msg);
    if (rc != SQLITE_OK)
    {
        db_err("An error occured while selecting user id: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }
    else if (result == NULL)
    {
        db_cleanup(sql, db_err_msg);
        return DB_NO_RESULT;
    }
    db_cleanup(sql, db_err_msg);
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
int db_select_tasks(user_t *user, sqlite3 *db, node_t *results)
{
    char *db_err_msg = NULL;
    int rc = SQLITE_OK;
    char *sql = sqlite3_mprintf(
        "SELECT content, timestamp FROM task WHERE author = '%d'",
        user->id);
    rc = sqlite3_exec(db, sql, create_task, results, &db_err_msg);
    if (rc != SQLITE_OK)
    {
        db_err("An error occured while selecting tasks: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }

    for (node_t *item = results; item != NULL; item = item->next)
    {
        strcpy(((task_t *)item->value)->author, user->name);
    }

    db_cleanup(sql, db_err_msg);
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
int db_insert(char *sql, sqlite3 *db, int *affected_rows)
{
    char *db_err_msg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK) 
    {
        db_err("An error occured while inserting to database: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(NULL, db_err_msg);
        return DB_ERR;
    }
    get_row_count(db, affected_rows);
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
int db_update_task(task_t *task, sqlite3 *db, int *rows_affected)
{
    int id = DB_NO_RESULT;
    if (db_select_user(task->author, db, &id) != DB_SUCCESS)
    {
        return DB_ERR;
    }
    char *db_err_msg;
    char *sql = sqlite3_mprintf(
        "UPDATE task\
         SET author='%q', content='%q', timestamp='%d'\
         WHERE id='%q'",
            task->author, task->content,
            task->timestamp, id
    );
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("An error occured while updating a task: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }
    get_row_count(db, rows_affected);
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
int db_insert_task(task_t *task, sqlite3 *db, int *rows_affected)
{
    char *db_err_msg;
    char *sql = sqlite3_mprintf(
        "INSERT INTO task (content, author, timestamp) \
            VALUES ('%q', '%q', '%d')",
            task->content, task->author, task->timestamp
    );
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("An error occured while inserting task: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }
    get_row_count(db, rows_affected);
    sqlite3_free(sql);
    return DB_SUCCESS;
}

/**
 * @brief Updates user row on database. Note: user's
 *        id needs to be set in user struct.
 * 
 * @param user user struct
 * @param db sqlite3 database handle
 * @param rows_affected affected row count
 * 
 * @return int DB_ERR if sql execution failed
 * @return int SUCCESS if all OK
 */
int db_update_user(user_t *user, sqlite3 *db, int *rows_affected)
{
    char *db_err_msg = NULL;
    char *sql = sqlite3_mprintf(
        "UPDATE task\
            SET author='%q', content='%q', timestamp='%d'\
            WHERE id='%q'",
            user->name, user->timestamp,
            user->id
    );

    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("An error occured while updating user: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }
    get_row_count(db, rows_affected);
    db_cleanup(sql, db_err_msg);
    return DB_SUCCESS;
}

/**
 * @brief Inserts user to database
 * 
 * @param user user struct
 * @param db sqlite3 database handle
 * @param rows_affected count of affected rows
 * 
 * @return int DB_ERR if sql execution failed
 * @return int SUCCESS if all OK
 */
int db_insert_user(user_t *user, sqlite3 *db, int *rows_affected)
{
    char *db_err_msg = NULL;
    char *sql = sqlite3_mprintf("INSERT INTO user (name, timestamp) \
            VALUES ('%q', '%d')",
            user->name, user->timestamp);
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("An error occured while inserting user: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }
    get_row_count(db, rows_affected);
    db_cleanup(sql, db_err_msg);
    return DB_SUCCESS;
}

/**
 * @brief Deletes user from database. Note: user id
 *        needs to be set in user struct.
 * 
 * @param user user struct
 * @param db sqlite3 database handle
 * @param rows_affected count of rows affected
 * 
 * @return int DB__ERR if query execuion fails
 * @return int DB_SUCCESS if all is OK
 */
int db_delete_user(user_t *user, sqlite3 *db, int *rows_affected)
{
    char *db_err_msg = NULL;
    char *sql = sqlite3_mprintf("DELETE FROM user WHERE id='%d'",
        user->id);
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("An error occured while deleting user: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }
    get_row_count(db, rows_affected);
    db_cleanup(sql, db_err_msg);
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
int db_delete_task(task_t *task, sqlite3 *db, int *rows_affected)
{
    int id = DB_NO_RESULT;
    if (db_select_user(task->author, db, &id) != DB_SUCCESS)
    {
        return DB_ERR;
    }
    char *db_err_msg;
    char *sql = sqlite3_mprintf("DELETE FROM task WHERE id = '%d'", id);
    if (sqlite3_exec(db, sql, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("An error occured while deleting task: %s\n",
            db_err_msg, sql, stderr);
        db_cleanup(sql, db_err_msg);
        return DB_ERR;
    }
    get_row_count(db, rows_affected);
    db_cleanup(sql, db_err_msg);
    return DB_SUCCESS;
}

/**
 * @brief Drops all tables and creates new ones.
 * 
 * @param db sqlite3 database handle
 * @return int DB_SUCCESS if everything is OK
 */
int db_init(sqlite3 *db)
{
    char *db_err_msg = NULL;
    char *sql_pragma = "PRAGMA foreign_keys = ON";
    char *sql_drop_tasks = "DROP TABLE IF EXISTS task";
    char *sql_drop_users = "DROP TABLE IF EXISTS user";
    char *sql_usertable =
        "CREATE TABLE IF NOT EXISTS user( \
            id INTEGER PRIMARY KEY AUTOINCREMENT, \
            timestamp INTEGER NOT NULL, \
            name TEXT NOT NULL UNIQUE \
        );";
    char *sql_tasktable =
        "CREATE TABLE IF NOT EXISTS task( \
            id INTEGER PRIMARY KEY AUTOINCREMENT, \
            content TEXT NOT NULL, \
            author INTEGER NOT NULL, \
            timestamp INTEGER NOT NULL, \
            FOREIGN KEY(author) REFERENCES user(id) \
        );";
    if (sqlite3_exec(db, sql_pragma, NULL, NULL, &db_err_msg))
    {
        db_err("Error occured while dropping task table: %s\n",
            db_err_msg, sql_pragma, stderr);
        db_cleanup(NULL, db_err_msg);
        return DB_ERR;
    }
    if (sqlite3_exec(db, sql_drop_tasks, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("Error occured while dropping task table: %s\n",
            db_err_msg, sql_drop_tasks, stderr);
        db_cleanup(NULL, db_err_msg);
        return DB_ERR;
    }
    if (sqlite3_exec(db, sql_drop_users, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("Error occured while dropping user table: %s\n",
            db_err_msg, sql_drop_users, stderr);
        db_cleanup(NULL, db_err_msg);
        return DB_ERR;
    }
    if (sqlite3_exec(db, sql_usertable, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("Error occured while creating user table: %s\n",
            db_err_msg, sql_usertable, stderr);
        db_cleanup(NULL, db_err_msg);
        return DB_ERR;
    }
    if (sqlite3_exec(db, sql_tasktable, NULL, NULL, &db_err_msg) != SQLITE_OK)
    {
        db_err("Error occured while creating task table: %s\n",
            db_err_msg, sql_tasktable, stderr);
        db_cleanup(NULL, db_err_msg);
        return DB_ERR;
    }
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
int db_open(char *db_name, sqlite3 **db)
{
    if (sqlite3_open(db_name, db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return DB_CONN_ERR;
    }
    return DB_SUCCESS;
}




/*************************************************************/
/***************** Static helper functions *******************/
/*************************************************************/

/**
 * @brief Get the affected row count for most
 *        recent database commit.
 * 
 * @param db sqlite3 database handle
 * @param affected_rows affected row count
 */
static void get_row_count(sqlite3 *db, int *affected_rows)
{
    if (affected_rows != NULL)
    {
        *affected_rows = sqlite3_changes(db);
    }
}

/**
 * @brief Prints database related error to provided
 *        stream.
 * 
 * @param template template string
 * @param db_err_msg error message string
 * @param sql sql string
 * @param stream output stream
 */
static void db_err(const char *template, const char *db_err_msg,
    const char *sql, FILE *stream)
{
    fprintf(stream, template, db_err_msg);
#ifdef DEBUG
    fprintf(stream, "DEBUG defined -> Executed SQL statement: \n\t%s\n", sql);
#endif
}

/**
 * @brief Cleans up possible memory allocations
 *        made by sqlite3 library.
 * 
 * @param sql dynamically allocated sql string
 * @param err_msg dynamically allocated err string
 */
static void db_cleanup(char *sql, char *err_msg)
{
    if (sql != NULL)
    {
        sqlite3_free(sql);
    }
    if (err_msg != NULL)
    {
        sqlite3_free(err_msg);
    }
}

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
static int deserialize_user(void *result, int count, char** values, char **col_name)
{
    user_t *user_res = (user_t *)result;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(col_name[i], "id") == 0)
        {
            user_res->id = atoi(values[i]);
        }
        else if (strcmp(col_name[i], "name") == 0)
        {
            strcpy(user_res->name, values[i]);
        }
        else if (strcmp(col_name[i], "timestamp") == 0)
        {
            user_res->timestamp = atoi(values[i]);
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
static int deserialize_task(task_t *task, int count, char **values, char **col_name)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(col_name[i], "content") == 0)
        {
            strcpy(task->content, values[i]);
        }
        else if (strcmp(col_name[i], "timestamp") == 0)
        {
            task->timestamp = atoi(values[i]);
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
static int create_task(void* list, int count, char **values, char **col_names)
{
    node_t *last_node = ll_last((node_t *)list);
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    new_task->id = -1;
    last_node->value = new_task;
    return deserialize_task(new_task, count, values, col_names);
}
