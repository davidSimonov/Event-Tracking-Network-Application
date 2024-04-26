/*
    David Simonov
    das892
    11301273
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utils.h>
#include <sys/select.h>

#define BUFFER_SIZE 128 /* Buffer size for reading from sockets */
#define MAX_CLIENTS 10  /* Maximum number of clients */
#define PORT 36000      /* Server port */

int main() {
    int server_sock, port_number = PORT;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    fd_set read_fds, ready_fds;
    int i, max_sd, new_sd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    /* Create a TCP socket */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    /* Initialize server address structure 
    Bind the server socket to the local address and port */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; /* Internet address family */
    server_addr.sin_addr.s_addr = INADDR_ANY; /* Any incoming interface */
    server_addr.sin_port = htons(port_number); /* Local port */

    if (bind(server_sock, (struct sockaddr*)&server_addr, 
        sizeof(server_addr)) < 0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    /* Listen for incoming connection requests on the server socket */
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    /* Initialize the set of active sockets */
    FD_ZERO(&read_fds); /* Clear the socket set */
    FD_SET(server_sock, &read_fds); /* Add the server socket to the set */
    max_sd = server_sock; /* Keep track of the highest file descriptor number */

    /* Main loop: wait for new connections or data from existing connections */
    while (1) {
        ready_fds = read_fds; /* Copy the set of read file descriptors */
        if (select(max_sd + 1, &ready_fds, NULL, NULL, NULL) < 0) {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        /* Iterate through file descriptors to check which ones are ready */
        for (i = 0; i <= max_sd; i++) {
            /* Check if the file descriptor is part of the set */
            if (FD_ISSET(i, &ready_fds)) {
                if (i == server_sock) {
                    /* A new connection is incoming */
                    client_addr_len = sizeof(client_addr);
                    new_sd = accept(server_sock, (struct sockaddr*)&client_addr,
                         &client_addr_len);

                    if (new_sd < 0) {
                        perror("accept failed");
                        continue;
                    }

                    /* Add the new socket to the set */
                    FD_SET(new_sd, &read_fds);
                    /* Update the maximum file descriptor number if necessary */
                    if (new_sd > max_sd) {
                        max_sd = new_sd;
                    }

                    printf("New connection from %s on socket %d\n", 
                        inet_ntoa(client_addr.sin_addr), new_sd);
                } else {
                    /* Data is ready to be read from a client socket */
                    memset(buffer, 0, BUFFER_SIZE);
                    bytes_read = recv(i, buffer, BUFFER_SIZE, 0);
                    if (bytes_read <= 0) {
                        /* Connection closed by client or error */
                        close(i); /* Close the socket */
                        FD_CLR(i, &read_fds); /* Remove from read set */
                        printf("Client on socket %d disconnected\n", i);
                    } else {
                        /* Process the client's request */
                        char* response = processRequest(buffer);
                        printf("Server select: processing\n"); /* TEST */
                        /* Send response back to client */
                        if (response) {
                            send(i, response, strlen(response), 0);
                            free(response);
                        } else {
                            printf("Calendar not initialized\n");
                        }
                    }
                }
            }
        }
    }

    /* Clean up */
    close(server_sock);
    return 0;
}