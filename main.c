#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include "glfw.h"
#include "X.h"

#define SCREENSHOT_FILE_PATH "/tmp/zoomer_screenshot.ppm"
#define LERP_AMOUNT 0.03

void ppm_skip_comments(size_t* current_offset, char* ppm)
{
    size_t ppm_size = strlen(ppm);

    if (ppm[*current_offset] == '#') {
        while (ppm[*current_offset] != '\n') {
            if (*current_offset >= ppm_size) {
                fprintf(stderr, "ERROR: reached end of file while parsing comment in image\n");
                exit(1);
            }

            *current_offset += 1;
        }
        *current_offset += 1;

        if (*current_offset >= ppm_size) {
            fprintf(stderr, "ERROR: reached end of file while parsing comment in image\n");
            exit(1);
        }
    }
}

Image image_from_ppm(char* ppm)
{
    if (strncmp(ppm, "P6", 2) != 0) {
        fprintf(stderr, "ERROR: unsupported image format: `%.*s`\n", 2, ppm);
        exit(1);
    }

    char width[256] = { 0 };
    char height[256] = { 0 };
    char maximum_color_value[256] = { 0 };

    size_t current_offset = 3;

    ppm_skip_comments(&current_offset, ppm);
    for (int i = 0; current_offset < strlen(ppm); ++current_offset, ++i) {
        if (isspace(ppm[current_offset]))
            break;
        width[i] = ppm[current_offset];
    }
    current_offset++;

    ppm_skip_comments(&current_offset, ppm);
    for (int i = 0; current_offset < strlen(ppm); ++current_offset, ++i) {
        if (isspace(ppm[current_offset]))
            break;
        height[i] = ppm[current_offset];
    }
    current_offset++;

    ppm_skip_comments(&current_offset, ppm);
    for (int i = 0; current_offset < strlen(ppm); ++current_offset, ++i) {
        if (isspace(ppm[current_offset]))
            break;
        maximum_color_value[i] = ppm[current_offset];
    }
    current_offset++;

    int i_width = atoi(width);
    int i_height = atoi(height);
    int i_maximum_color_value = atoi(maximum_color_value);

    if (i_width == 0 || i_height == 0 || i_maximum_color_value == 0) {
        fprintf(stderr,
                "ERROR: could not parse image: invalid width, height, or "
                "maximum color value\n");
        exit(1);
    }

    printf("[INFO] Loaded image\n");
    printf("  -> Dimensions: %d x %d\n", i_width, i_height);
    printf("  -> Maximum color value: %d\n", i_maximum_color_value);

    return (Image) {
        .data = &ppm[current_offset],
        .width = i_width,
        .height = i_height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8,
    };
}

size_t get_file_size(FILE* file)
{
    size_t x = ftell(file);

    fseek(file, 0L, SEEK_END);
    size_t sz = ftell(file);

    fseek(file, x, SEEK_SET);

    return sz;
}

FILE* take_screenshot_wayland(void)
{
    pid_t grim = fork();
    if (grim == 0) {
        if (execlp("grim", "grim", "-t", "ppm", SCREENSHOT_FILE_PATH, NULL) == -1) {
            fprintf(stderr, "ERROR: could not execute `grim`: %s\n",
                    strerror(errno));
            exit(1);
        }
        exit(0);
    }

    int grim_status = 0;
    if (waitpid(grim, &grim_status, 0) == -1) {
        fprintf(stderr, "ERROR: could not `waitpid`: %s\n", strerror(errno));
        return NULL;
    }

    if (!WIFEXITED(grim_status)) {
        fprintf(stderr, "ERROR: `grim` subprocess exited abnormally\n");
        return NULL;
    }

    int grim_return_code = WEXITSTATUS(grim_status);
    if (grim_return_code != 0) {
        fprintf(stderr, "ERROR: `grim` subprocess exited with code `%d`\n",
                grim_return_code);
        return NULL;
    }

    FILE* file = fopen(SCREENSHOT_FILE_PATH, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n",
                SCREENSHOT_FILE_PATH, strerror(errno));
        return NULL;
    }

    return file;
}

FILE* take_screenshot_x11(void)
{
    pid_t scrot = fork();
    if (scrot == 0) {
        if (execlp("scrot", "scrot", "-o", "-F", SCREENSHOT_FILE_PATH, NULL) == -1) {
            fprintf(stderr, "ERROR: could not execute `scrot`: %s\n",
                    strerror(errno));
            exit(1);
        }
        exit(0);
    }

    int scrot_status = 0;
    if (waitpid(scrot, &scrot_status, 0) == -1) {
        fprintf(stderr, "ERROR: could not `waitpid`: %s\n", strerror(errno));
        return NULL;
    }

    if (!WIFEXITED(scrot_status)) {
        fprintf(stderr, "ERROR: `scrot` subprocess exited abnormally\n");
        return NULL;
    }

    int scrot_return_code = WEXITSTATUS(scrot_status);
    if (scrot_return_code != 0) {
        fprintf(stderr, "ERROR: `scrot` subprocess exited with code `%d`\n",
                scrot_return_code);
        return NULL;
    }

    FILE* file = fopen(SCREENSHOT_FILE_PATH, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n",
                SCREENSHOT_FILE_PATH, strerror(errno));
        return NULL;
    }

    return file;
}

void draw_circle_lines(Vector2 center, float radius, Color color, int thickness)
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

FILE* take_screenshot(void)
{
    return (getenv("WAYLAND_DISPLAY") == NULL)
        ? take_screenshot_x11()
        : take_screenshot_wayland();
}

void set_loading_cursor(void)
{
    if (getenv("WAYLAND_DISPLAY")) return;

    unsigned long window = glfwGetX11Window(GetWindowHandle());
    if (window == 0) return;

    void* display = glfwGetX11Display();
    if (display == NULL) return;

    unsigned long cursor = XcursorLibraryLoadCursor(display, "watch");
    XDefineCursor(display, window, cursor);
}

int main(void)
{
    FILE* file = take_screenshot();
    if (file == NULL)
        return 1;

    size_t file_size = get_file_size(file);
    char* ppm = malloc(file_size + 1);
    ppm[file_size] = 0;
    fread(ppm, file_size, 1, file);
    fclose(file);

    Image image = image_from_ppm(ppm);

    InitWindow(image.width, image.height, "Zoomer");
    ToggleFullscreen();
    set_loading_cursor();

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

        Vector2 mouse_pos_world = GetScreenToWorld2D(GetMousePosition(), camera);

        if (IsKeyPressed(KEY_F))
            flashlight_opacity_target = flashlight_opacity_target > 0 ? 0.f : 1.f;

        if (IsKeyPressed(KEY_ZERO)) {
            camera = (Camera2D) {
                .zoom = 1.f,
            };
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
            if (wheel_move > 0)
                flashlight_radius_target *= 1.2f;
            else if (wheel_move < 0)
                flashlight_radius_target *= 0.8f;
        } else {
            if (wheel_move > 0)
                camera_target_zoom *= 1.1;
            else if (wheel_move < 0)
                camera_target_zoom *= 0.9;
        }

        // Update camera zoom
        {
            Vector2 mouse_world_before_zoom = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.zoom = Lerp(camera.zoom, camera_target_zoom, LERP_AMOUNT);

            Vector2 mouse_world_after_zoom = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.target = Vector2Add(camera.target,
                                       Vector2Subtract(mouse_world_before_zoom,
                                                       mouse_world_after_zoom));
        }

        // Update flashlight radius
        {
            flashlight_radius = Lerp(flashlight_radius, flashlight_radius_target, LERP_AMOUNT);
        }

        // Update flashlight opacity
        {
            flashlight_opacity = Lerp(flashlight_opacity, flashlight_opacity_target, LERP_AMOUNT);
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

    free(ppm);

    if (unlink(SCREENSHOT_FILE_PATH) != 0) {
        fprintf(stderr, "WARNING: could not remove screenshot in `%s`: %s\n",
                SCREENSHOT_FILE_PATH, strerror(errno));
    }

    return 0;
}
