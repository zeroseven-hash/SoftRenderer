#include "Ibl.h"

void Ibl::Init()
{
	m_skybox = TextureCube::Create("../assets/skybox/skyEnvHDR.dds");
}

void Ibl::ImguiUpdate()
{
}

void Ibl::Update(TimeStep ts, Input::MouseState mouse_state)
{
}

void Ibl::OnEvent(const Event* e)
{
}
