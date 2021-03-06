#include "Game.h"

#include "Window.h"
#include "GlGetError.h"

#include <iostream>
#include <sstream>
//REMOVE: iomanip likely isn't necessary for final release
#include <iomanip>

#include "glm/gtc/random.hpp"

Game::Game(Window& window)
	:
	window(window),
	player(glm::vec3(0.0f, 0.0f, 0.0f)),
	input(window),
	mainMenu(window, audioManager, 1.0f),
	pauseMenu(window, audioManager, 1.0f),
	gameOverMenu(window, audioManager, 1.0f),
	gameplayUI(window, audioManager, 1.0f),
	tutorialUI(window, audioManager, 1.0f),
	fishingPenguinRotationRange(1.57079f, 4.71238f),
	rng(std::random_device()()),
	light(glm::vec3(0.0f, 10.0f, 0.0f), saveFile.GetShadowRes()),
	screenQuad(window, saveFile),
	penguinDresser(rng),
	randomStackSpawnInterval(10.0f, 30.0f),	//REPLACE these values
	gameOverFlashSound("CameraFlash.wav", audioManager),
	bonkSound("Bonk.wav", audioManager),
	iceSkatingSound0("IceSkatingSnow.wav", audioManager),
	iceSkatingSound1("IceSkatingMetal.wav", audioManager),
	penguinStackSound("Stack.wav", audioManager),
	penguinStackFallSound("StackFall.wav", audioManager),
	windSound("Wind.wav", audioManager),
	windChimeSound("WindChimes.wav", audioManager),
	randomWindChimeInterval(10.0f, 30.0f),
	candyCaneSound("CandyCane.wav", audioManager),
	choir(audioManager)
{
	window.SetMainCamera(&camera);
	window.SetScreenQuad(&screenQuad);
	camera.SetPos(glm::vec3(0.0f, 10.0f, 1.0f));
	
	//Seed randomness for penguin spawns, REPLACE if there's a better way
	srand(std::random_device()());

	//Setup UI
	SetUpMainMenu();
	SetUpPauseMenu();
	SetUpGameOverMenu();

	SetUpGameplayUI();
	SetUpTutorialUI();

	//Tweak audio
	gameOverFlashSound.SetVolume(0.4f);
	bonkSound.SetPitch(0.5f);
	bonkSound.SetVolume(2.0f);
	iceSkatingSound0.SetLooping(true);
	iceSkatingSound0.SetPitch(1.5f);
	iceSkatingSound0.SetVolume(2.0f);
	iceSkatingSound1.SetLooping(true);
	iceSkatingSound1.SetPitch(1.5f);
	iceSkatingSound1.SetVolume(2.5f);
	penguinStackSound.SetPitch(2.0f);
	penguinStackSound.SetLooping(true);
	penguinStackSound.SetVolume(2.0f);
	penguinStackFallSound.SetPitch(2.0f);
	penguinStackFallSound.SetVolume(2.0f);
	windSound.SetLooping(true);
	windSound.SetPitch(0.5f);
	windSound.SetVolume(2.0f);
	windSound.SetFollowListener(true);
	windSound.Play();
	windChimeSound.SetVolume(0.5f);
	candyCaneSound.SetVolume(2.0f);

	//Bake shadows for static objects
	SetUpBakedShadows();

	//Load save data
	saveFile.LoadData("SaveData.json");
	highScore = saveFile.GetHighScore();
	window.SetSelectedMonitor(saveFile.GetSelectedMonitor());
	window.SetFullscreen(saveFile.GetFullScreenOn());

	//Preload some models to save time later
	AnimatedModel::Preload("Goopie.gltf");
	Model::Preload("Crate.gltf");
	Model::Preload("FishingPole.gltf");
	Model::Preload("Bucket.gltf");
	Model::Preload("CandyCane.gltf");
}

void Game::Update()
{
	const float frameTime = ft.Mark();
	switch (state)
	{
	case State::Tutorial:
		UpdateTutorial(frameTime);
		break;
	case State::Playing:
		UpdatePlaying(frameTime);
		break;
	case State::Paused:
		UpdatePauseMenu();
		break;
	case State::MainMenu:
		UpdateMainMenuCam(frameTime);
		UpdateMainMenu(frameTime);
		break;
	case State::GameOverCam:
		UpdateGameOverCam(frameTime);
		break;
	case State::GameOver:
		UpdateGameOver();
		break;
	}

	windChimeSoundCountDown -= frameTime;
	if (windChimeSoundCountDown <= 0.0f)
	{
		windChimeSound.Play();
		windChimeSoundCountDown = randomWindChimeInterval(rng);
	}

	//Toggle full screen
	if (input.IsPressed(GLFW_KEY_LEFT_ALT) && input.IsShortPressed(InputAction::Enter))
	{
		window.SetFullscreen(!window.IsFullScreen());
	}
}

void Game::Draw()
{
	switch (state)
	{
	case State::Tutorial:
		DrawPlaying();
		DrawGamePlayUI();
		DrawTutorialUI();
		break;
	case State::Playing:
		DrawPlaying();
		DrawGamePlayUI();
		if (!tutorialFinished)
		{
			DrawTutorialUI();
		}
		break;
	case State::Paused:
		DrawPlaying();	//Still show paused gameplay in the background
		DrawPauseMenu();
		break;
	case State::MainMenu:
		DrawPlaying();
		DrawMainMenu();
		break;
	case State::GameOverCam:
		DrawPlaying();
		break;
	case State::GameOver:
		DrawPlaying();	//Still show paused gameplay in the background
		DrawGameOverMenu();
		break;
	}
}

bool Game::ReadyToQuit() const
{
	return quit;
}

void Game::SetUpMainMenu()
{
	mainMenu.AddButton(glm::vec2(-0.8f, -0.6f), glm::vec2(0.0f, -0.9f), "Start", "Start.png");
	mainMenu.AddButton(glm::vec2(0.0f, -0.6f), glm::vec2(0.8f, -0.9f), "Quit", "Quit.png");
	mainMenu.AddButton(glm::vec2(-0.5f, 0.9f), glm::vec2(0.5f, 0.4f), "Logo", "Logo.png");
}

void Game::SetUpPauseMenu()
{
	pauseMenu.AddButton(glm::vec2(-0.4f, 0.4f), glm::vec2(0.4f, 0.1f), "Resume", "Resume.png");
	pauseMenu.AddButton(glm::vec2(-0.4f, -0.1f), glm::vec2(0.4f, -0.4f), "Quit", "Quit.png");
}

void Game::SetUpGameOverMenu()
{
	gameOverMenu.AddButton(glm::vec2(-0.7f, 0.9f), glm::vec2(0.7f, 0.3f), "Background", "ScoreScreen.png");
	gameOverMenu.AddButton(glm::vec2(-0.8f, -0.6f), glm::vec2(0.0f, -0.9f), "Retry", "Retry.png");
	gameOverMenu.AddButton(glm::vec2(0.0f, -0.6f), glm::vec2(0.8f, -0.9f), "Quit", "Quit.png");
	gameOverMenu.AddNumberDisplay(glm::vec2(0.0f, -0.0f), glm::vec2(0.25f, 0.5f), Anchor::Center, "Score");
	
	gameOverMenu.AddButton(glm::vec2(-0.4f, -0.3), glm::vec2(0.0f, -0.4f), "PersonalBest", "PersonalBest.png");
	gameOverMenu.AddNumberDisplay(glm::vec2(0.0f, -0.36f), glm::vec2(0.0375f, 0.075f), Anchor::Left, "HighScore");
	gameOverMenu.AddButton(glm::vec2(-0.3f, -0.3), glm::vec2(0.3f, -0.4f), "NewPersonalBest", "NewPersonalBest.png");

	gameOverMenu.GetButton("NewPersonalBest").SetOnColor(glm::vec3(1.0f, 1.0f, 0.2f));
}

void Game::SetUpGameplayUI()
{
	gameplayUI.AddNumberDisplay(glm::vec2(0.0f, 0.9f), glm::vec2(0.03f, 0.06f), Anchor::Center, "Score");
	gameplayUI.GetNumberDisplay("Score").SetNumber(score);
	gameplayUI.AddButton(glm::vec2(-0.6f, 0.905f), glm::vec2(0.6f, 0.895f), "ScoreLine", "ScoreLine.png");
}

void Game::SetUpTutorialUI()
{
	tutorialUI.AddButton(glm::vec2(-1.0f, 0.525f), glm::vec2(1.0f, 0.275f), "Tutorial", "Tutorial.png");
}

void Game::SetUpBakedShadows()
{
	//Draw all objects that can be baked
	iceRink.DrawStatic(camera);
	choir.Draw(camera);

	light.UseBakeTexture();

	glCullFace(GL_FRONT);

	//Prepare shadow FBO
	glViewport(0, 0, light.GetShadowResolutionX(), light.GetShadowResolutionY());
	glBindFramebuffer(GL_FRAMEBUFFER, light.GetFBO());
	glClear(GL_DEPTH_BUFFER_BIT);
	GL_ERROR_CHECK();
	//Bind shader and draw shadows
	light.UseNonAnimationShader();
	Model::DrawShadows(light);
	//Revert to default FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window.GetWidth(), window.GetHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_BACK);

	light.UseNonBakeTexture();
}

void Game::StartPlaying()
{
	//Clear previous run
	penguins.clear();
	collectibles.clear();
	homingPenguins.clear();
	penguinStack.reset();

	score = 0;
	scoreTimer = 0.0f;
	
	totalPlayTime = 0.0f;

	fishingPenguin.reset();
	fishingPenguinSpawned = false;
	iceRink.Reset();

	//Reset spawn timers
	collectibleTimer = 0.0f;
	penguinSpawnTimer = 0.0f;
	penguinStackSpawnTimer = stackedPenguinSpawnTime;
	homingPenguinSpawnTimer = homingPenguinSpawnInterval;

	//Set things up for new run
	player.Reset();

	penguins.reserve(maxPenguins);
	penguinSpawnTimer = 0.0f;

	if (tutorialFinished)
	{
		state = State::Playing;
		window.HideMouse();
		iceSkatingSound0.Play();
		iceSkatingSound1.Play();
	}
	else
	{
		state = State::Tutorial;
	}

	gameplayUI.GetNumberDisplay("Score").SetNumber(score);

	ft.Mark();

	//Ensure at least one physics update takes place before rendering the first frame of gameplay
	UpdatePlaying(0.01f);
}

void Game::UpdateTutorial(float frameTime)
{
	accumulator += frameTime;
	accumulator = std::min(accumulator, 0.02f);
	while (accumulator > deltaTime)
	{
		accumulator -= deltaTime;
		camera.Follow(player.GetPos());
	}
	camera.CalculateVPMatrix();

	tutorialUI.Update();
	if (abs(input.GetForwardAxis()) > 0.1f || abs(input.GetRightAxis()) > 0.1f)
	{
		iceSkatingSound0.Play();
		iceSkatingSound1.Play();
		state = State::Playing;
		window.HideMouse();
	}
}

void Game::UpdatePlaying(float frameTime)
{
	frameTime = std::min(frameTime, 0.1f);

	totalPlayTime += frameTime;
	
	if (totalPlayTime > 5.0f)
	{
		tutorialFinished = true;
	}

	//Update particle effects (do this before adding new particle effects)
	smokeMachine.Update(frameTime);
	plus5Dispenser.Update(frameTime);

	//Spawn new penguins
	if (penguins.size() < maxPenguins)
	{
		penguinSpawnTimer += frameTime;
		if (penguinSpawnTimer > penguinSpawnInterval)
		{
			penguins.emplace_back(spawner.FindOffScreenSpawnPoint(camera.GetPos(), player.GetPos(), camera.GetFOVRadians(), 1.0f));
			auto outfit = penguinDresser.GeneratePenguinOutfit();
			for (auto& accessory : outfit)
			{
				penguins[penguins.size() - 1].AddAccessory(accessory.name, accessory.bone, accessory.vertShader, accessory.fragShader);
			}
			penguinSpawnTimer -= penguinSpawnInterval;
		}
	}
	//Spawn fishingPenguin
	if (!fishingPenguinSpawned && totalPlayTime >= fishingPenguinSpawnTime)
	{
		auto spawn = spawner.FindDistancedSpawnPoint(player.GetPos(),
			10.0f,
			iceRink.GetRight() - iceRink.GetCornerRadius(),
			iceRink.GetTop() - iceRink.GetCornerRadius());
		float rotation = fishingPenguinRotationRange(rng);
		iceRink.SetIcePos(spawn);
		fishingPenguin = std::make_unique<FishingPenguin>(spawn, rotation, audioManager);
		fishingPenguinSpawned = true;
		smokeMachine.SpawnSmoke(spawn);
	}
	//Spawn penguin stack
	penguinStackSpawnTimer -= frameTime;
	if (penguinStackSpawnTimer <= 0.0)
	{
		penguinStackSpawnTimer += randomStackSpawnInterval(rng);
		auto stackSpawn = spawner.FindDistancedSpawnPoint(player.GetPos(),
			12.0f,
			iceRink.GetRight() - iceRink.GetCornerRadius(),
			iceRink.GetTop() - iceRink.GetCornerRadius());
		auto stackTarget = spawner.FindCloseTarget(player.GetPos(), 5.0f);
		penguinStack = std::make_unique<PenguinStack>(stackSpawn, stackTarget, rng);
		smokeMachine.SpawnSmoke(stackSpawn);
		penguinStackSound.Play();
	}
	//Spawn collectibles
	collectibleTimer += frameTime;
	if (collectibleTimer > collectibleInterval && !input.IsPressed(GLFW_KEY_X))
	{
		collectibleTimer -= collectibleInterval;
		glm::vec3 spawn = spawner.FindDistancedSpawnPoint(player.GetPos(),
			5.0f,
			iceRink.GetRight() - iceRink.GetCornerRadius(),
			iceRink.GetTop() - iceRink.GetCornerRadius());
		spawn.y = 10.0f;
		collectibles.emplace_back(spawn);
	}
	//Spawn HomingPenguins
	if (totalPlayTime >= homingPenguinSpawnTime && homingPenguins.size() < maxHomingPenguins)
	{
		homingPenguinSpawnTimer += frameTime;
		if (homingPenguinSpawnTimer >= homingPenguinSpawnInterval)
		{
			homingPenguinSpawnTimer -= homingPenguinSpawnInterval;
			glm::vec3 spawn = spawner.FindDistancedSpawnPoint(player.GetPos(),
				10.0f,
				iceRink.GetRight() - iceRink.GetCornerRadius(),
				iceRink.GetTop() - iceRink.GetCornerRadius());
			smokeMachine.SpawnSmoke(spawn);
			homingPenguins.emplace_back(spawn);
		}
	}

	//Update entities (Fixed deltaTime)
	accumulator += frameTime;
	accumulator = std::min(accumulator, 0.02f);
	while (accumulator > deltaTime)
	{
		player.Update(deltaTime, input);
		iceSkatingSound0.SetPos(player.GetPos());
		iceSkatingSound1.SetPos(player.GetPos());
		camera.Follow(player.GetPos());
		for (HomingPenguin& hp : homingPenguins)
		{
			hp.Update(player, collectibles, iceRink, deltaTime);
		}

		accumulator -= deltaTime;
	}
	//Update entities (Dynamic deltaTime)
	for (Penguin& penguin : penguins)
	{
		penguin.Update(frameTime);
	}
	if (penguinStack)
	{
		penguinStack->Update(frameTime, iceRink, smokeMachine, penguinStackFallSound, bonkSound);
		if (!penguinStack->IsCrashing())
		{
			penguinStackSound.SetPos(penguinStack->GetPos());
		}
		else
		{
			penguinStackSound.Stop();
		}
	}
	camera.CalculateVPMatrix();
	for (Collectible& c : collectibles)
	{
		c.Update(frameTime);
	}
	iceRink.Update(frameTime);
	choir.Update(frameTime, totalPlayTime);
	//Pick up collectibles (either by player or by homing penguin)
	{
		const auto newEnd = std::remove_if(collectibles.begin(), collectibles.end(),
			[&](Collectible& c)
			{
				for (HomingPenguin& hp : homingPenguins)
				{
					if (hp.GetCollider().CalculateCollision(c.GetCollider()).isColliding)
					{
						hp.GiveCandyCane();
						return true;
					}
				}
				if (player.GetCollider().CalculateCollision(c.GetCollider()).isColliding)
				{
					score += 5;
					plus5Dispenser.Dispense(player.GetPos());
					candyCaneSound.SetPos(player.GetPos());
					candyCaneSound.Play();
					gameplayUI.GetNumberDisplay("Score").SetNumber(score);
					return true;
				}
				return false;
			});
		collectibles.erase(newEnd, collectibles.end());
	}

	//Check collisions
	for (int i = 0; i < penguins.size(); i++)
	{
		penguins[i].Collide(i, penguins, fishingPenguin, iceRink);
	}
	for (HomingPenguin& hp : homingPenguins)
	{
		if (hp.IsLockedOnToPlayer())
		{
			hp.Collide(iceRink, smokeMachine, bonkSound);
		}
	}
	bool gameOver = false;
	if (player.IsColliding(penguins, fishingPenguin, penguinStack, homingPenguins, iceRink))
	{
		gameOver = true;
	}

	//Remove homingPenguins that have crashed into the iceRink
	{
		const auto newEnd = std::remove_if(homingPenguins.begin(), homingPenguins.end(),
			[](HomingPenguin& hp)
			{
				return hp.IsFinished();
			});
		homingPenguins.erase(newEnd, homingPenguins.end());
	}

	//Update audio listener
	audioManager.SetListenerPosition(camera.GetPos());
	audioManager.SetListenerOrientation(glm::normalize(player.GetPos() - camera.GetPos()));

	//Update animations
	player.UpdateAnimation(frameTime);
	for (Penguin& penguin : penguins)
	{
		penguin.UpdateAnimation(frameTime);
	}
	if (fishingPenguinSpawned)
	{
		fishingPenguin->UpdateAnimation(frameTime);
	}
	if (penguinStack)
	{
		penguinStack->UpdateAnimation(frameTime);
	}
	for (HomingPenguin& hp : homingPenguins)
	{
		hp.UpdateAnimation(frameTime);
	}

	//Play game over animation
	if (gameOver)
	{
		iceSkatingSound0.Stop();
		iceSkatingSound1.Stop();
		penguinStackSound.Stop();
		state = State::GameOverCam;
		EndPlaying();
	}

	//Update score and gameplay UI
	scoreTimer += frameTime;
	while (scoreTimer > scoreInterval)
	{
		score++;
		gameplayUI.GetNumberDisplay("Score").SetNumber(score);
		scoreTimer -= 1.0f;
	}
	gameplayUI.Update();
	//Count number of homing penguins
	std::vector<HomingPenguin*> lockedOnPenguins;
	for (HomingPenguin& p : homingPenguins)
	{
		if (p.IsLockedOnToPlayer() || (p.IsLockedOnToCollectible() && p.TargetInSight()))
		{
			lockedOnPenguins.emplace_back(&p);
		}
	}
	gameplayUI.SetUpPenguinWarnings((int)lockedOnPenguins.size());
	for (int i = 0; i < lockedOnPenguins.size(); i++)
	{
		gameplayUI.GetPenguinWarning(i).Update(lockedOnPenguins[i]->GetPos(), camera, window.GetDimensions());
		gameplayUI.GetPenguinWarning(i).SetColor(lockedOnPenguins[i]->IsLockedOnToPlayer());
	}

	//REMOVE output fps and player pos
	//std::cout << "fps: " << std::fixed << std::setprecision(2) << (1.0f / frameTime) << std::endl;
	//std::cout << "Player x: " << std::fixed << std::setprecision(2) << player.GetPos().x << " y: " << player.GetPos().z << std::endl;

	if (input.IsShortPressed(InputAction::Pause))
	{
		iceSkatingSound0.Stop();
		iceSkatingSound1.Stop();
		penguinStackSound.Stop();
		state = State::Paused;
		window.ShowMouse();
	}

	//Only used for interpolating camera when returning to main menu
	currentCamLookat = player.GetPos();
}

void Game::UpdatePauseMenu()
{
	pauseMenu.Update();
	if (input.IsShortPressed(InputAction::Pause))
	{
		iceSkatingSound0.Play();
		iceSkatingSound1.Play();
		state = State::Playing;
		window.HideMouse();
	}
	if (pauseMenu.GetButton("Resume").UpdateAndCheckClick(input))
	{
		ft.Mark();
		iceSkatingSound0.Play();
		iceSkatingSound1.Play();
		state = State::Playing;
		window.HideMouse();
	}
	if (pauseMenu.GetButton("Quit").UpdateAndCheckClick(input))
	{
		EndPlaying();
		state = State::MainMenu;
	}
}

void Game::UpdateMainMenuCam(float frameTime)
{
	glm::vec3 newPos = glm::mix(camera.GetPos(), menuCamPos, 0.03f);
	currentCamLookat = glm::mix(currentCamLookat, glm::vec3(0.0f, 10.0f, 0.0f), 0.03f);
	camera.LookAt(newPos, currentCamLookat);

	camera.CalculateVPMatrix();
}

void Game::UpdateMainMenu(float frameTime)
{
	//Update ferris wheel and carousel while rest of game is frozen
	iceRink.UpdateFerrisWheelAndCarousel(frameTime);

	mainMenu.Update();
	if (mainMenu.GetButton("Start").UpdateAndCheckClick(input))
	{
		StartPlaying();
	}
	if (mainMenu.GetButton("Quit").UpdateAndCheckClick(input))
	{
		window.SetTitle(":(");
		quit = true;
	}
}

void Game::UpdateGameOverCam(float frameTime)
{
	screenEffect.Update(frameTime);
	if (nGameOverFlashes >= maxGameOverFlashes && screenEffect.GetCurrentEffectType() != ScreenEffect::EffectType::Flash)
	{
		state = State::GameOver;
		nGameOverFlashes = 0;
	}
	else if (screenEffect.GetCurrentEffectType() != ScreenEffect::EffectType::Flash)
	{
		//Select camPos
		bool found = false;
		glm::vec2 camPosXZ;
		while (!found)
		{
			camPosXZ = glm::circularRand(7.0f);
			//Make sure it's in the rink
			camPosXZ += glm::vec2(player.GetPos().x, player.GetPos().z);
			float maxOutOfRink = 4.0f;
			if (camPosXZ.x < iceRink.GetRight() + maxOutOfRink
				&& camPosXZ.x > -iceRink.GetRight() - maxOutOfRink
				&& camPosXZ.y < iceRink.GetTop() + maxOutOfRink
				&& camPosXZ.y > -iceRink.GetTop() - maxOutOfRink)
			{
				found = true;
			}
		}
		float camPosY = glm::linearRand(3.0f, 13.0f);
		camera.LookAt(glm::vec3(camPosXZ.x, camPosY, camPosXZ.y), player.GetPos());
		camera.CalculateVPMatrix();
		//Do flash
		screenEffect.SetFlashEffect(1.3f);
		gameOverFlashSound.SetPos(camera.GetPos());
		gameOverFlashSound.Play();
		nGameOverFlashes++;
	}
}

void Game::UpdateGameOver()
{
	if (newBest)
	{
		gameOverMenu.HideForOneFrame("PersonalBest");
		gameOverMenu.HideForOneFrame("HighScore");
	}
	else
	{
		gameOverMenu.HideForOneFrame("NewPersonalBest");
	}

	gameOverMenu.Update();
	if (gameOverMenu.GetButton("Retry").UpdateAndCheckClick(input))
	{
		StartPlaying();
	}
	if (gameOverMenu.GetButton("Quit").UpdateAndCheckClick(input))
	{
		state = State::MainMenu;
	}
}

void Game::EndPlaying()
{
	newBest = score > highScore;
	if (newBest)
	{
		highScore = score;
	}

	window.ShowMouse();

	gameOverMenu.GetNumberDisplay("Score").SetNumber(score);
	gameOverMenu.GetNumberDisplay("HighScore").SetNumber(highScore);

	saveFile.SetHighScore(highScore);
	saveFile.SaveData("SaveData.json");

	score = 0;
	scoreTimer = 0.0f;
}

void Game::DrawShadows()
{
	glCullFace(GL_FRONT);
	//Prepare shadow FBO
	glViewport(0, 0, light.GetShadowResolutionX(), light.GetShadowResolutionY());
	glBindFramebuffer(GL_FRAMEBUFFER, light.GetFBO());
	glClear(GL_DEPTH_BUFFER_BIT);
	GL_ERROR_CHECK();
	//Bind shader and draw shadows
	light.UseAnimationShader();
	AnimatedModel::DrawShadows(light);
	light.UseNonAnimationShader();
	Model::DrawShadows(light);
	//Revert to default FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, (GLsizei)window.GetDimensions().x, (GLsizei)window.GetDimensions().y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
}

void Game::DrawPlaying()
{
	//Draw all items that cast shadows
	player.Draw(camera);
	for (Penguin& p : penguins)
	{
		p.Draw(camera);
	}
	if (fishingPenguinSpawned)
	{
		fishingPenguin->Draw(camera);
	}
	if (penguinStack)
	{
		penguinStack->Draw(camera);
	}
	iceRink.DrawNonStatic(camera, GetCandyCanePositions());
	for (HomingPenguin& hp : homingPenguins)
	{
		hp.Draw(camera);
	}

	//Cast shadows
	DrawShadows();

	//Draw all items that don't cast (dynamic) shadows
	iceRink.DrawStatic(camera);
	choir.Draw(camera);
	for (Collectible& c : collectibles)
	{
		c.Draw(camera);
	}

	//Bind screenQuad
	screenQuad.StartFrame();
	GL_ERROR_CHECK();

	//Draw all entities
	AnimatedModel::DrawAllInstances(light);
	Model::DrawAllInstances(light);
	glEnable(GL_BLEND);
	smokeMachine.Draw(camera);
	glDisable(GL_BLEND);

	screenQuad.EndFrame();

	//Draw using effect
	screenEffect.UseEffect();
	auto screenTexture = screenQuad.GetTexture();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTexture);

	screenQuad.Draw();
}

void Game::DrawGamePlayUI()
{
	gameplayUI.Draw();

	glEnable(GL_BLEND);
	plus5Dispenser.Draw(camera);
	glDisable(GL_BLEND);
}

void Game::DrawTutorialUI()
{
	tutorialUI.Draw();
}

void Game::DrawPauseMenu()
{
	pauseMenu.Draw();
}

void Game::DrawMainMenu()
{
	mainMenu.Draw();
}

void Game::DrawGameOverMenu()
{
	gameOverMenu.Draw();
}

std::vector<glm::vec3> Game::GetCandyCanePositions() const
{
	std::vector<glm::vec3> result;
	for (const Collectible& candyCane : collectibles)
	{
		result.emplace_back(candyCane.GetPos());
	}
	return std::move(result);
}
