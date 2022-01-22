#include<glad/glad.h>
#include <GLFW/glfw3.h>
#include<memory>
#include<SoftRenderer.h>
int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    unsigned char* bg = new unsigned char[640 * 480 * 4];
    for (int i = 0; i < 480; i++)
    {
        for (int j = 0; j < 640*4; j+=4)
        {
            bg[640 * 4 * i + j+0] = 0x00;
            bg[640 * 4 * i + j+1] = 0x00;
            bg[640 * 4 * i + j+2] = 0xff;
            bg[640 * 4 * i + j+3] = 0xff;
        }
    }
    Renderer renderer(640, 480);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        renderer.Clear(COLOR_BUFFER_BIT);
        glDrawPixels(640,480, GL_RGBA, GL_UNSIGNED_BYTE, renderer.get_canvas());
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}