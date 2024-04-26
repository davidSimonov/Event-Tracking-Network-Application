/*
    David Simonov
    das892
    11301273
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utils.h>

#define BUFFER_SIZE 128 /* Buffer size for reading from sockets */
#define MAX_CLIENTS 10  /* Maximum number of clients */
#define PORT 36000      /* Server port */

/* Thread to process connection */
void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    char* response;

    /* Bild the socket */
    bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        perror("Recv failed");
        close(client_sock);
        pthread_exit(NULL);
    }

    /* If GETALL dont break connection */
    if (isGetall(buffer)) {
        int i, numEvents;
        User* user;
        printf("Server thread: processing GETALL request\n"); /* TEST */
        user = lockUser(buffer);

        response = processRequest(buffer);
        send(client_sock, response, strlen(response), 0);

        if (!user) goto close;

        sscanf(response, "COUNT:%d", &numEvents);
        free(response);

        for (i = 0; i < numEvents; i++) {
            memset(buffer, 0, BUFFER_SIZE);
            bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_read <= 0) {
                perror("Recv failed on GETEVENT");
                break;
            }

            printf("Server thread: processing GETEVENT request %d\n", i + 1);
            response = processRequest(buffer);
            send(client_sock, response, strlen(response), 0);
            free(response);
        }
        unlockUser(user);
    } else {
        /* Process the request and send a response */
        printf("Server thread: thread processing request\n"); /* TEST */
        response = processRequest(buffer);
        send(client_sock, response, strlen(response), 0);
        free(response);
    }

    close:
    close(client_sock);
    pthread_exit(NULL);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;

    /* Create server socket */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Failed to create socket");
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
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    /* Listen for connections */
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    while (1) {
        /* Accept connection */
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, 
            &client_addr_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connection accepted from %s\n",inet_ntoa(client_addr.sin_addr));

        /* Create thread to process the connection */
        printf("Server thread: creating thread to process request\n");/* TEST */
        if (pthread_create(&thread_id, NULL, handle_client, 
            (void*)&client_sock) != 0) 
        {
            perror("Could not create thread");
            close(client_sock);
        }
    }
    
    /* Clean-up */
    close(server_sock);
    return 0;
}