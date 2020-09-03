#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include "glm/glm.hpp"

#include <unordered_map>

#include "Shader.h"

class Camera;

/*WARNING: This class will leak memory, but due to the predictable nature of the gameplay,
it does not make a difference whether I implement the rule of 5 or not.*/

class Model
{
private:
	struct ModelData
	{
		//Geometry
		unsigned int vao = 0;
		//std::map<int, unsigned int> vbos;
		//unsigned int ebo = 0;
		size_t nIndices = 0;

		//Shader
		std::unique_ptr<Shader> shader;

		//Texture
		unsigned int texture = 0;	//only supports models with single textures for now

		//Queue of transforms (Model and MVP) for all instances of this model
		std::vector<std::pair<glm::mat4, glm::mat4>> renderQueue;
	};
public:
	Model(std::string name,
		const glm::mat4& ownerTransform,
		std::string vertexShader = "CelShader.vert",
		std::string fragShader = "CelShader.frag");
	void AddToRenderQueue(Camera& camera);
	static void DrawAllInstances();
private:
	ModelData& ConstructModelData(std::string name, std::string vertexShader, std::string fragShader);
private:
	//Reference to owner transform
	const glm::mat4& ownerTransform;

	//Data for instancing
	static std::unordered_map<std::string, ModelData> existingModels;
	ModelData& modelData;
};