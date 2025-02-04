#include "tests/mocks.h"

#ifdef _WIN32
    int mock_recv(SOCKET sockfd, char *buf, int len, int flags) {
#else
    ssize_t mock_recv(int sockfd, void *buf, size_t len, int flags) {
#endif
    void *recv_data = mock_type(void *);
    ssize_t n = mock_type(ssize_t);
    if (n > 0) {
        memcpy(buf, recv_data, n);
    }
    return n;
}

#ifdef _WIN32
    int mock_send(SOCKET sockfd, const char *buf, int len, int flags) {
#else
    ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags) {
#endif
    ssize_t n = mock_type(ssize_t);
    return n;
}

#ifdef _WIN32
    int mock_connect(SOCKET sockfd, const struct sockaddr *addr, int addrlen) {
#else
    int mock_connect(
        int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
#endif
    return 0;
}
