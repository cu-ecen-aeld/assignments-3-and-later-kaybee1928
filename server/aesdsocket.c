//
// Created by kashribha on 2/11/26.
//

#include "aesdsocket.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <pthread.h>

static volatile sig_atomic_t caught_signal = 0;

void signal_handler(int signum) {
    caught_signal = 1;
    syslog(LOG_INFO, "Caught signal %d\n", signum);
}

char* append_char(char *buffer, size_t *current_size, size_t *used_size, char c) {
    if (*current_size == 0 || *used_size >= *current_size - 1) {

        size_t new_size = (*current_size == 0) ? 16 : (*current_size * 2);


        char *new_buffer = realloc(buffer, new_size);
        if (new_buffer == NULL) {

            syslog(LOG_ERR, "Failed to allocate memory for buffer: %s\n", strerror(errno));
            return buffer;
        }

        buffer = new_buffer;
        *current_size = new_size;
    }

    buffer[*used_size] = c;
    (*used_size)++;
    buffer[*used_size] = '\0';

    return buffer;
}

void server() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    int confd;
    struct addrinfo *serv_addr;
    struct sockaddr cli_addr;
    struct addrinfo hint;
    memset(&cli_addr, 0, sizeof(cli_addr));
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    socklen_t addrlen = sizeof(struct sockaddr);
    // int optval = 1;
    // setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    int status;
    if ((status = getaddrinfo(NULL, PORT, &hint, &serv_addr)) != 0) {
        syslog(LOG_ERR, "getaddrinfo error: %s\n", gai_strerror(status));
        closelog();
        exit(1);
    }

    if ((status = bind(sfd, serv_addr->ai_addr, serv_addr->ai_addrlen)) != 0) {
        syslog(LOG_ERR, "bind error: %s\n", strerror(errno));
        freeaddrinfo(serv_addr);
        closelog();
        exit(1);
    }

    listen(sfd, BACKLOG);

    if ((confd = accept(sfd, &cli_addr, &addrlen)) == -1) {
        if (!caught_signal) syslog(LOG_ERR, "accept error: %s\n", strerror(errno));
        else remove("/var/tmp/aesdsocketdata");
        freeaddrinfo(serv_addr);
        closelog();
        exit(1);
    }
    syslog(LOG_INFO, "Accepted connection from %s\n", inet_ntoa(((struct sockaddr_in *)&cli_addr)->sin_addr));

    int fd = open("/var/tmp/aesdsocketdata", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        syslog(LOG_ERR, "Failed to open file for writing: %s\n", strerror(errno));
        close(sfd);
        close(confd);
        freeaddrinfo(serv_addr);
        closelog();
        exit(1);
    }

    char *buffer_char = malloc(sizeof(char));
    char *buffer = NULL;
    size_t buffer_size = 0;
    size_t buffer_used_size = 0;
    ssize_t nread;
    ssize_t nwritten;
    while ((nread = recv(confd, buffer_char, 1, 0)) > 0 && !caught_signal) {
        buffer = append_char(buffer, &buffer_size, &buffer_used_size, buffer_char[0]);
        if (buffer_char[0] == '\n') {
            if (buffer != NULL) {
                nwritten = write(fd, buffer, buffer_used_size);
                if (nwritten != buffer_used_size) {
                    syslog(LOG_ERR, "Failed to write to file: %s\n", strerror(errno));
                }
                free(buffer);
                buffer = NULL;
                buffer_size = 0;
                buffer_used_size = 0;
            }
            int rfd = open("/var/tmp/aesdsocketdata", O_RDONLY);
            if (rfd != -1) {
                char send_buf[1024];
                ssize_t bytes_read;
                while ((bytes_read = read(rfd, send_buf, sizeof(send_buf))) > 0) {
                    send(confd, send_buf, bytes_read, 0);
                }
                close(rfd);
            }
        }
    }
    if (buffer != NULL) {
        nwritten = write(fd, buffer, buffer_used_size);
        if (nwritten != buffer_used_size) {
            syslog(LOG_ERR, "Failed to write to file: %s\n", strerror(errno));
        }
        free(buffer);
    }
    syslog(LOG_INFO, "Connection closed from %s\n", inet_ntoa(((struct sockaddr_in *)&cli_addr)->sin_addr));
    free(buffer_char);
    close(fd);
    close(confd);
    close(sfd);
    freeaddrinfo(serv_addr);
    closelog();
    if (caught_signal) {
        remove("/var/tmp/aesdsocketdata");
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    openlog("aesdsocket", LOG_CONS | LOG_PID, LOG_USER);
    if (argc > 2) {
        syslog(LOG_ERR, "Invalid number of arguments\n");
        exit(1);
    }
    if (argc == 2 && strcmp(argv[1], "-d") != 0) {
        syslog(LOG_ERR, "Invalid argument: %s\n", argv[1]);
        exit(1);
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        syslog(LOG_ERR, "Failed to set signal handler: %s\n", strerror(errno));
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        syslog(LOG_ERR, "Failed to set signal handler: %s\n", strerror(errno));
        exit(1);
    }

    if (argc == 2) {
        // Daemonize using fork
        pid_t pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "Failed to fork: %s\n", strerror(errno));
            exit(1);
        }
        if (pid > 0) {
            // Parent exits, child continues as daemon
            exit(0);
        }
        // Child process continues here
        setsid();              // Create new session, detach from terminal
        chdir("/");            // Change working directory to root
        close(STDIN_FILENO);   // Close standard file descriptors
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    while (!caught_signal) server();
    if (caught_signal) remove("/var/tmp/aesdsocketdata");
    closelog();
    return 0;
}
