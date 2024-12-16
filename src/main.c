#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "screenshot.h"

#ifdef __WIN32__
#define SCREENSHOT_FILE_PATH                                                       \
    ({                                                                             \
        const char* temp_folder = getenv("TEMP");                                  \
        const size_t temp_folder_len = strlen(temp_folder);                        \
                                                                                   \
        const char* slash_file_name = "\\zoomer_screenshot.bmp";                   \
        const size_t slash_file_name_len = strlen(slash_file_name);                \
                                                                                   \
        const size_t full_path_len = temp_folder_len + slash_file_name_len;        \
        char* full_path = alloca(full_path_len + 1);                               \
        memcpy(full_path, temp_folder, temp_folder_len);                           \
        memcpy(full_path + temp_folder_len, slash_file_name, slash_file_name_len); \
        full_path[full_path_len] = '\0';                                           \
                                                                                   \
        full_path;                                                                 \
    })
#else
#define SCREENSHOT_FILE_PATH "/tmp/zoomer_screenshot.png"
#endif

#define LERP_AMOUNT 5.f

static void draw_circle_lines(Vector2 center, float radius, Color color, int thickness)
{
    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < 360; i += 10) {
        rlVertex2f(center.x + cosf(DEG2RAD * i) * (radius + thickness),
                   center.y + sinf(DEG2RAD * i) * (radius + thickness));
        rlVertex2f(center.x + cosf(DEG2RAD * i) * radius,
                   center.y + sinf(DEG2RAD * i) * radius);
        rlVertex2f(center.x + cosf(DEG2RAD * (i + 10)) * radius,
                   center.y + sinf(DEG2RAD * (i + 10)) * radius);
        rlVertex2f(center.x + cosf(DEG2RAD * (i + 10)) * (radius + thickness),
                   center.y + sinf(DEG2RAD * (i + 10)) * (radius + thickness));
    }

    rlEnd();
}

int main(void)
{
    if (!take_screenshot(SCREENSHOT_FILE_PATH)) {
        fprintf(stderr, "ERROR: failed to take screenshot\n");
        return 1;
    }

    Image image = LoadImage(SCREENSHOT_FILE_PATH);

    InitWindow(image.width, image.height, "Zoomer");
    ToggleFullscreen();

    Texture texture = LoadTextureFromImage(image);

    Camera2D camera = {
        .zoom = 1.f,
    };
    float camera_target_zoom = camera.zoom;

    float flashlight_radius = 200;
    float flashlight_radius_target = flashlight_radius;
    float flashlight_opacity = 0.f;
    float flashlight_opacity_target = flashlight_opacity;

    SetExitKey(KEY_Q);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));

        float dt = GetFrameTime();
        Vector2 mouse_pos_world = GetScreenToWorld2D(GetMousePosition(), camera);

        if (IsKeyPressed(KEY_F))
            flashlight_opacity_target = flashlight_opacity_target > 0 ? 0.f : 1.f;

        if (IsKeyPressed(KEY_ZERO)) {
            camera = (Camera2D) {
                .zoom = 1.f,
            };
            camera_target_zoom = 1.f;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_delta = GetMouseDelta();
            camera.target.x += -mouse_delta.x / camera.zoom;
            camera.target.y += -mouse_delta.y / camera.zoom;
        }

        float wheel_move = GetMouseWheelMove();
        bool is_modifier_pressed = IsKeyDown(KEY_LEFT_SHIFT)
            || IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_CONTROL)
            || IsKeyDown(KEY_RIGHT_CONTROL);

        if (is_modifier_pressed) {
            if (flashlight_opacity_target != 0) {
                if (wheel_move > 0)
                    flashlight_radius_target *= 1.2f;
                else if (wheel_move < 0)
                    flashlight_radius_target *= 0.8f;
            }
        } else {
            if (wheel_move > 0)
                camera_target_zoom *= 1.1;
            else if (wheel_move < 0)
                camera_target_zoom *= 0.9;
        }

        // Update camera zoom
        {
            Vector2 mouse_world_before_zoom = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.zoom = Lerp(camera.zoom, camera_target_zoom, LERP_AMOUNT * dt);

            Vector2 mouse_world_after_zoom = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.target = Vector2Add(camera.target,
                                       Vector2Subtract(mouse_world_before_zoom,
                                                       mouse_world_after_zoom));
        }

        // Update flashlight radius
        {
            flashlight_radius = Lerp(flashlight_radius, flashlight_radius_target, LERP_AMOUNT * dt);
        }

        // Update flashlight opacity
        {
            flashlight_opacity = Lerp(flashlight_opacity, flashlight_opacity_target, LERP_AMOUNT * dt);
        }

        BeginMode2D(camera);

        DrawTexture(texture, 0, 0, WHITE);

        draw_circle_lines(mouse_pos_world, flashlight_radius,
                          Fade(BLACK, 0.7 * flashlight_opacity), 8000);

        EndMode2D();

        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    if (unlink(SCREENSHOT_FILE_PATH) != 0) {
        fprintf(stderr, "WARNING: could not remove screenshot in `%s`: %s\n",
                SCREENSHOT_FILE_PATH, strerror(errno));
    }

    return 0;
}
