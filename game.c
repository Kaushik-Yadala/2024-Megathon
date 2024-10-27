#include "raylib.h"
#include "network.h" // Include your network header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Removed <unistd.h> since it's not used here

#define SCREEN_WIDTH 1800
#define SCREEN_HEIGHT 1000
#define NUM_OPPONENTS 2
#define BUFFER_SIZE 256 // Define BUFFER_SIZE

typedef enum GameScreen
{
    INTRO,
    RULES,
    PLAYER_DETAILS,
    WAITING,
    TRICK_OR_TREAT,
    TREAT,
    TRICK,
    STANDINGS
} GameScreen;

void intToStr(int N, char *str)
{
    int i = 0;
    int sign = N;

    if (N < 0)
        N = -N;

    while (N > 0)
    {
        str[i++] = N % 10 + '0';
        N /= 10;
    }

    if (sign < 0)
        str[i++] = '-';

    str[i] = '\0';

    for (int j = 0, k = i - 1; j < k; j++, k--)
    {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

typedef struct
{
    int characterSelected;
    int points;
} Player;

// Global variable to hold the player
Player player;

// Networking Variables
char server_ip[16] = "127.0.0.1"; // Local server IP
int server_running = 0;

void connect_to_server()
{
    if (start_client(server_ip) == 0)
    {
        server_running = 1;
        printf("Connected to server.\n");
    }
    else
    {
        printf("Failed to connect to server.\n");
    }
}

void disconnect_from_server()
{
    stop_client();
    server_running = 0;
    printf("Disconnected from server.\n");
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trick or Treat");

    GameScreen currentScreen = INTRO;
    SetTargetFPS(60);

    // Player data initialization
    player.characterSelected = -1;
    player.points = 0;

    // Button definitions
    Rectangle nextButton = {SCREEN_WIDTH - 200, SCREEN_HEIGHT - 100, 180, 60};
    Rectangle backButton = {20, SCREEN_HEIGHT - 100, 180, 60};
    Rectangle zombieButton = {SCREEN_WIDTH / 2 - 350, 450, 150, 60};
    Rectangle vampireButton = {SCREEN_WIDTH / 2 - 150, 450, 150, 60};
    Rectangle witchButton = {SCREEN_WIDTH / 2 + 50, 450, 150, 60};
    Rectangle ghostButton = {SCREEN_WIDTH / 2 + 250, 450, 150, 60};

    // Connect to the server
    connect_to_server();

    // Main game loop
    while (!WindowShouldClose())
    {
        // Handle screen-specific input
        switch (currentScreen)
        {
        case INTRO:
            if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = RULES;
            }
            break;

        case RULES:
            if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = PLAYER_DETAILS;
            }
            break;

        case PLAYER_DETAILS:
            // Character selection buttons
            if (CheckCollisionPointRec(GetMousePosition(), zombieButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                player.characterSelected = 0;
            if (CheckCollisionPointRec(GetMousePosition(), vampireButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                player.characterSelected = 1;
            if (CheckCollisionPointRec(GetMousePosition(), witchButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                player.characterSelected = 2;
            if (CheckCollisionPointRec(GetMousePosition(), ghostButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                player.characterSelected = 3;

            if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && player.characterSelected != -1)
            {
                // Send character selection to server
                char message[BUFFER_SIZE];
                sprintf(message, "PLAYER_CHOICE:%d", player.characterSelected);
                send_message(message);

                currentScreen = WAITING;
            }
            break;

        case WAITING:
            // Wait for other players (placeholder)
            if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = TRICK_OR_TREAT;
            }
            break;

        case TRICK_OR_TREAT:
            ClearBackground(DARKBROWN);
            DrawText("Choose: Trick or Treat?", SCREEN_WIDTH / 2 - 200, 100, 50, RAYWHITE);

            Rectangle trickButton = {SCREEN_WIDTH / 2 - 250, 400, 200, 80};
            Rectangle treatButton = {SCREEN_WIDTH / 2 + 50, 400, 200, 80};

            DrawRectangleRec(trickButton, ORANGE);
            DrawRectangleRec(treatButton, DARKGREEN);
            DrawText("Trick", SCREEN_WIDTH / 2 - 190, 420, 40, BLACK);
            DrawText("Treat", SCREEN_WIDTH / 2 + 110, 420, 40, BLACK);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 mousePosition = GetMousePosition();
                if (CheckCollisionPointRec(mousePosition, trickButton))
                {
                    currentScreen = TRICK;
                    player.points -= 5;
                    send_message("TRICK");
                    printf("Selected Trick, points: %d\n", player.points);
                }
                else if (CheckCollisionPointRec(mousePosition, treatButton))
                {
                    currentScreen = TREAT;
                    player.points += 10;
                    send_message("TREAT");
                    printf("Selected Treat, points: %d\n", player.points);
                }
            }
            break;

        case TREAT:
            if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = STANDINGS;
            }
            break;

        case TRICK:
            if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = STANDINGS;
            }
            break;

        case STANDINGS:
            if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = INTRO;
                player.points = 0;
                player.characterSelected = -1;
                disconnect_from_server();
            }
            break;
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentScreen)
        {
        case INTRO:
            ClearBackground(DARKBLUE);
            DrawText("Welcome to Trick or Treat!", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 50, 40, RAYWHITE);
            break;

        case RULES:
            ClearBackground(DARKGREEN);
            DrawText("Game Rules:", SCREEN_WIDTH / 2 - 100, 100, 50, GOLD);
            DrawText("1. Choose between Trick or Treat.", 200, 200, 30, WHITE);
            DrawText("2. Gain points based on outcome.", 200, 250, 30, WHITE);
            DrawText("3. Highest points win!", 200, 300, 30, WHITE);
            break;

        case PLAYER_DETAILS:
            ClearBackground(DARKPURPLE);
            DrawText("Choose Your Character", SCREEN_WIDTH / 2 - 200, 100, 50, RAYWHITE);
            DrawRectangleRec(zombieButton, player.characterSelected == 0 ? GREEN : GRAY);
            DrawRectangleRec(vampireButton, player.characterSelected == 1 ? RED : GRAY);
            DrawRectangleRec(witchButton, player.characterSelected == 2 ? PURPLE : GRAY);
            DrawRectangleRec(ghostButton, player.characterSelected == 3 ? LIGHTGRAY : GRAY);
            DrawText("Zombie", zombieButton.x + 20, zombieButton.y + 15, 20, RAYWHITE);
            DrawText("Vampire", vampireButton.x + 20, vampireButton.y + 15, 20, RAYWHITE);
            DrawText("Witch", witchButton.x + 20, witchButton.y + 15, 20, RAYWHITE);
            DrawText("Ghost", ghostButton.x + 20, ghostButton.y + 15, 20, BLACK);
            break;

        case WAITING:
            ClearBackground(DARKGRAY);
            DrawText("Waiting for other players...", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2, 40, LIGHTGRAY);
            break;

        case TREAT:
            ClearBackground(GREEN);
            DrawText("You Chose Treat! Points awarded!", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 50, 40, YELLOW);
            break;

        case TRICK:
            ClearBackground(RED);
            DrawText("You Chose Trick! Points deducted!", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 50, 40, YELLOW);
            break;

        case STANDINGS:
            ClearBackground(DARKBLUE);
            DrawText("Game Standings", SCREEN_WIDTH / 2 - 150, 100, 50, GOLD);
            DrawText(TextFormat("Your Points: %d", player.points), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 40, RAYWHITE);
            DrawText(TextFormat("Opp Points: %s", receive_message()), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50, 40, RAYWHITE);
            DrawText("Press 'Back' to restart.", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 100, 30, LIGHTGRAY);
            break;
        }

        DrawRectangleRec(nextButton, GRAY);
        DrawRectangleRec(backButton, GRAY);
        DrawText("Next", nextButton.x + 50, nextButton.y + 20, 30, BLACK);
        DrawText("Back", backButton.x + 50, backButton.y + 20, 30, BLACK);

        EndDrawing();
    }

    // Cleanup and exit
    disconnect_from_server();
    CloseWindow();
    return 0;
}
