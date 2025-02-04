/**
 * @brief Contains mock functions for unit tests.
 */

#ifndef TESTS_MOCKS_H_
#define TESTS_MOCKS_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "include/networking.h"

#define MOCK_SOCKET 99

#ifdef _WIN32
    int mock_recv(SOCKET sockfd, char *buf, int len, int flags);
#else
    ssize_t mock_recv(int sockfd, void *buf, size_t len, int flags);
#endif

#ifdef _WIN32
    int mock_send(SOCKET sockfd, const char *buf, int len, int flags);
#else
    ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags);
#endif

#ifdef _WIN32
    int mock_connect(SOCKET sockfd, const struct sockaddr *addr, int addrlen);
#else
    int mock_connect(
        int sockfd, const struct sockaddr *addr, socklen_t addrlen);
#endif

#endif  // TEST_MOCKS_H_
