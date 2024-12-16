#include "screenshot.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static bool take_screenshot_wayland(const char* screenshot_file_path)
{
    pid_t grim = fork();
    if (grim == 0) {
        if (execlp("grim", "grim", "-t", "ppm", screenshot_file_path, NULL) == -1) {
            fprintf(stderr, "ERROR: could not execute `grim`: %s\n",
                    strerror(errno));
            exit(1);
        }
        exit(0);
    }

    int grim_status = 0;
    if (waitpid(grim, &grim_status, 0) == -1) {
        fprintf(stderr, "ERROR: could not `waitpid`: %s\n", strerror(errno));
        return false;
    }

    if (!WIFEXITED(grim_status)) {
        fprintf(stderr, "ERROR: `grim` subprocess exited abnormally\n");
        return false;
    }

    int grim_return_code = WEXITSTATUS(grim_status);
    if (grim_return_code != 0) {
        fprintf(stderr, "ERROR: `grim` subprocess exited with code `%d`\n",
                grim_return_code);
        return false;
    }

    return true;
}

static bool take_screenshot_x11(const char* screenshot_file_path)
{
    pid_t scrot = fork();
    if (scrot == 0) {
        if (execlp("scrot", "scrot", "-o", "-F", screenshot_file_path, NULL) == -1) {
            fprintf(stderr, "ERROR: could not execute `scrot`: %s\n",
                    strerror(errno));
            exit(1);
        }
        exit(0);
    }

    int scrot_status = 0;
    if (waitpid(scrot, &scrot_status, 0) == -1) {
        fprintf(stderr, "ERROR: could not `waitpid`: %s\n", strerror(errno));
        return false;
    }

    if (!WIFEXITED(scrot_status)) {
        fprintf(stderr, "ERROR: `scrot` subprocess exited abnormally\n");
        return false;
    }

    int scrot_return_code = WEXITSTATUS(scrot_status);
    if (scrot_return_code != 0) {
        fprintf(stderr, "ERROR: `scrot` subprocess exited with code `%d`\n",
                scrot_return_code);
        return false;
    }

    return true;
}

static bool take_screenshot_spectacle(const char* screenshot_file_path)
{
    pid_t spectacle = fork();
    if (spectacle == 0) {
        if (execlp("spectacle", "spectacle", "-b", "-n", "-o", screenshot_file_path, NULL) == -1) {
            fprintf(stderr, "ERROR: could not execute `spectacle`: %s\n",
                    strerror(errno));
            exit(1);
        }
        exit(0);
    }

    int spectacle_status = 0;
    if (waitpid(spectacle, &spectacle_status, 0) == -1) {
        fprintf(stderr, "ERROR: could not `waitpid`: %s\n", strerror(errno));
        return false;
    }

    if (!WIFEXITED(spectacle_status)) {
        fprintf(stderr, "ERROR: `spectacle` subprocess exited abnormally\n");
        return false;
    }

    int spectacle_return_code = WEXITSTATUS(spectacle_status);
    if (spectacle_return_code != 0) {
        fprintf(stderr, "ERROR: `spectacle` subprocess exited with code `%d`\n",
                spectacle_return_code);
        return false;
    }

    return true;
}

bool take_screenshot(const char* screenshot_file_path)
{
    if (getenv("WAYLAND_DISPLAY") == NULL)
        return take_screenshot_x11(screenshot_file_path);

    char const* xdg_current_desktop = getenv("XDG_CURRENT_DESKTOP");
    if (xdg_current_desktop == NULL)
        return take_screenshot_wayland(screenshot_file_path);

    if (strcmp(xdg_current_desktop, "KDE") == 0)
        return take_screenshot_spectacle(screenshot_file_path);

    return take_screenshot_wayland(screenshot_file_path);
}
