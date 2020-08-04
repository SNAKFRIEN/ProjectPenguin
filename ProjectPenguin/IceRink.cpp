#include "IceRink.h"

IceRink::IceRink(bool initModels)
{
	if (initModels)
	{
		InitModels();
	}
	transform = glm::mat4(1.0f);
}

void IceRink::Draw(Camera& camera)
{
	stadiumModel->Draw(camera);
	iceModel->Draw(camera);
}

void IceRink::InitModels()
{
	stadiumModel = std::make_unique<Model>("Stadium.gltf", transform, "SmoothShader.vert", "SmoothShader.frag");
	iceModel = std::make_unique<Model>("Ice.gltf", transform);
}
