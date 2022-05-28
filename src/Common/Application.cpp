#include"Application.h"
#include"Scene.h"
#include"Event.h"

#include<functional>

#include<glad/glad.h>
#include <GLFW/glfw3.h> 
#include<imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include"Renderer/Renderer.h"
Application* Application::s_instance = nullptr;
Application::Application(const char* name, uint32_t width, uint32_t height)
:m_name(name),m_width(width),m_height(height)
{
    m_last_time = 0.0f;
    Init();
}


void Application::Init()
{
    // Setup window
    if (!glfwInit())
        return ;


    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(m_width, m_height, m_name.c_str(), NULL, NULL);
    if (window == NULL)
        return ;
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSwapInterval(0); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    Input::Init(window, std::bind(&Application::OnEvent, this, std::placeholders::_1));
    //init scene
 
    Renderer::Init(m_width, m_height);
    if(m_scene) m_scene->Init();
    
    
}
void Application::Run()
{
    auto window = glfwGetCurrentContext();
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(m_scene) m_scene->ImguiUpdate();
        
        
        // Rendering
        float cur_time = (float)glfwGetTime();
        TimeStep ts(cur_time - m_last_time);
        m_last_time = cur_time;
        if(m_scene) m_scene->Update(ts);
        

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        auto& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }
}



void Application::Destory()
{
    // Cleanup
    Renderer::Destory();
    m_scene->Destory();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(glfwGetCurrentContext());
    glfwTerminate();
}

void Application::OnEvent(const Event* e)
{
    m_scene->OnEvent(e);
}

Application* Application::CreateApp(const char* name, uint32_t width, uint32_t height)
{
    if (s_instance == nullptr)
        s_instance = new Application(name, width, height);
    return s_instance;
}
