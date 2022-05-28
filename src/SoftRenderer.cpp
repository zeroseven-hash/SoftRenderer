#include"Common/Application.h"
#include"Example/BlinnPhone.h"
#include<omp.h>
int main()
{
    ThreadPool::Init(std::thread::hardware_concurrency());
    Application* app = Application::CreateApp("SoftRenderer", 980, 720);
    shared_ptr<BlinnPhone> blinn_phone = std::make_shared<BlinnPhone>();
    blinn_phone->Init();
    app->set_scene(blinn_phone);
    app->Run();
}