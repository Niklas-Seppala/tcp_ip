#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

void print_info(struct sockaddr *addr, FILE *stream)
{
    if (addr == NULL || stream == NULL)
        return;

    void *numeric_addr;
    char buffer[INET6_ADDRSTRLEN];
    in_port_t port;
    switch (addr->sa_family)
    {
    case AF_INET:
        numeric_addr = &((struct sockaddr_in *)addr)->sin_addr;
        port = ntohs(((struct sockaddr_in *)addr)->sin_port);
        break;
        numeric_addr = &((struct sockaddr_in6 *)addr)->sin6_addr;
    case AF_INET6:
        port = ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
        break;
    default:
        fputs("unknown address family", stream);
        return;
    }
    if (inet_ntop(addr->sa_family, numeric_addr, buffer, sizeof(buffer)) == NULL)
        fputs("Invalid address", stream);
    else
    {
        fprintf(stream, "%s", buffer);
        if (port != 0)
            fprintf(stream, ":%u", port);
        fputc('\0', stream);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
        return 1;

    char *addr_str = argv[1];
    char *port_str = argv[2];

    struct addrinfo query;
    memset(&query, 0, sizeof(query));
    query.ai_family = AF_UNSPEC;
    query.ai_protocol = IPPROTO_TCP;
    query.ai_socktype = SOCK_STREAM;
    query.ai_flags = AI_CANONNAME;

    struct addrinfo *addr_list;
    int ret_val = getaddrinfo(addr_str, port_str, &query, &addr_list);
    if (ret_val != 0)
        printf("%s: %s", "getaddrinfo()", gai_strerror(ret_val));

    for (struct addrinfo *addr = addr_list; addr != NULL; addr = addr->ai_next)
        print_info(addr->ai_addr, stdout);

    freeaddrinfo(addr_list);
    return 0;
}
