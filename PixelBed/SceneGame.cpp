#include "include.h"

struct PixelBuffer {
	unsigned char* pixels;
	int w, h, bpp;
};

static struct sSceneGame : public ZL_Scene
{
	ZL_Surface srfLogo;
	ZL_Surface srfBuffer;
	PixelBuffer img, frame;

	sSceneGame() : ZL_Scene(SCENE_GAME) { }

	void InitGlobal()
	{
		srfLogo = ZL_Surface("Data/ZILLALIB.png").SetDrawOrigin(ZL_Origin::Center);
		frame.w = (int)ZLWIDTH;
		frame.h = (int)ZLHEIGHT;
		frame.bpp = 3;
		frame.pixels = new unsigned char[frame.w * frame.h * frame.bpp];
		srfBuffer = ZL_Surface(frame.pixels, frame.w, frame.h, frame.bpp);
		memset(frame.pixels, 0, frame.w * frame.h * frame.bpp);

		ZL_Surface srfTmp;
		srfTmp.LoadImg("Data/BRICKS.png", &img.pixels, img.w, img.h, img.bpp);
	}

	void SetPixel(PixelBuffer* pb, const int x, const int y, unsigned char r, unsigned char g, unsigned char b) {
		unsigned char* p = pb->pixels;
		p += (y * pb->w + x) * pb->bpp;
		*p++ = r;
		*p++ = g;
		*p++ = b;
	}

	void GetPixel(PixelBuffer* pb, const int x, const int y, unsigned char& r, unsigned char& g, unsigned char& b) {
		unsigned char* p = pb->pixels;
		p += (y * pb->w + x) * pb->bpp;
		r = *p++;
		g = *p++;
		b = *p++;
	}

	void Draw()
	{
		/*
		unsigned char* p = pixels;
		int t = rand() % 255;
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++) {
				*p++ = (x * y + t) % 255;
				*p++ = (x + y * t) % 255;
				*p++ = (x * x + t * y * y) % 255;
			}
		*/
		unsigned char r, g, b;
		for (int y = 0; y < img.h; y++) {
			for (int x = 0; x < img.w; x++) {
				GetPixel(&img, x, y, r, g, b);
				SetPixel(&frame, x, frame.h - y, r, g, b);
			}
		}
		
		srfBuffer.Update(frame.pixels, frame.w, frame.h, frame.bpp);
		srfBuffer.Draw(0, 0);
//		srfBuffer.Clear();

		scalar rotation = s(ZLTICKS) / s(1000);
		scalar scale = 1 + (s(0.2) * ssin(rotation * 3));
		srfLogo.SetRotate(rotation);
		srfLogo.SetScale(scale);
		srfLogo.Draw(ZLHALFW, ZLHALFH);
	}

	void StartGame()
	{
	}

	void InitAfterTransition()
	{
		ZL_Display::sigPointerDown.connect(this, &sSceneGame::OnPointerDown);
		ZL_Display::sigPointerUp.connect(this, &sSceneGame::OnPointerUp);
		ZL_Display::sigPointerMove.connect(this, &sSceneGame::OnPointerMove);
		ZL_Display::sigKeyDown.connect(this, &sSceneGame::OnKeyDown);
		ZL_Display::sigKeyUp.connect(this, &sSceneGame::OnKeyUp);
		ZL_Display::sigActivated.connect(this, &sSceneGame::OnActivated);
		StartGame();
	}

	void DeInitLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		delete[] img.pixels;
		delete[] frame.pixels;
	}

	void OnPointerDown(ZL_PointerPressEvent& e)
	{
	}

	void OnPointerUp(ZL_PointerPressEvent& e)
	{
	}

	void OnPointerMove(ZL_PointerMoveEvent& e)
	{
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (e.key == ZLK_ESCAPE) ZL_Application::Quit();
	}

	void OnKeyUp(ZL_KeyboardEvent& e)
	{
	}

	void OnActivated(ZL_WindowActivateEvent& e)
	{
	}

	void Calculate()
	{
	}

} SceneGame;
