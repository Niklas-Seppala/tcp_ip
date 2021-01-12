#include "todolist.h"
#include <sqlite3.h>
#include <stdbool.h>

#if !defined(DATA_H)
#define DATA_H

#define DB_CONN_ERR (-1)
#define DB_ERR (-1)
#define DB_SUCCESS 1
#define DB_NAME "tasks.db"
#define DB_NO_RESULT (-10)
typedef int (*select_cb)(void *, int, char **, char **);

typedef struct todo_task
{
    char author[SBUFFER_SIZE];
    char content[BUFFER_SIZE];
    time_t add_time;
    struct todo_task *next;
} task_t;

// DATABSE

/**
 * @brief Opens connection to sqlite3 database.
 * 
 * @param db_name database name
 * @param db database handle
 * 
 * @return int DB_CONN_ERR if fails
 * @return int DB_SUCCESS if success
 */
int open_db(char *db_name, sqlite3 **db);

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
int get_user_id(char *username, sqlite3 *db, int* result);

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
int get_tasks_by_username(char *username, sqlite3 *db, task_t* results);

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
int insert_task(task_t *task, sqlite3 *db, int *rows_affected);

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
int insert(char *sql, sqlite3 *db, int *affected_rows);

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
int update_task(task_t *task, sqlite3 *db, int *rows_affected);

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
int delete_task(task_t *task, sqlite3 *db, int *rows_affected);

// DATA STRUCTURES

/**
 * @brief Check if the first item of the
 *        linked list is used.
 * 
 * @param first linked list head.
 * @return true if head is used.
 * @return false if head is not used.
 */
bool ll_first_empty(task_t *first);

/**
 * @brief Prints all the tasks in provided
 *        linked list.
 * 
 * @param head list head
 * @param stream output stream
 */
void ll_print(task_t *head, FILE *stream);

/**
 * @brief Links the provided node at the end of the
 *        list. If list head is NULL, it will then
 *        point to provided node.
 * 
 * @param head First item in the list
 * @param node Node to be added
 * @return int 1 if success
 */
int ll_add_node(task_t **head, task_t *node);

/**
 * @brief Get the last node of the linked list.
 * 
 * @param head linked list head
 * @return task_t* pointer to last node
 */
task_t *ll_last(task_t *head);

/**
 * @brief Execute provided operation for each item
 *        in the list.
 * 
 * @param head head of the linked list
 * @param operation operation that gets executed for each iteration
 */
void ll_foreach(task_t *head, void (*operation)(void *node));

#endif // DATA_H
