/*
 * =====================================================================================
 *
 *       Filename:  myecho.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/19/20 22:53:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Niklas Seppala, 
 *   Organization:  Metropolia AMK
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "myecho.h"

/**
 * Prints user related error to console. Exits program if
 * flag was set to FATAL.
 *
 * @param source source function
 * @param message description message
 * @param flag error severity flag
 */
void user_err(const char* source, const char* details, int flag)
{
    fputs(source, stderr);
    fputs(": ", stderr);
    fputs(details, stderr);
    fputc('\n', stderr);
    if (flag == E_FATAL)
        exit(1);
}

/**
 * Prints syste related error to console. Exits the program
 * if flag was set to FATAL.
 *
 * @param message description message
 * @param flag error severity flag
 */
void sys_err(const char* message, int flag)
{
    perror(message);
    if (flag == E_FATAL)
        exit(1);
}

/**
 * Creates a TCP socket. Uses IPv4.
 *
 * @param sock pointer to socket handle
 */
void create_socket(int* sock)
{
    *sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ((*sock) < 0)
        sys_err("socket()", E_FATAL);
}

