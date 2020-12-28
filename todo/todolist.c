#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "todolist.h"

/**
 * @brief Prints user related error to console. Exits program if
 * flag was set to FATAL.
 * 
 * @param source source function
 * @param detail description message
 * @param flags error severity flag
 */
void user_err(const char *source, const char *detail, int flags)
{
    fputs(source, stderr);
    fputs(": ", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
    if (flags == FATAL)
        exit(EXIT_FAILURE);
}

/**
 * @brief Prints syste related error to console. Exits the program
 * if flag was set to FATAL.
 *
 * @param source description message
 * @param flags error severity flag
 */
void sys_err(const char *source, int flags)
{
    perror(source);
    if (flags == FATAL)
        exit(EXIT_FAILURE);
}

uint8_t word_count(const char *buffer)
{
    uint8_t size_counter = 0;
    char quote = 0;
    for (size_t i = 0; i < COMMAND_SIZE; i++)
    {
        if (buffer[i] == '"')
        {
            quote ? 0 : 1;
        }
        if (buffer[i] == '\0')
        {
            size_counter++;
            break;
        }
        else if (buffer[i] == ' ' && quote)
        {
            size_counter++;
        }
    }
    size_counter++;
    return size_counter;
}

void clear()
{
#ifdef unix
    system("clear");
#elif _WIN32
    system("cls");
#endif
}

void print_peer(int sock)
{
    void *ip = NULL;
    char buffer[INET6_ADDRSTRLEN];
    struct sockaddr_in ipv4_addr;
    socklen_t size = sizeof(ipv4_addr);

    in_port_t port = ntohs(ipv4_addr.sin_port);
    getpeername(sock, (struct sockaddr *)(&ipv4_addr), &size);
    ip = &(ipv4_addr.sin_addr);
    inet_ntop(ipv4_addr.sin_family, ip, buffer, size);
    fprintf(stdout, "%s:%u\n", buffer, port);
}

void print_addr_info(struct sockaddr *addr, FILE *stream)
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
    case AF_INET6:
        numeric_addr = &((struct sockaddr_in6 *)addr)->sin6_addr;
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
        fputc('\n', stream);
    }
}

void create_addr_query(struct addrinfo *query, int family, int proto, int sockt, int flags)
{
    memset(query, 0, sizeof(query));
    query->ai_family = family;
    query->ai_protocol = proto;
    query->ai_socktype = sockt;
    query->ai_flags = flags;
}

int setup_server_socket(in_addr_t ip, in_port_t port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(ip);
    addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        sys_err("socket()", FATAL);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        sys_err("bind()", FATAL);
    if (listen(sock, MAX_PENDING_CONNECTIONS) < 0)
        sys_err("listen()", FATAL);

    // print_addr_info((struct sockaddr *)&addr, stdout);
    return sock;
}

/**
 * @brief Create and connect a TCP socket to specified
 * host. The socket an be IPv4 or IPv6.
 * 
 * @param host hostname
 * @param service service name or port
 * @return socket descriptor, -1 if fails.
 */
int setup_client_socket(const char *host, const char *service)
{
    struct addrinfo addr_query;
    memset(&addr_query, 0, sizeof(addr_query));
    addr_query.ai_family = AF_UNSPEC;
    addr_query.ai_socktype = SOCK_STREAM;
    addr_query.ai_protocol = IPPROTO_TCP;

    // Get address infos based on hostname and service.
    struct addrinfo *server_addr;
    int ret_val = getaddrinfo(host, service, &addr_query, &server_addr);
    if (ret_val != 0)
        user_err("getaddrinfo()", gai_strerror(ret_val), FATAL);

    // Search working address from linked address list.
    int sock = -1;
    for (struct addrinfo *addr = server_addr; addr != NULL; addr = addr->ai_next)
    {
        // Create a socket based on address info node and connect it.
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock >= 0 && connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
            break; // success

        // Socket creation failed, close and continue.
        close(sock);
        sock = -1;
    }
    freeaddrinfo(server_addr);
    return sock;
}