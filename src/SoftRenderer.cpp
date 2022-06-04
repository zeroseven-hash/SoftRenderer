#include"Common/Application.h"
#include"Example/BlinnPhone.h"
#include"Example/SkeletalAnimation.h"
#include<omp.h>
int main()
{
    ThreadPool::Init(std::thread::hardware_concurrency());
    Application* app = Application::CreateApp("SoftRenderer", 980, 720);
    /*shared_ptr<BlinnPhone> blinn_phone = std::make_shared<BlinnPhone>("blinn-phone");
    blinn_phone->Init();*/
    //app->AddScene(blinn_phone);

    shared_ptr<SkeletalAnimation> skeletal_animation = std::make_shared<SkeletalAnimation>("SkeletalAnimation");
    skeletal_animation->Init();
    
    app->AddScene(skeletal_animation);
    app->Run();
}