#ifndef X_H_
#define X_H_

// I cannot include real X.h because raylib has
// types that conflict with it.

unsigned long XcursorLibraryLoadCursor(void *dpy, const char *file);
int XDefineCursor(void* display, unsigned long window, unsigned long cursor);

#endif // X_H_
