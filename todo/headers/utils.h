#if !defined(TODO_LIST_H)
#define TODO_LIST_H

#define CMD_SIZE 3
#define CMD_ADD "ADD"
#define CMD_RMV "RMV"
#define NETWORKBUF_OFFSET 4
#define FATAL 1
#define NON_FATAL 2

enum CMD {
    INV,
    ADD,
    RMV
};

void sys_err(const char *source);
void user_err(const char *source, const char *detail, int flag);
void get_input(const char* output, char *buffer, size_t buff_len, size_t *out_len);
void str_ins(char *dest, char *src, int offset, size_t len);
void clear();

#endif // TODO_LIST_H
