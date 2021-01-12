#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "todolist.h"

void get_input(const char* output, char *buffer, size_t buff_len, size_t *out_len)
{
    if (output != NULL) {
        fputs(output, stdout);
    }
    fgets(buffer, buff_len, stdin);
    size_t len = strlen(buffer);
    if (buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        if (out_len != NULL) {
            *out_len = len-1;
        }
    } else if (out_len != NULL) {
        *out_len = len;
    }
}

/**
 * @brief Prints user related error to console. Exits program
 * with EXIT_FAILURE.
 * 
 * @param source source function
 * @param detail description message
 */
void user_err(const char *source, const char *detail)
{
    fputs(source, stderr);
    fputs(": ", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints syste related error to console. Exits the program
 * with EXIT_FAILURE.
 *
 * @param source description message
 */
void sys_err(const char *source)
{
    perror(source);
    exit(EXIT_FAILURE);
}

void clear()
{
#ifdef unix
    system("clear");
#elif _WIN32
    system("cls");
#endif
}

void print_port(int sock, const char* template_str, FILE *stream) {
    
    struct sockaddr_in addr;
    socklen_t s_size = sizeof(addr);
    getsockname(sock, (struct sockaddr*)&addr, &s_size);
    in_port_t port = ntohs(addr.sin_port);

    const char * temp = template_str == NULL ? "port: %u\n" : template_str;
    fprintf(stream, temp, port);
}

void print_sock(int sock, FILE *stream)
{
    void *ip = NULL;
    char buffer[INET_ADDRSTRLEN];

    struct sockaddr_in ipv4_addr;
    socklen_t size = sizeof(ipv4_addr);
    getsockname(sock, (struct sockaddr*)&ipv4_addr, &size);
    ip = &(ipv4_addr.sin_addr);
    in_port_t port = ntohs(ipv4_addr.sin_port);
    inet_ntop(AF_INET, ip, buffer, size);
    fprintf(stream, "%s:%u\n", buffer, port);
}

void print_peer(int sock)
{
    void *ip = NULL;
    char buffer[INET6_ADDRSTRLEN];
    struct sockaddr_in ipv4_addr;
    socklen_t size = sizeof(ipv4_addr);

    getpeername(sock, (struct sockaddr *)&ipv4_addr, &size);
    in_port_t port = ntohs(ipv4_addr.sin_port);
    ip = &(ipv4_addr.sin_addr);
    inet_ntop(ipv4_addr.sin_family, ip, buffer, size);
    fprintf(stdout, "%s:%u\n", buffer, port);
}

void str_ins(char *dest, char *src, int offset, size_t len) {
    dest += offset;
    for (size_t i = 0; i < len; i++)
        *dest++ = *src++;
}

void print_addr_info(struct sockaddr *addr, FILE *stream)
{
    if (addr == NULL || stream == NULL) {
        return;
    }

    void *numeric_addr;
    char buffer[INET6_ADDRSTRLEN];
    in_port_t port;

    switch (addr->sa_family) {
    case AF_INET:
        numeric_addr = &((struct sockaddr_in *)addr)->sin_addr;
        port = ntohs(((struct sockaddr_in *)addr)->sin_port);
        break;
    case AF_INET6:
        numeric_addr = &((struct sockaddr_in6 *)addr)->sin6_addr;
        port = ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
        break;
    default:
        fputs("unknown address family\n", stream);
        return;
    }
    if (inet_ntop(addr->sa_family, numeric_addr, buffer, sizeof(buffer)) == NULL) {
        fputs("Invalid address\n", stream);
    }
    else {
        fprintf(stream, "%s", buffer);
        if (port != 0) {
            fprintf(stream, ":%u", port);
        }
        fputc('\n', stream);
    }
}

int setup_server_socket(in_addr_t ip, in_port_t port, int queue_size)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(ip);
    addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        sys_err("socket()");
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        sys_err("bind()");
    if (listen(sock, queue_size) < 0)
        sys_err("listen()");
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
        user_err("getaddrinfo()", gai_strerror(ret_val));

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