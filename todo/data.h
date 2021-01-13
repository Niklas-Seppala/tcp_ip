#include "todolist.h"
#include <sqlite3.h>
#include <stdbool.h>

#if !defined(DATA_H)
#define DATA_H

#define DB_CONN_ERR (-1)
#define DB_ERR (-1)
#define DB_SUCCESS 1
#define DB_INVALID_PARAM (-5)
#define DB_NO_RESULT (-10)

typedef int (*select_cb)(void *, int, char **, char **);

typedef struct node {
    void *value;
    struct node *next;
} node_t;

typedef struct user {
    int id;
    char name[SBUFFER_SIZE];
    time_t timestamp;
} user_t;

typedef struct task
{
    int id;
    char author[SBUFFER_SIZE];
    char content[BUFFER_SIZE];
    time_t timestamp;
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
int db_open(char *db_name, sqlite3 **db);

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
int db_select_user(user_t *user, sqlite3 *db);

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
int db_select_tasks(user_t *user, sqlite3 *db, node_t* results);

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
int db_update_user(user_t *user, sqlite3 *db, int *rows_affected);

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
int db_delete_user(user_t *user, sqlite3 *db, int *rows_affected);

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
int db_insert_user(user_t *user, sqlite3 *db, int *rows_affected);

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
int db_insert_task(task_t *task, sqlite3 *db, int *rows_affected);

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
int db_insert(char *sql, sqlite3 *db, int *affected_rows);

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
int db_update_task(task_t *task, sqlite3 *db, int *rows_affected);

/**
 * @brief Drops all tables and creates new ones.
 * 
 * @param db sqlite3 database handle
 * @return int DB_SUCCESS if everything is OK
 */
int db_init(sqlite3 *db);

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
int db_delete_task(task_t *task, sqlite3 *db, int *rows_affected);


// DATA STRUCTURES
/**
 * @brief Check if the first item of the
 *        linked list is used.
 * 
 * @param first linked list head.
 * @return true if head is used.
 * @return false if head is not used.
 */
bool ll_first_empty(node_t *first);

/**
 * @brief Prints all the tasks in provided
 *        linked list.
 * 
 * @param head list head
 * @param stream output stream
 */
// void ll_print(node_t *head, FILE *stream);

/**
 * @brief Links the provided node at the end of the
 *        list. If list head is NULL, it will then
 *        point to provided node.
 * 
 * @param head First item in the list
 * @param node Node to be added
 * @return int 1 if success
 */
void ll_add_node(node_t **head, node_t *node);

/**
 * @brief Get the last node of the linked list.
 * 
 * @param head linked list head
 * @return task_t* pointer to last node
 */
node_t *ll_last(node_t *head);

/**
 * @brief Execute provided operation for each item
 *        in the list.
 * 
 * @param head head of the linked list
 * @param operation operation that gets executed for each iteration
 */
void ll_foreach(node_t *head, void (*operation)(void *node));

#endif // DATA_H
