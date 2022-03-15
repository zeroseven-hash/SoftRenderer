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



    struct V
    {
        TinyMath::Vec4f pos_;
        TinyMath::Vec4f color_;
    };

    std::vector<V> vertices = {
       {{-0.5f,0.0f,0.0f,1.0f},{1.0f,0.0f,0.0f,1.0f}},
       {{0.5f,0.0f,0.0f,1.0f},{0.0f,1.0f,0.0f,1.0f}},
       {{0.0f,0.5f,0.0f,1.0f},{0.0f,0.0f,1.0f,1.0f}},
    };
    std::vector<uint32_t> indices = { 0,1,2};
    VertexArrayBuffer<V,uint32_t> vao(std::move(vertices), std::move(indices));
    const int VARYING_COLOR = 0;
    Shader<V> shader;

    shader.vertex_shader_ = [&](const VertexArrayBuffer<V>& vao,int index, ShaderContext& output)->TinyMath::Vec4f {
        auto& v = vao.get_vertex(index);
        output.varying_vec4f_[VARYING_COLOR] = v.color_;
        return  v.pos_;
    };
    shader.fragment_shader_ = [&](ShaderContext& input)->TinyMath::Vec4f {
        return input.varying_vec4f_[VARYING_COLOR];
    };
    Renderer renderer(640, 480);

    renderer.set_render_state(true, false);



    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        renderer.Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
        renderer.DrawArray(vao, shader);
        glDrawPixels(640, 480, GL_RGBA, GL_UNSIGNED_BYTE, renderer.get_canvas());
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}