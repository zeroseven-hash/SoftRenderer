#include"Common/Application.h"
#include"Example/BlinnPhone.h"
int main()
{
    Application* app = Application::CreateApp("SoftRenderer", 1420, 980);
    shared_ptr<BlinnPhone> blinn_phone = std::make_shared<BlinnPhone>();
    blinn_phone->Init();
    app->set_scene(blinn_phone);
    app->Run();
}