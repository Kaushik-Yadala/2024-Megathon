// main.c
#include "network.h"
#include <stdio.h>

int main()
{
    // Uncomment to start the server
    // start_server();

    // Uncomment to start the client, connect to server, and send a message
    // start_client("127.0.0.1");
    // send_message("Hello from client!");
    // stop_client();

    int choice;

    printf("server enter: 0, client enter: 1 ");
    scanf("%d", &choice);

    if (choice == 0)
    {
        start_server();
    }
    else if (choice == 1)
    {
        start_client("127.0.0.1");
        while (1)
        {
            char message[100];
            scanf("%s", message);
            send_message(message);
        }
    }

    return 0;
}
