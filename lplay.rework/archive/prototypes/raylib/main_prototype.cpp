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
            BeginMode3D(camera);

            DrawGrid(10, 1.0f);

            if (cmd == "cube")
            {
                DrawCube(
                    { 0.0f, 0.5f, 0.0f },
                    1.0f, 1.0f, 1.0f,
                    RED
                );
            }
            else if (cmd == "pyramid")
            {
                DrawCylinder(
                    { 0.0f, 0.0f, 0.0f },
                    0.0f, 1.0f, 2.0f, 4,
                    ORANGE
                );
            }

            EndMode3D();

            DrawText("Move mouse left/right to look around",
                     20, 20, 20, DARKGRAY);
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
