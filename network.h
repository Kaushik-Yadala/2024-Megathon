#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 4

// Server Functions
int start_server();
void stop_server();

// Client Functions
int start_client(const char *server_ip);
void send_message(const char *message);
void stop_client();
char* receive_message(); // New function for receiving messages

extern char message_buffer[BUFFER_SIZE]; // Expose buffer for display

#endif // NETWORK_H
