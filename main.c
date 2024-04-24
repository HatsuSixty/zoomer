#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <raylib.h>

#define GRIM_SCREENSHOT_FILE_PATH "/tmp/zoomer_grim_screen.ppm"

Image image_from_ppm(char* ppm)
{
    int i;

    if (strncmp(ppm, "P6", 2) != 0) {
        fprintf(stderr, "ERROR: unsupported image format: `%.*s`\n", 2, ppm);
        exit(1);
    }

    char width[256] = { 0 };
    char height[256] = { 0 };
    char maximum_color_value[256] = { 0 };

    size_t current_offset = 3;
    for (i = 0; current_offset < strlen(ppm); ++current_offset, ++i) {
        if (isspace(ppm[current_offset]))
            break;
        width[i] = ppm[current_offset];
    }
    current_offset++;

    for (i = 0; current_offset < strlen(ppm); ++current_offset, ++i) {
        if (isspace(ppm[current_offset]))
            break;
        height[i] = ppm[current_offset];
    }
    current_offset++;

    for (i = 0; current_offset < strlen(ppm); ++current_offset, ++i) {
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

FILE* take_screenshot(void)
{
    pid_t grim = fork();
    if (grim == 0) {
        if (execlp("grim", "grim", "-t", "ppm", GRIM_SCREENSHOT_FILE_PATH, NULL)
            == -1) {
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

    FILE* file = fopen(GRIM_SCREENSHOT_FILE_PATH, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: could not open file `%s`: %s\n",
                GRIM_SCREENSHOT_FILE_PATH, strerror(errno));
        return NULL;
    }

    return file;
}

int main(void)
{
    FILE* file = take_screenshot();
    if (file == NULL) return 1;

    size_t file_size = get_file_size(file);
    char* ppm = malloc(file_size + 1);
    ppm[file_size] = 0;
    fread(ppm, file_size, 1, file);
    fclose(file);

    Image image = image_from_ppm(ppm);

    InitWindow(image.width, image.height, "Zoomer");
    ToggleFullscreen();

    Texture texture = LoadTextureFromImage(image);
    Rectangle texture_dst_rect = {
        .x = 0,
        .y = 0,
        .width = image.width,
        .height = image.height,
    };

    SetExitKey(KEY_Q);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (IsKeyPressed(KEY_ZERO)) {
            texture_dst_rect = (Rectangle) {
                .x = 0,
                .y = 0,
                .width = image.width,
                .height = image.height,
            };
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_delta = GetMouseDelta();
            texture_dst_rect.x += mouse_delta.x;
            texture_dst_rect.y += mouse_delta.y;
        }

        float scale_factor = 0.0f;

        float wheel_move = GetMouseWheelMove();
        if (wheel_move == 1)
            scale_factor = 1.2f;
        else if (wheel_move == -1)
            scale_factor = 0.8f;

        if (scale_factor != 0) {
            Vector2 mouse_position = GetMousePosition();
            float offset_x = (mouse_position.x - texture_dst_rect.x)
                / texture_dst_rect.width;
            float offset_y = (mouse_position.y - texture_dst_rect.y)
                / texture_dst_rect.height;

            texture_dst_rect.width *= scale_factor;
            texture_dst_rect.height *= scale_factor;

            texture_dst_rect.x
                = mouse_position.x - offset_x * texture_dst_rect.width;
            texture_dst_rect.y
                = mouse_position.y - offset_y * texture_dst_rect.height;
        }

        DrawTexturePro(texture, (Rectangle) { 0, 0, image.width, image.height },
                       texture_dst_rect, (Vector2) { 0, 0 }, 0.0f, WHITE);

        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    free(ppm);

    return 0;
}
