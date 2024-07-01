#ifndef GLFW_H_
#define GLFW_H_

// Raylib bundles glfw and I don't wanna depend
// on glfw just for the headers.

unsigned long glfwGetX11Window(void* window);
void* glfwGetX11Display(void);

#endif // GLFW_H_
