#include"Common/Application.h"
#include"Example/BlinnPhone.h"
#include"Example/SkeletalAnimation.h"
#include"Example/Pbr.h"
#include"Example/Ibl.h"

#include"tinyddsloader.h"
int main()
{
    //ThreadPool::Init(std::thread::hardware_concurrency());
    ThreadPool::Init(1);

    Application* app = Application::CreateApp("SoftRenderer", 980, 720);

  
  /*  shared_ptr<BlinnPhone> blinn_phone = std::make_shared<BlinnPhone>("Blinn-Phone");
    blinn_phone->Init();
    app->AddScene(blinn_phone);*/

    //shared_ptr<SkeletalAnimation> skeletal_animation = std::make_shared<SkeletalAnimation>("SkeletalAnimation");
    //skeletal_animation->Init();
    //app->AddScene(skeletal_animation);

    
    //shared_ptr<Pbr> pbr = std::make_shared<Pbr>("Pbr");
    //pbr->Init();
    //app->AddScene(pbr);


    std::shared_ptr<Ibl> ibl = std::make_shared<Ibl>("Ibl");
    ibl->Init();

    app->AddScene(ibl);

    app->Run();
}