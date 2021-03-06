#pragma once

#include "UIButton.h"
#include "UINumberDisplay.h"
#include "PenguinWarning.h"
#include "Window.h"
#include "AudioSource.h"

#include <unordered_map>

/*This class is a container for UI elements. Calling its Update function makes sure that buttons maintain their aspect ratio no matter the size of the window*/

class UICanvas
{
public:
	UICanvas(const Window& window, AudioManager& audioManager, float aspectRatio);

	void AddButton(glm::vec2 topLeft, glm::vec2 bottomRight, std::string name, std::string textureName = "DefaultButton.png");
	void AddNumberDisplay(glm::vec2 pos, glm::vec2 scale, Anchor anchor, std::string name);
	void SetUpPenguinWarnings(int numberOfPenguinWarnings);

	UIButton& GetButton(std::string name);
	UINumberDisplay& GetNumberDisplay(std::string name);
	PenguinWarning& GetPenguinWarning(int index);

	void HideForOneFrame(std::string name);	//Hide the element for one frame

	//Recalculate dimensions based on window aspect ratio
	void Update();

	//Render all buttons and then clear the canvas
	void Draw();
private:
	const Window& window;

	//UI elements
	std::unordered_map<std::string, UIButton> buttons;
	std::unordered_map<std::string, UINumberDisplay> numberDisplays;
	std::vector<PenguinWarning> penguinWarnings;

	std::vector<std::string> hiddenElements;

	//Menu dimensions
	const float aspectRatio;
	float width;
	float height;

	//Sounds
	AudioSource buttonQuacker;
};