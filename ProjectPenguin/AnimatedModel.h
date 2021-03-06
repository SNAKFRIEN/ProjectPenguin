#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include <glm/glm.hpp>

#include "Shader.h"
#include "Joint.h"
#include "KeyFrame.h"

#include <memory>

class Camera;
class Light;

/*WARNING: This class will leak memory, but due to the predictable nature of the gameplay,
it does not make a difference whether I implement the rule of 5 or not.*/

class AnimatedModel
{
private:
	struct Animation
	{
		float duration = 0.0f;
		std::vector<KeyFrame> frames;
	};
	struct ModelData
	{
		//Geometry
		unsigned int vao = 0;
		size_t nIndices = 0;

		//Shader
		std::unique_ptr<Shader> shader;

		//Texture
		unsigned int texture = 0;	//Only supports models with single textures for now


		//Animation data
		std::vector<Joint> joints;
		std::vector<Joint*> rootJoints;

		std::unordered_map<std::string, Animation> animations;	//Map of all the animations in this model

		//Queue of transforms and poses for all instances of this model
		std::vector<std::tuple<glm::mat4, glm::mat4,  std::vector<glm::mat4>>> renderQueue;
	};
public:
	AnimatedModel(std::string name,
		const glm::mat4& ownerTransform,
		std::string animationName,
		std::string vertexShader = "AnimationCelShader.vert",
		std::string fragShader = "CelShader.frag");
	
	static void Preload(std::string name,
		std::string vertexShader = "AnimationCelShader.vert",
		std::string fragShader = "CelShader.frag");

	void Update(float dt);
	void AddToRenderQueue(Camera& camera);
	static void DrawAllInstances(const Light& light);
	static void DrawShadows(const Light& light);

	void SetAnimation(std::string name);
	void SetCurrentAnimationTime(float time);
	void SetLooping(bool shouldLoop);

	std::string GetAnimation() const;
	float GetCurrentAnimationTime() const;
	bool IsFinished() const;


	//Functionality for joint attachments
	int GetJointIndex(std::string jointName) const;
	const std::vector<glm::mat4>& GetPose() const;
	const glm::mat4& GetTransform() const;
private:
	static ModelData& ConstructModelData(std::string name, std::string vertexShader, std::string fragShader);

	std::vector<glm::mat4> GetJointTransforms() const;	//Retrieve transform per joint
	void ApplyPoseToJointsRecursively(const std::vector<glm::mat4>& pose, Joint& headJoint, const glm::mat4& parentTransform);
private:
	//Animation
	std::string currentAnimation;	//Name of current animation
	float animationTime = 0.0f;	//Current time in animation
	std::vector<glm::mat4> pose;	//Stores pose for when it's time to draw

	bool finished = false;
	bool looping = true;

	//Reference to owner transform
	const glm::mat4& ownerTransform;

	//Data for instancing
	static std::unordered_map<std::string, ModelData> existingModels;
	ModelData& modelData;
};