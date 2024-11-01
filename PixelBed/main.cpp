#include "include.h"

static struct sPixelBed : public ZL_Application
{
	ZL_Font fntMain;

	sPixelBed() : ZL_Application(120) { }

	virtual void Load(int argc, char *argv[])
	{
		if (!ZL_Application::LoadReleaseDesktopDataBundle()) return;
		if (!ZL_Display::Init("PixelBed", 800, 600, ZL_DISPLAY_ALLOWRESIZEHORIZONTAL)) return;
		fntMain = ZL_Font("Data/fntMain.png");
		ZL_Display::ClearFill(ZL_Color::White);
		ZL_Display::SetAA(true);
		ZL_Audio::Init();
		ZL_Application::SettingsInit("PixelBed");		
		ZL_SceneManager::Init(SCENE_GAME);
	}

	//display fps
	void AfterFrame()
	{
		fntMain.Draw(ZLFROMW(30), ZLFROMH(30), (const char*)ZL_String::format("%d FPS", FPS), ZL_Color::White, ZL_Origin::CenterRight);
	}
} PixelBed;
