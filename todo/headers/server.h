#if !defined SERVER_H
#define SERVER_H

#include "network.h"

#define CONF_RESET_DB 0x2

struct server_config {
    int flags;
    int max_pending;
    char db_name[S_NETWORKBUF_SIZE];
    in_port_t port;
};

#endif