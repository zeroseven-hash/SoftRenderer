#include"Common/Application.h"
#include"Example/BlinnPhone.h"
#include"Example/SkeletalAnimation.h"
#include"Example/Pbr.h"


#include"tinyddsloader.h"
int main()
{
    ThreadPool::Init(std::thread::hardware_concurrency());
    Application* app = Application::CreateApp("SoftRenderer", 980, 720);

    using namespace tinyddsloader;

    DDSFile dds;
    auto ret = dds.Load("L:\\dev\\E-Star-Hw\\project-e-star-homework\\resource\\env_maps\\skyDiffuseHDR.dds");
    if (tinyddsloader::Result::Success != ret) {
        std::cout << "Result : " << int(ret) << "\n";
        return 1;
    }

    std::cout << "Width: " << dds.GetWidth() << "\n";
    std::cout << "Height: " << dds.GetHeight() << "\n";
    std::cout << "Depth: " << dds.GetDepth() << "\n";

    std::cout << "Mip: " << dds.GetMipCount() << "\n";
    std::cout << "Array: " << dds.GetArraySize() << "\n";
    for (uint32_t arrayIdx = 0; arrayIdx < dds.GetArraySize(); arrayIdx++) {
        for (uint32_t mipIdx = 0; mipIdx < dds.GetMipCount(); mipIdx++) {
            const auto* imageData = dds.GetImageData(mipIdx, arrayIdx);
            std::cout << "Array[" << arrayIdx << "] "
                << "Mip[" << mipIdx << "]: "
                << "(" << imageData->m_width << ", "
                << imageData->m_height << ", " << imageData->m_depth
                << ")\n";
        }
    }
    std::cout << "Cubemap: " << dds.IsCubemap() << "\n";
    //shared_ptr<BlinnPhone> blinn_phone = std::make_shared<BlinnPhone>("Blinn-Phone");
    //blinn_phone->Init();
    //app->AddScene(blinn_phone);

    //shared_ptr<SkeletalAnimation> skeletal_animation = std::make_shared<SkeletalAnimation>("SkeletalAnimation");
    //skeletal_animation->Init();
    //app->AddScene(skeletal_animation);


    shared_ptr<Pbr> pbr = std::make_shared<Pbr>("Pbr");
    pbr->Init();
    app->AddScene(pbr);


    app->Run();
}