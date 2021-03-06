#pragma once

#include <glm/glm.hpp>

#include "AnimatedModel.h"
#include "CircleCollider.h"
#include "JointAttachment.h"

#include <random>


class Camera;
class IceRink;
class FishingPenguin;

class Penguin
{
private:
	enum class State
	{
		Walking,
		Thinking
	};
public:
	//REPLACE: Add destructor?
	Penguin(glm::vec3 pos, bool initModel = true);
	Penguin(const Penguin& rhs);
	Penguin operator=(const Penguin& rhs) = delete;
	Penguin(Penguin&& rhs) noexcept;
	Penguin operator=(Penguin&& rhs) = delete;
	
	void AddAccessory(std::string name, std::string joint, std::string vertShader, std::string fragShader);
	void Collide(int index, std::vector<Penguin>& penguins, std::unique_ptr<FishingPenguin>& fishingPenguin, const IceRink& rink);
	void Update(float dt);
	void UpdateAnimation(float dt);
	void Draw(Camera& camera);

	glm::vec3 GetPos() const;
	const AnimatedModel& GetModel() const;
	CircleCollider& GetCollider();

	static constexpr float personalSpaceRadius = 0.25f;	//Collision radius
	static constexpr float minDistanceFromRinkEdges = 0.5f;
private:
	void InitModel();
	void SetState(State newState);
private:
	glm::vec3 pos;

	glm::mat4 transform;

	std::unique_ptr<AnimatedModel> model;
	std::vector<JointAttachment> accessories;

	//Gameplay
	glm::vec3 direction;	//Must be normalized at all times
	float speed = 1.5f;

	State state = State::Walking;	//Always use SetState() to make sure the animation is changed too

	std::mt19937 rng;
	const std::uniform_real_distribution<float> minMaxWalkTime;
	const std::uniform_real_distribution<float> minMaxThinktime;
	float stateCountDown;

	CircleCollider collider;
};