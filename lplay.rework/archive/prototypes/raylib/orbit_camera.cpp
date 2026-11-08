#include "raylib.h"
#include <cmath>

int main()
{
    InitWindow(800, 450, "Mouse-controlled 3D surface");
    SetTargetFPS(60);

    Vector3 target{ 0.0f, 0.0f, 0.0f };
    float yaw = 45.0f * DEG2RAD;
    float pitch = 30.0f * DEG2RAD;
    float distance = 12.0f;

    while (!WindowShouldClose())
    {
        // Drag with left mouse to orbit; scroll to zoom.
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 delta = GetMouseDelta();
            yaw -= delta.x * 0.01f;
            pitch -= delta.y * 0.01f;
        }

        if (pitch > 1.4f) pitch = 1.4f;
        if (pitch < -1.4f) pitch = -1.4f;

        distance -= GetMouseWheelMove();
        if (distance < 3.0f) distance = 3.0f;
        if (distance > 40.0f) distance = 40.0f;

        Camera3D camera{};
        camera.target = target;
        camera.position = {
            target.x + distance * cosf(pitch) * sinf(yaw),
            target.y + distance * sinf(pitch),
            target.z + distance * cosf(pitch) * cosf(yaw)
        };
        camera.up = { 0.0f, 1.0f, 0.0f };
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawPlane({ 0.0f, -0.05f, 0.0f }, { 20.0f, 20.0f }, LIGHTGRAY);
        DrawGrid(20, 1.0f);
        DrawCube({ 0.0f, 0.5f, 0.0f }, 1.0f, 1.0f, 1.0f, RED);
        EndMode3D();

        DrawText("Hold left mouse and drag to orbit; scroll to zoom",
                 10, 10, 18, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// Build with a local Raylib installation and the link flags for your platform.
