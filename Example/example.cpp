#include<glad/glad.h>
#include <GLFW/glfw3.h>
#include<memory>
#include<cstring>
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

    struct { TinyMath::Vec4f pos; TinyMath::Vec4f color; } vs_input[3] = {
		{ {  0.0,  0.7, 0.90, 1}, {1, 0, 0, 1} },
		{ { -0.6, -0.2, 0.01, 1}, {0, 1, 0, 1} },
		{ { +0.6, -0.2, 0.01, 1}, {0, 0, 1, 1} },
    };
    const int VARYING_COLOR = 0;
    Shader shader;
    
    shader.vertex_shader_ = [&](int index, ShaderContext& output)->TinyMath::Vec4f {
        output.varying_vec4f_[VARYING_COLOR] = vs_input[index].color;
        return vs_input[index].pos;
    };
    shader.fragment_shader_ = [&](ShaderContext& input)->TinyMath::Vec4f {
        return input.varying_vec4f_[VARYING_COLOR];
    };
    Renderer renderer(640, 480);


   
   

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        renderer.Clear(COLOR_BUFFER_BIT|DEPTH_BUFFER_BIT);
        renderer.DrawTriangle(shader);
        glDrawPixels(640, 480, GL_RGBA, GL_UNSIGNED_BYTE, renderer.get_canvas());
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}