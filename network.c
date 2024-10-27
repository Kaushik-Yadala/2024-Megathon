#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>

static int server_socket;
static int client_sockets[MAX_CLIENTS] = {0};
static int sock = 0;
static int playerId = -1; // For client use
static pthread_t recv_thread;
static int running = 1;

// Server Function to Start and Manage Connections
int start_server()
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket, max_sd, sd, activity;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        return -1;
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            perror("select error");
        }

        if (FD_ISSET(server_socket, &readfds))
        {
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket < 0)
            {
                perror("accept");
                return -1;
            }

            printf("New connection, socket fd is %d\n", client_socket);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = client_socket;

                    // Send player ID (index) to client
                    char player_id_msg[BUFFER_SIZE];
                    snprintf(player_id_msg, sizeof(player_id_msg), "PLAYER_ID|%d", i);
                    write(client_socket, player_id_msg, strlen(player_id_msg));

                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds))
            {
                int bytes_read = read(sd, buffer, sizeof(buffer) - 1);
                if (bytes_read == 0)
                {
                    close(sd);
                    client_sockets[i] = 0;
                    printf("Client disconnected\n");
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    printf("Received message from player %d: %s\n", i, buffer);

                    // Broadcast to other clients
                    for (int j = 0; j < MAX_CLIENTS; j++)
                    {
                        if (client_sockets[j] > 0 && client_sockets[j] != sd)
                        {
                            write(client_sockets[j], buffer, bytes_read);
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// Stop the server and close all client connections
void stop_server()
{
    close(server_socket);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] > 0)
        {
            close(client_sockets[i]);
        }
    }
}

char message_buffer[BUFFER_SIZE]; // Buffer to store received message

char* receive_message()
{
    int bytes_read = read(sock, message_buffer, BUFFER_SIZE - 1);
    if (bytes_read > 0)
    {
        message_buffer[bytes_read] = '\0';
        printf("Received broadcast: %s\n", message_buffer);
    }
    else
    {
        printf("Error or no message received\n");
    }
    return message_buffer;
}

// Thread function to handle server messages for the client
void *handle_server_messages(void *arg)
{
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while (running && (bytes_read = read(sock, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';

        // Check for player ID message
        if (strncmp(buffer, "PLAYER_ID|", 10) == 0)
        {
            playerId = atoi(buffer + 10);
            printf("Assigned Player ID: %d\n", playerId);
        }
        else
        {
            printf("Message from server: %s\n", buffer);
        }
    }
    return NULL;
}

// Start the client and connect to the server
int start_client(const char *server_ip)
{
    struct sockaddr_in server_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        return -1;
    }

    running = 1;
    pthread_create(&recv_thread, NULL, handle_server_messages, NULL);
    return 0;
}

// Send a message from the client to the server
void send_message(const char *message)
{
    if (message && sock > 0)
    {
        send(sock, message, strlen(message), 0);
    }
}

// Stop the client and terminate the message handling thread
void stop_client()
{
    running = 0;
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
    close(sock);
}