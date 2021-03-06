#include "audio.h"
#include "osureader.h"
#include "animator.h"
#include <gdiplus.h>
#include <algorithm>

VOID GameFolderInit()
{
	global.totalSongCount = 0;
	global.currentSong = 1;
	HANDLE findFile;
	WIN32_FIND_DATA fNextInfo;
	findFile = FindFirstFile(_T("*"), &fNextInfo);
	WCHAR firstOsuFile[60];
	while (FindNextFile(findFile, &fNextInfo))
	{
		if (fNextInfo.cFileName[0] == '.')
			continue;
		if (wcscmp(fNextInfo.cFileName, _T("res")) == 0)
			continue;
		else if (fNextInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			SetCurrentDirectory(fNextInfo.cFileName);
			HANDLE findOsuFile;
			WIN32_FIND_DATA fOsuInfo;
			wsprintf(firstOsuFile, TEXT("%s.osu"), _T("*"));
			findOsuFile = FindFirstFile(firstOsuFile, &fOsuInfo);
			global.songs.push_back(SONGINFO());
			global.songs.back().beatmapSetId = _wtoi(fNextInfo.cFileName);
			if (findOsuFile != INVALID_HANDLE_VALUE)
			{
				global.songs[global.totalSongCount].osuFile = fOsuInfo.cFileName;
				readBasicInfo(global.songs[global.totalSongCount].osuFile.data(), &global.songs[global.totalSongCount]);
			}
			FindClose(findOsuFile);
			SetCurrentDirectory(_T(".."));

			global.songs[global.totalSongCount].name = fNextInfo.cFileName;
			global.totalSongCount++;
		}
	}
	FindClose(findFile);
}

VOID MCIOffsetInit()
{
	FILE *mciOffsetFile = fopen("mcioffset.txt", "r");
	if(mciOffsetFile)
	{
		int beatmapSetID, offset;
		while (fscanf(mciOffsetFile, "%d %d", &beatmapSetID, &offset) != EOF)
			for (size_t i = 0; i < global.songs.size(); ++i)
				if (global.songs[i].beatmapSetId == beatmapSetID)
				{
					global.songs[i].mciOffset = offset;
					if (global.songs[i].audioLeadIn < offset)
						global.songs[i].audioLeadIn = offset + 100;
				}
		fclose(mciOffsetFile);
	}
}

VOID GlobalInit()
{
	AddFontResource(_T("res/font/fantiquefour.ttf"));
	QueryPerformanceFrequency(&global.clockFrequency);

	GameFolderInit();
	MCIOffsetInit();
	if (global.songs.empty())
	{
		global.status = global.GS_NOSONG;
		return;
	}
	AudioInit();

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	global.status = global.GS_WELCOME;
	global.heroWidth = 38;
	global.heroHeight = 45;
	settings.foggyMode = FALSE;
	settings.hideJudgeLine = FALSE;
	settings.showAccuracyIndicator = FALSE;
	settings.randomMode = FALSE;
	settings.universalOffset = 0;
}

VOID WindowInit(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//  Load Hero Bitmap
	resource.hero[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO1));
	resource.hero[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO2));
	resource.hero[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO3));
	resource.hero[3] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO4));
	resource.hero[4] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO5));
	resource.hero[5] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO6));
	resource.hero[6] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO7));
	resource.hero[7] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO8));
	resource.hero[8] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_HERO9));

	resource.wHero[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO1));
	resource.wHero[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO2));
	resource.wHero[2] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO3));
	resource.wHero[3] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO4));
	resource.wHero[4] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO5));
	resource.wHero[5] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO6));
	resource.wHero[6] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO7));
	resource.wHero[7] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO8));
	resource.wHero[8] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WHITEHERO9));

	//  Load Note Resource
	resource.note[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_NOTEB));
	resource.note[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_NOTEW));

	//  Load Noise Resource
	resource.noise[0] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_NOISEB));
	resource.noise[1] = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_NOISEW));

	//  Load Other Bitmaps
	resource.cloud = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_CLOUD));
	resource.machine = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_MACHINE));
	resource.machineok = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_MACHINEOK));
	resource.pauseButton = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_PAUSEBUTTON));
	resource.welcomeBackground = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance,
		MAKEINTRESOURCE(IDB_WELCOMEBACKGROUND));
	
	//  Register Touch Window To Enable Touch Event
	RegisterTouchWindow(hWnd, 0);

	//  Start Timer
	SetTimer(hWnd, TIMER, 20, NULL);
}

VOID GameInit()
{
	readBeats((global.currSong().name + _T("/") + global.currSong().osuFile).data());
	global.accummulatedTime = 0;
	global.isGamePaused = false;
	QueryPerformanceCounter(&global.beginTime);
	if (global.currSong().audioLeadIn > 0)
	{
		global.accummulatedTime = -global.currSong().audioLeadIn;
		WCHAR firstMp3File[200];
		wsprintf(firstMp3File, TEXT("%s/%s"), global.currSong().name.data(), global.currSong().audioFilename.data());
		AudioOpen(firstMp3File);
		AudioPlayOnce();
		AudioPause();
	}
	else
		SwitchSong();

	killAnimator(&global.blood);
	global.blood = 100.;
	global.accuracyIndicator = 0.;
	for (int i = 0; i < 4; i++)
	{
		global.heroes[i].height = 0.;
		global.heroes[i].startHeight = 0.;
		global.heroes[i].jpCount = 0;
		global.heroes[i].jpStartTime = INT_MIN;
		global.fHeight[i] = 0.;
		global.keySequence[i].clear();
	}
	global.status = global.GS_PLAYING;
}

VOID GameOverInit()
{
	global.finalTime = global.timePass();
	if (global.blood <= 0)
	{
		AudioClose();
		PlaySound(_T("res/sound/falldown.wav"), NULL, SND_ASYNC | SND_FILENAME);
	}

	UINT totalCombo = 0;
	DOUBLE finalScore = 0.;
	for (int i = 0; i < 4; i++)
	{
		totalCombo += global.barriers[i].size();
		for (size_t j = 0; j < global.barriers[i].size(); j++)
		{
			auto keyToBarrier = std::lower_bound(global.keySequence[i].begin(), global.keySequence[i].end(), global.barriers[i][j].msecs);
			INT delta = INT_MAX;
			if (keyToBarrier != global.keySequence[i].begin())
				delta = abs(*(keyToBarrier - 1) - global.barriers[i][j].msecs);
			if (keyToBarrier != global.keySequence[i].end())
				delta = min(delta, abs(*keyToBarrier - global.barriers[i][j].msecs));
			if (delta <= global.currSong().msPerBeat * 0.4 / 4.)
				finalScore += 1;
			else if (delta <= global.currSong().msPerBeat * 0.4 / 3.)
				finalScore += 0.95;
			else if (delta <= global.currSong().msPerBeat * 0.4 / 2.)
				finalScore += 0.6;
		}
	}

	global.finalScore = finalScore / totalCombo;

	global.status = global.GS_GAMEOVER;
}
