#pragma once

#include <string>

class SaveFile
{
public:
	SaveFile();
	void LoadData(std::string fileName);
	void SaveData(std::string fileName);
	static void RemoveFile(std::string fileName);
	static bool FileExists(std::string fileName);

	//Gameplay values
	int GetHighScore() const;

	void SetHighScore(int inScore);

	//Settings
	unsigned int GetShadowRes() const;
	unsigned int GetMsaaQuality() const;
	int GetSelectedMonitor() const;
	bool GetFullScreenOn() const;

	//TODO: add setters?
private:
	//Gameplay values
	int highScore = 0;

	//Graphics settings
	unsigned int shadowResolution = 1024;
	unsigned int msaa = 4;
	int selectedMonitor = -1;
	bool fullScreenOn = true;

	//TODO: various user settings, or should be included in separate file?	Yep, should absolutely be stored in different file
};