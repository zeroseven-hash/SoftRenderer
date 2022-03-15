#include<glad/glad.h>
#include <GLFW/glfw3.h>
#include<memory>
#include<cstring>
#include<SoftRenderer.h>
#include"Model.h"
int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1024, 720, "Hello World", NULL, NULL);
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
        TinyMath::Vec4f color_ = TinyMath::Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
        TinyMath::Vec2f tex_coords_;
    };

    
    std::vector<uint32_t> indices = { 0,1,2 };
    Model<V> model("L:/dev/SoftRenderer/Example/assets/nanosuit/nanosuit.obj");
    
    
    const int VARY_TEXCOORDS = 0;
    Shader<V> shader;

    TinyMath::Mat4f model_matrix = TinyMath::Tranlate(TinyMath::Mat4f::GetIdentity(), TinyMath::Vec3f(0.0f, -0.65f, -3.0f));
    model_matrix = TinyMath::Scale(model_matrix, TinyMath::Vec3f(2 / 15.0f, 2 / 15.0f, 2 / 15.0f));
    TinyMath::Mat4f perpective = TinyMath::Perspective(60.0f, 1024 / 720.0f, 1.0f, 500.0f);
    shader.vertex_shader_ = [&](const VertexArrayBuffer<V, uint32_t>& vao, int index, ShaderContext& output)->TinyMath::Vec4f {
        auto& v = vao.get_vertex(index);
        output.varying_vec2f_[VARY_TEXCOORDS] = v.tex_coords_;

        
        return   perpective*model_matrix * v.pos_;
    };
    shader.fragment_shader_ = [&](ShaderContext& input)->TinyMath::Vec4f {
        return shader.textures_[0]->Sample2D(input.varying_vec2f_[VARY_TEXCOORDS]).TransformToVec();
    };
    Renderer renderer(1024, 720);

    renderer.set_render_state(true, false);
    renderer.set_line_color(Color(0x00, 0x00, 0xff,0xff));


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        renderer.Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
        //renderer.DrawArray(vao, shader);
        model.Draw(renderer,shader);
        glDrawPixels(1024, 720, GL_RGBA, GL_UNSIGNED_BYTE, renderer.get_canvas());
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}