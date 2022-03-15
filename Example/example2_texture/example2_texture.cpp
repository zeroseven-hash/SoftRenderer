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


    Canvas texture(256, 256);
    //texture.LoadFile("output.bmp");
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            int k = (x / 32 + y / 32) & 1;
            texture.set_pixel(x, y, k ? Color(0xff,0xff,0xff,0xff) : Color(0xff,0x3f,0xbc,0xef));
        }
    }

    
    struct V
    {
        TinyMath::Vec4f pos_;
        TinyMath::Vec4f color_;
        TinyMath::Vec2f coords;
    };

    //TinyMath::Mat4f model = TinyMath::Tranlate(TinyMath::Mat4f::GetIdentity(), TinyMath::Vec3f(0.5f, 0.0f, 0.0f));
    //model = TinyMath::Scale(model, TinyMath::Vec3f(0.5f, 0.5f, 0.5f));
    //TinyMath::Mat4f model = TinyMath::Rotate(TinyMath::Mat4f::GetIdentity(), 30.0f, TinyMath::Vec3f(1.0f, 0.0f, 0.0f));
    TinyMath::Mat4f view = TinyMath::LookAt(TinyMath::Vec3f(0.0f, -0.3, 0.0f), TinyMath::Vec3f(0.0, 0.0, -1.0), TinyMath::Vec3f(0.0, 1.0, -1.0));
    TinyMath::Mat4f perpective = TinyMath::Perspective(60.0f, 640 / 480.0f, 1.0f, 500.0f);
    
    //TinyMath::Mat4f view = TinyMath::Lookat({ -0.7, 0, 1.5 }, { 0,0,0 }, { 0,0,1 });	// ÉãÏñ»ú·½Î»
    //TinyMath::Mat4f proj = TinyMath::Perspective(3.1415926f * 0.5f, 800 / 600.0, 1.0, 500.0f);
    TinyMath::Mat4f mvp = perpective*view ;

    std::vector<V> vertices = {
        { { -0.5, 0.5, -1.5f, 1}, {0, 1, 0, 1},{0,1} },
        { { -0.5, -0.5, -1.5f, 1}, {0, 0, 1, 1},{0,0}},
        { {  0.5,  0.5, -1.5, 1}, {1, 0, 0, 1},{1,1} },
        {{0.5,-0.5,-1.5,1},{0.5,0.5,0.5,1},{1,0}}
    };
    std::vector<uint32_t> indices = { 0,1,2,1,3,2};
    VertexArrayBuffer vao(std::move(vertices), std::move(indices));
    const int VARYING_COLOR = 0;
    const int VARYING_COORD = 1;
    Shader<V> shader;
    
    shader.vertex_shader_ = [&](const VertexArrayBuffer<V>& vao,int index, ShaderContext& output)->TinyMath::Vec4f {
        auto& v = vao.get_vertex(index);
        output.varying_vec4f_[VARYING_COLOR] = v.color_;
        output.varying_vec2f_[VARYING_COORD] = v.coords;
        return mvp*v.pos_;
    };
    shader.fragment_shader_ = [&](ShaderContext& input)->TinyMath::Vec4f {
        TinyMath::Vec2f coord = input.varying_vec2f_[VARYING_COORD];
        return texture.Sample2D(coord).TransformToVec();
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