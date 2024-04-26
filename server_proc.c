/*
    David Simonov
    das892
    11301273
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <utils.h>

#define BUFFER_SIZE 128 /* Buffer size for reading from sockets */
#define MAX_CLIENTS 10  /* Maximum number of clients */
#define PORT 36000      /* Server port */

/* Signal handler to clean up zombie processes */
void sigchld_handler(int s) {
    (void)s; 
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    struct sigaction sa;

    /* Create server socket */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    /* Bind server socket */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, 
        sizeof(server_addr)) < 0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    /* Listening for connections */
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    /* Zombie process reaping */
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    while(1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr,
            &client_addr_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        /* Handling client requests */
        bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read < 0) {
            close(client_sock);
            continue;
        }

        /* Deciding whether to handle the request in a child process */
        if (isRead(buffer)) {
            printf("Server proc: request is read\n"); /* TEST */
            if (fork() == 0) { /* Child process */
                char* response = processRequest(buffer);
                
                printf("Server proc: child processing request\n"); /* TEST */
                /* If GETALL dont break connection */
                if (isGetall(buffer)) {
                    User* user;
                    int i, numEvents;
                    printf("Server proc: processing GETALL request\n"); /*TEST*/
                    user = lockUser(buffer);
                    
                    response = processRequest(buffer);
                    send(client_sock, response, strlen(response), 0);

                    sscanf(response, "COUNT:%d", &numEvents);
                    free(response);
                    
                    if (!user) goto close;

                    for (i = 0; i < numEvents; i++) {
                        memset(buffer, 0, BUFFER_SIZE);
                        bytes_read = recv(client_sock, buffer, 
                            BUFFER_SIZE - 1, 0);
                        if (bytes_read <= 0) {
                            perror("Recv failed on GETEVENT");
                            break;
                        }

                        printf("Server proc: processing GETEVENT request %d\n",
                             i + 1);
                        response = processRequest(buffer);
                        send(client_sock, response, strlen(response), 0);
                        free(response);
                    }
                    unlockUser(user);
                } else {
                    /* Process the request and send a response */
                    printf("Server proc: child processing request\n"); /*TEST */
                    response = processRequest(buffer);
                    send(client_sock, response, strlen(response), 0);
                    free(response);
                }
                close:
                exit(0);
            }
            close(client_sock); /* Parent process closes the socket */
        } else { /* Parent process handles the request directly */
            char* response = processRequest(buffer);
            printf("Server proc: parent processes write request\n"); /* TEST */
            send(client_sock, response, strlen(response), 0);
            free(response);
            close(client_sock);
        }
    }

    return 0;
}