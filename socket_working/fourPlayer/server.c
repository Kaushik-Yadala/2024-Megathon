#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 4

int main()
{
    int server_socket, client_socket, max_sd, sd, activity;
    int client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 3) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        // Clear the socket set and add the server socket
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // Add child sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        // Wait for activity on any socket
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            perror("select error");
        }

        // If something happened on the server socket, it’s an incoming connection
        if (FD_ISSET(server_socket, &readfds))
        {
            if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("New connection, socket fd is %d\n", client_socket);

            // Add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }

        // Check each client socket for incoming messages
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds))
            {
                int bytes_read = read(sd, buffer, sizeof(buffer) - 1);
                if (bytes_read == 0)
                {
                    // Client disconnected
                    close(sd);
                    client_sockets[i] = 0;
                    printf("Client disconnected\n");
                }
                else
                {
                    int tempClientSock;
                    // Null-terminate the received message
                    buffer[bytes_read] = '\0';
                    printf("Received message: %s\n", buffer);

                    // Broadcast the message to all other clients
                    // for (int j = 0; j < MAX_CLIENTS; j++)
                    // {
                    //     if (client_sockets[j] > 0 && client_sockets[j] != sd)
                    //     {
                    //         write(client_sockets[j], buffer, bytes_read);
                    //     }
                    // }

                    // code to broadcast to individual client

                    for (int j = 0; j < MAX_CLIENTS; i++)
                    {
                        if (j + 1 == buffer[0])
                        {
                            write(client_sockets[j], buffer, bytes_read);
                        }
                    }

                    write(tempClientSock, buffer, bytes_read);
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
