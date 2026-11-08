#include "raylib.h"

#include <cctype>
#include <iostream>
#include <string>

int main()
{
    std::cout << "Enter mode (2D or 3D): ";

    std::string dmode;
    std::cin >> dmode;

    // Make mode input case-insensitive.
    for (char& c : dmode)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    const bool is3d = (dmode == "3d" || dmode == "3");
    const bool is2d = (dmode == "2d" || dmode == "2");

    std::string cmd;

    if (is3d)
    {
        std::cout << "Enter shape (cube or pyramid): ";
        std::cin >> cmd;

        for (char& c : cmd)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    InitWindow(800, 450, "RLB-BE");
    SetTargetFPS(60);

    Camera3D camera{};
    camera.position = { 0.0f, 1.5f, 6.0f };
    camera.target = { 0.0f, 1.5f, 5.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float yaw = 0.0f;
    const float mouseSensitivity = 0.003f;
    bool firstMouseSample = true;

    if (is3d)
        DisableCursor();

    while (!WindowShouldClose())
    {
        if (is3d)
        {
            Vector2 mouseDelta = GetMouseDelta();

            // Skip the first mouse reading to avoid a sudden initial turn.
            if (firstMouseSample)
            {
                firstMouseSample = false;
            }
            else
            {
                yaw += mouseDelta.x * mouseSensitivity;
            }

            camera.target = {
                camera.position.x + sinf(yaw),
                camera.position.y,
                camera.position.z - cosf(yaw)
            };
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

if (is3d)
{
    Vector2 mouseDelta = GetMouseDelta();

    if (firstMouseSample)
    {
        firstMouseSample = false;
    }
    else
    {
        yaw += mouseDelta.x * mouseSensitivity;
    }

    // Forward and right directions on the XZ plane.
    const float forwardX = sinf(yaw);
    const float forwardZ = -cosf(yaw);
    const float rightX = cosf(yaw);
    const float rightZ = sinf(yaw);

    float moveX = 0.0f;
    float moveZ = 0.0f;

    if (IsKeyDown(KEY_W)) { moveX += forwardX; moveZ += forwardZ; }
    if (IsKeyDown(KEY_S)) { moveX -= forwardX; moveZ -= forwardZ; }
    if (IsKeyDown(KEY_D)) { moveX += rightX;   moveZ += rightZ; }
    if (IsKeyDown(KEY_A)) { moveX -= rightX;   moveZ -= rightZ; }

    // Normalize diagonal movement so it isn't faster than straight movement.
    const float length = sqrtf(moveX * moveX + moveZ * moveZ);
    if (length > 0.0f)
    {
        moveX /= length;
        moveZ /= length;

        const float speed = 4.0f; // World units per second
        const float distance = speed * GetFrameTime();

        camera.position.x += moveX * distance;
        camera.position.z += moveZ * distance;
    }

    camera.target = {
        camera.position.x + forwardX,
        camera.position.y,
        camera.position.z + forwardZ
    };
}

        else if (is2d)
        {
            DrawText("Hello!", 20, 20, 24, DARKGRAY);
            DrawText("2D mode", 20, 50, 20, BLUE);
        }
        else
        {
            DrawText("Unknown mode: enter 2D or 3D",
                     20, 20, 20, RED);
        }

        EndDrawing();
    }

    EnableCursor();
    CloseWindow();

    std::cout << "\nRLB-BE closed.\n";
    return 0;
}
