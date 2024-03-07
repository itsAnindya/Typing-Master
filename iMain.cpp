#include "iGraphics.h"
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <windows.h>
#include <ctime>
#include <string.h>

#pragma comment(lib, "winmm.lib")

#define GRIDLINES 0
#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define BGCOLOR_RED 255
#define BGCOLOR_GREEN 255
#define BGCOLOR_BLUE 255
#define TEXTCOLOR_RED 255
#define TEXTCOLOR_GREEN 255
#define TEXTCOLOR_BLUE 255
#define MYFONT GLUT_BITMAP_TIMES_ROMAN_24
#define CLOCKFONT GLUT_BITMAP_9_BY_15
#define NUM_OF_WORDS 9894
#define SOLIDBG 0
#define TICK_X 300
#define LINE_LENGTH 75

#pragma comment(lib, "winmm.lib")

int timer[10];
char screenText[50][1000];
char bgdir[100] = "assets\\image\\background.bmp";
int currentCursorIndex = 0, currentLineIndex = 0, maxLineIndex = 10;
int line_x_pos = 40, line_y_pos = 380;
int max_cols = 100;
int num_of_lines = 0;
int max_num_of_lines = 5;
int wpm = 0, words = 0, accuracy;
int timerRecord[20];
unsigned short menuButton = 0; // 0 - None, 1 - Play, 2 - Profile, 3 - Options, 4 - Leaderboard, 5 - Exit
char sentence[5000], word[50][20], paragraph[1024];
char displayClock[11], displayWPM[4];
char meaningfulWord[10000][20];
bool timerStatus = false;
bool randomly_generated_words = false;
// Scenes
bool typing = false;
bool menu = true, playmenu = false, durationSelection = false;
int durationStatus = 0; // 0 - 30s, 1 - 60s, 2 - 90s, 3 - 120s, 4 - 180s, 5 - 300s, 6 - 600s
bool sound = true;		// true means background music will be turned on
bool result = false;
bool profilemenu = false, createProfileMenu = false; // For profile menu
bool leaderboard = false;							 // Screen that shows leaderboard
bool loadProfileMenu = false;						 // Menu shown after clicking 'Load profile'
bool optionsMenu = false;
bool exerciseLevel[5] = {false};
bool isMouseOnPlay = false;
POINT cursorPos;
HWND hwnd;
RECT rect;
bool keyboardControl = false; // This becomes 1 when keyboard takes over mouse control
int charCount_text;
int errorCount = 0, duration = 30, savedDuration = 30;
int totalTypedEntries = 0, grossWPM, netWPM;
char errorDisplay[5];
char durationText[20];
char username_display[33]; // This text is shown in createprofile menu while typing
int totalParagraphLength = 0;
int user_sortedList[8] = {0, 1, 2, 3, 4, 5, 6, 7};
unsigned short selectedProfileNo = 0;
unsigned short loggedInProfileNo = 0;
bool logInStatus = false;

// Functions
void calculateResult();
void createUser(char *username);
void sort_users();
void list_netWPM();
void showLeaderboard();
void loadProfileList();
void filecpy(char src[], char dest[]);
void deleteProfile(int selectedProfile); // takes 1-8 as input.

// User Profile
int userCount;
char countfilename[50] = "user\\usercount.txt";

struct usercontent
{
	char username[33];
	int gross_wpm;
	int accuracy;
	int net_wpm;
	int best_net_wpm;
} user[256];

void displayError()
{
	itoa(errorCount, errorDisplay, 10);
}

void countUsers()
{
	FILE *count;
	count = fopen(countfilename, "r");
	fscanf(count, "%d", &userCount);
	fclose(count);
}

void loadUsers()
{
	countUsers();
	FILE *local_fptr;
	char profile[50];
	for (int i = 0; i < userCount; i++)
	{
		sprintf(profile, "user\\user_%d.txt", i);
		local_fptr = fopen(profile, "r");
		fscanf(local_fptr, "%s\n%d\n%d\n%d\n%d", user[i].username, &user[i].gross_wpm, &user[i].accuracy, &user[i].net_wpm, &user[i].best_net_wpm);
		fclose(local_fptr);
	}
	fclose(local_fptr);
}

struct clock
{
	int hour;
	int minute;
	int second;
	int centisecond;
} timerClock, screenClock, accurateClock;

struct vector
{
	int x;
	int y;
} mousePos, textCursor, max_textCursor;

void displayInputText()
{
	for (int i = 0; i < strlen(paragraph); i += LINE_LENGTH)
	{
		iText(line_x_pos, line_y_pos - 25 * (i / LINE_LENGTH), screenText[i / LINE_LENGTH], MYFONT);
	}
}

void setDuration()
{
	switch (durationStatus)
	{
	case 0:
		duration = 30;
		break;
	case 1:
		duration = 60;
		break;
	case 2:
		duration = 90;
		break;
	case 3:
		duration = 120;
		break;
	case 4:
		duration = 180;
		break;
	case 5:
		duration = 300;
		break;
	case 6:
		duration = 600;
		break;
	default:
		break;
	};
	savedDuration = duration;
}

void handleDuration()
{
	char timeLeft[2][5];
	if (duration <= 0)
	{
		typing = false;
		result = true;
		calculateResult();
		iPauseTimer(timer[2]);
	}
	itoa(duration % 60, timeLeft[1], 10);
	itoa(duration / 60, timeLeft[0], 10);
	// strcpy(durationText, timeLeft[0]);
	// strcpy(durationText, "m");
	// strcpy(durationText, timeLeft[1]);
	// strcpy(durationText, "s");
	durationText[0] = '0' + ((duration / 60) / 10);
	durationText[1] = '0' + ((duration / 60) % 10);
	durationText[2] = ':';
	durationText[3] = '0' + ((duration % 60) / 10);
	durationText[4] = '0' + ((duration % 60) % 10);
	durationText[5] = '\0';
	duration--;
}

char *numToStr(int num)
{
	char str_num[100];
	itoa(num, str_num, 10);
	return str_num;
}

unsigned short checkRect() // Checks the interesting rectangles inside which the mouse pointer does funny things
{
	if (menu)
	{
		if (mousePos.x >= 67 && mousePos.x <= 190 && mousePos.y >= 298 && mousePos.y <= 298 + 54)
		{
			keyboardControl = false;
			return 1;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 256 && mousePos.y >= 371 && mousePos.y <= 371 + 54)
		{
			keyboardControl = false;
			return 2;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 264 && mousePos.y >= 442 && mousePos.y <= 442 + 54)
		{
			keyboardControl = false;
			return 3;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 374 && mousePos.y >= 516 && mousePos.y <= 516 + 54)
		{
			keyboardControl = false;
			return 4;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 177 && mousePos.y >= 590 && mousePos.y <= 590 + 54)
		{
			keyboardControl = false;
			return 5;
		}
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
	else if (playmenu)
	{
		if (mousePos.x >= 67 && mousePos.x <= 287 && mousePos.y >= 298 && mousePos.y <= 298 + 54)
		{
			keyboardControl = false;
			return 1;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 317 && mousePos.y >= 371 && mousePos.y <= 371 + 54)
		{
			keyboardControl = false;
			return 2;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 214 && mousePos.y >= 442 && mousePos.y <= 442 + 54)
		{
			keyboardControl = false;
			return 3;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 380 && mousePos.y >= 516 && mousePos.y <= 516 + 54)
		{
			keyboardControl = false;
			return 4;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 245 && mousePos.y >= 590 && mousePos.y <= 590 + 54)
		{
			keyboardControl = false;
			return 5;
		}
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
	else if (optionsMenu)
	{
		if (mousePos.x >= 79 && mousePos.x <= 249 && mousePos.y >= 586 && mousePos.y <= 631)
		{
			keyboardControl = false;
			return 2;
		}
		if (mousePos.x >= 67 && mousePos.x <= 429 && mousePos.y >= 296 && mousePos.y <= 347)
		{
			keyboardControl = false;
			return 1;
		}
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
	else if (durationSelection)
	{
		if (mousePos.x >= 546 && mousePos.x <= 583 && mousePos.y >= 303 && mousePos.y <= 340)
		{
			keyboardControl = false;
			return 1;
		}
		else if (mousePos.x >= 616 && mousePos.x <= 648 && mousePos.y >= 305 && mousePos.y <= 338)
		{
			keyboardControl = false;
			return 2;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 252 && mousePos.y >= 516 && mousePos.y <= 516 + 54)
		{
			keyboardControl = false;
			return 3;
		}
		else if (mousePos.x >= 518 && mousePos.x <= 708 && mousePos.y >= 516 && mousePos.y <= 516 + 54)
		{
			keyboardControl = false;
			return 4;
		}
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
	else if (result)
	{
		if (mousePos.x >= 84 && mousePos.x <= 581 && mousePos.y >= 586 && mousePos.y <= 634)
		{
			keyboardControl = false;
			return 1;
		}
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
	else if (profilemenu)
	{
		if (mousePos.x >= 67 && mousePos.x <= 435 && mousePos.y >= 298 && mousePos.y <= 298 + 54)
		{
			keyboardControl = false;
			return 1;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 376 && mousePos.y >= 371 && mousePos.y <= 371 + 54)
		{
			keyboardControl = false;
			return 2;
		}
		else if (mousePos.x >= 67 && mousePos.x <= 245 && mousePos.y >= 590 && mousePos.y <= 590 + 54)
		{
			keyboardControl = false;
			return 3;
		}
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
	else if (createProfileMenu)
	{
		if (userCount < 8)
		{
			if (mousePos.x >= 85 && mousePos.x <= 242 && mousePos.y >= 590 && mousePos.y <= 590 + 54)
			{
				keyboardControl = false;
				return 1;
			}
			else if (mousePos.x >= 500 && mousePos.x <= 681 && mousePos.y >= 590 && mousePos.y <= 590 + 54)
			{
				keyboardControl = false;
				return 2;
			}
			if (!keyboardControl)
			{
				return 0;
			}
			else
			{
				return menuButton;
			}
		}
		else
		{
			if (mousePos.x >= 85 && mousePos.x <= 320 && mousePos.y >= 590 && mousePos.y <= 590 + 54)
			{
				keyboardControl = false;
				return 1;
			}
			if (!keyboardControl)
			{
				return 0;
			}
			else
			{
				return menuButton;
			}
		}
	}
	else if (leaderboard)
	{
		if (mousePos.x >= 85 && mousePos.x <= 244 && mousePos.y >= 631 && mousePos.y <= 631 + 54)
		{
			keyboardControl = false;
			return 1;
		}
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
	else if (loadProfileMenu)
	{
		if (mousePos.x >= 85 && mousePos.x <= 244 && mousePos.y >= 631 && mousePos.y <= 631 + 54)
		{
			keyboardControl = false;
			return 1;
		}
		else if (mousePos.x >= 357 && mousePos.x <= 477 && mousePos.y >= 631 && mousePos.y <= 631 + 54)
		{
			keyboardControl = false;
			return 2;
		}
		else if (mousePos.x >= 571 && mousePos.x <= 747 && mousePos.y >= 631 && mousePos.y <= 631 + 54)
		{
			keyboardControl = false;
			return 3;
		}
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >= 216 && mousePos.y <= 216 + 50)
		// {
		// 	keyboardControl = false;
		// 	return 4;
		// }
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >= 216+50+1 && mousePos.y <= 216+50*2)
		// {
		// 	keyboardControl = false;
		// 	return 5;
		// }
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >= 216+50*2 + 1 && mousePos.y <= 216+50*3)
		// {
		// 	keyboardControl = false;
		// 	return 6;
		// }
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >= 216+50*3 + 1 && mousePos.y <= 216+50 * 4)
		// {
		// 	keyboardControl = false;
		// 	return 7;
		// }
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >= 216+50 * 4 + 1 && mousePos.y <= 216+50 * 5)
		// {
		// 	keyboardControl = false;
		// 	return 8;
		// }
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >= 216+50 * 5 + 1 && mousePos.y <= 216+50 * 6)
		// {
		// 	keyboardControl = false;
		// 	return 9;
		// }
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >=216+50 * 6 + 1 && mousePos.y <= 216+50 * 7)
		// {
		// 	keyboardControl = false;
		// 	return 10;
		// }
		// else if (mousePos.x >= 70 && mousePos.x <= 700 && mousePos.y >= 216+50 * 7 + 1 && mousePos.y <= 216+50 * 8)
		// {
		// 	keyboardControl = false;
		// 	return 11;
		// }
		if (!keyboardControl)
		{
			return 0;
		}
		else
		{
			return menuButton;
		}
	}
}

char displayAccuracy[20], displayGrossWPM[20], displayNetWPM[20];
void calculateResult()
{
	accuracy = (totalTypedEntries - errorCount) * 100 / totalTypedEntries;
	grossWPM = (totalTypedEntries) * 12 / savedDuration;
	netWPM = grossWPM - errorCount * 12 / savedDuration;
	sprintf(displayAccuracy, "Accuracy: %d", accuracy);
	sprintf(displayGrossWPM, "Gross WPM: %d", grossWPM);
	sprintf(displayNetWPM, "Net WPM: %d", netWPM);
	printf("%s\n%s", displayAccuracy, displayGrossWPM);
	if (logInStatus)
	{
		user[loggedInProfileNo - 1].gross_wpm = grossWPM;
		user[loggedInProfileNo - 1].accuracy = accuracy;
		user[loggedInProfileNo - 1].net_wpm = netWPM;
		char userfile[40];
		sprintf(userfile, "user//user_%d.txt", loggedInProfileNo - 1);
		FILE *fp_local = fopen(userfile, "w");
		fprintf(fp_local, "%s\n%d\n%d\n%d", user[loggedInProfileNo - 1].username, accuracy, user[loggedInProfileNo - 1].gross_wpm, user[loggedInProfileNo - 1].net_wpm);
		if (netWPM > user[loggedInProfileNo - 1].best_net_wpm)
		{
			fprintf(fp_local, "\n%d", netWPM);
			user[loggedInProfileNo - 1].best_net_wpm = netWPM;
		}
		fclose(fp_local);
	}
}

void createMenu()
{
	char menuButtonName[6][30] = {
		"assets\\image\\menu_0.bmp",
		"assets\\image\\menu_1.bmp",
		"assets\\image\\menu_2.bmp",
		"assets\\image\\menu_3.bmp",
		"assets\\image\\menu_4.bmp",
		"assets\\image\\menu_5.bmp"};
	char playMenuButtonName[6][30] = {
		"assets\\image\\playmenu_0.bmp",
		"assets\\image\\playmenu_1.bmp",
		"assets\\image\\playmenu_2.bmp",
		"assets\\image\\playmenu_3.bmp",
		"assets\\image\\playmenu_4.bmp",
		"assets\\image\\playmenu_5.bmp"};
	char durationScr[7][30] = {
		"assets\\image\\duration_0.bmp",
		"assets\\image\\duration_1.bmp",
		"assets\\image\\duration_2.bmp",
		"assets\\image\\duration_3.bmp",
		"assets\\image\\duration_4.bmp",
		"assets\\image\\duration_5.bmp",
		"assets\\image\\duration_6.bmp"};
	char durationButtons[7][40] = {
		"assets\\image\\duration_buttons_0.bmp",
		"assets\\image\\duration_buttons_1.bmp",
		"assets\\image\\duration_buttons_2.bmp",
		"assets\\image\\duration_buttons_3.bmp",
		"assets\\image\\duration_buttons_4.bmp"};
	char resultscr[2][60] = {
		"assets\\image\\resultscr\\resultScr_0.bmp",
		"assets\\image\\resultscr\\resultScr_1.bmp"};
	char profilescr[4][60] = {
		"assets\\image\\profile\\profilemenu_0.bmp",
		"assets\\image\\profile\\profilemenu_1.bmp",
		"assets\\image\\profile\\profilemenu_2.bmp",
		"assets\\image\\profile\\profilemenu_3.bmp"};
	char createProfileScr_noCreateButton[3][70] = {
		"assets\\image\\profile\\createprofilemenu_0.bmp",
		"assets\\image\\profile\\createprofilemenu_1.bmp",
		"assets\\image\\profile\\createprofilemenu_0.bmp"};
	char createProfileScr_CreateButton[3][70] = {
		"assets\\image\\profile\\createButton_0.bmp",
		"assets\\image\\profile\\createButton_0.bmp",
		"assets\\image\\profile\\createButton_1.bmp"};
	char max_user_reached_Scr[2][70] = {
		"assets\\image\\profile\\max_user_reached.bmp",
		"assets\\image\\profile\\max_user_reached_1.bmp"};
	char leaderboardScr[2][80] = {
		"assets\\image\\Leaderboard\\leaderboard.bmp",
		"assets\\image\\Leaderboard\\leaderboard_1.bmp"};
	char loadProfileScr[4][80] = {
		"assets\\image\\profile\\loadProfile.bmp",
		"assets\\image\\profile\\loadProfile_1.bmp",
		"assets\\image\\profile\\loadProfile_2.bmp",
		"assets\\image\\profile\\loadProfile_3.bmp"};
	char optionScr[2][80] = {
		"assets\\image\\options\\options.bmp",
		"assets\\image\\options\\options_1.bmp"};
	menuButton = checkRect();
	if (menu)
	{
		iShowBMP2(0, 0, menuButtonName[menuButton], 0);
		iShowBMP2(700, 300, "assets\\image\\logo.bmp", 0);
	}
	else if (playmenu)
	{
		iShowBMP2(0, 0, playMenuButtonName[menuButton], 0);
	}
	else if (durationSelection)
	{
		iShowBMP2(0, 0, durationScr[durationStatus], 0);
		iShowBMP2(0, 0, durationButtons[menuButton], 0);
	}
	else if (result)
	{
		iShowBMP2(0, 0, resultscr[menuButton], 0);
		iText(100, 400, displayGrossWPM, GLUT_BITMAP_TIMES_ROMAN_24);
		iText(100, 350, displayNetWPM, GLUT_BITMAP_TIMES_ROMAN_24);
		iText(100, 300, displayAccuracy, GLUT_BITMAP_TIMES_ROMAN_24);
	}
	else if (profilemenu)
	{
		iShowBMP2(0, 0, profilescr[menuButton], 0);
	}
	else if (createProfileMenu)
	{
		if (userCount < 8)
		{
			iShowBMP2(0, 0, createProfileScr_CreateButton[menuButton], 0);
			iShowBMP2(0, 0, createProfileScr_noCreateButton[menuButton], 0);
		}
		else
		{
			iShowBMP2(0, 0, max_user_reached_Scr[menuButton], 0);
		}
	}
	else if (leaderboard)
	{
		iShowBMP2(0, 0, leaderboardScr[menuButton], 0);
		showLeaderboard();
	}
	else if (loadProfileMenu)
	{
		iShowBMP2(0, 0, loadProfileScr[menuButton], 0);
		loadProfileList();
	}
	else if (optionsMenu)
	{
		iShowBMP2(0, 0, optionScr[sound], 0);
	}
}

void setMouse()
{
	mousePos.x = cursorPos.x - rect.left;
	mousePos.y = cursorPos.y - rect.top;
}

void updateTime()
{
	if (timerStatus)
	{
		timerClock.centisecond++;
		screenClock.centisecond = timerClock.centisecond % 100;
		screenClock.second = (timerClock.centisecond / 100) % 60;
		screenClock.minute = (timerClock.centisecond / 6000) % 60;
		screenClock.hour = (timerClock.centisecond / 360000) % 100;
		displayClock[0] = '0' + (screenClock.hour / 10);
		displayClock[1] = '0' + (screenClock.hour % 10);
		displayClock[2] = ':';
		displayClock[3] = '0' + (screenClock.minute / 10);
		displayClock[4] = '0' + (screenClock.minute % 10);
		displayClock[5] = ':';
		displayClock[6] = '0' + (screenClock.second / 10);
		displayClock[7] = '0' + (screenClock.second % 10);
		displayClock[8] = '.';
		displayClock[9] = '0' + (screenClock.centisecond / 10);
		displayClock[10] = '0' + (screenClock.centisecond % 10);
	}
}

void updateAccurateTime()
{
	if (timerStatus)
	{
		accurateClock.second++;
	}
}

void generateWord(char myword[])
{
	int i, wordLength = rand() % 11 + 1;
	char generatedWord[20];
	if (randomly_generated_words)
	{
		for (i = 0; i < wordLength; i++)
		{
			generatedWord[i] = 'a' + rand() % 26;
		}
		generatedWord[i] = '\0';
		strcpy(myword, generatedWord);
	}
	else
	{
		strcpy(myword, meaningfulWord[rand() % NUM_OF_WORDS]);
	}
}

void formSentence(char word1[], char word2[])
{
	int i, j;
	for (i = 0; word1[i] != '\0'; i++)
	{
		sentence[i] = word1[i];
	}
	sentence[i] = (char)32;
	for (j = 0; word2[j] != '\0'; j++)
	{
		sentence[i + j + 1] = word2[j];
	}
	sentence[i + j + 1] = '\0';
}

void randomizeSentence()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	for (int i = 0; i < 10; i++)
	{
		generateWord(word[i]);
	}
	strcpy(sentence, word[0]);
	strcat(sentence, " ");
	for (int i = 0; i < 9; i++)
	{
		strcat(sentence, word[i + 1]);
		strcat(sentence, " ");
	}
}

void iDraw()
{
	iClear();
	if (SOLIDBG)
	{
		iSetColor(BGCOLOR_RED, BGCOLOR_GREEN, BGCOLOR_BLUE);
		iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	} // This rectangle defines the background of the UI
	else
	{
		iShowBMP(0, 0, bgdir); // Background Image
	}
	if (GRIDLINES)
	{
		iSetColor(100, 100, 0);
		for (int i = 0; i <= SCREEN_WIDTH; i += 20)
		{
			iLine(i, 0, i, SCREEN_HEIGHT);
		}
		for (int i = 0; i <= SCREEN_HEIGHT; i += 20)
		{
			iLine(0, i, SCREEN_WIDTH, i);
		}
	} // Using Gridlines for guides
	if (menu)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
	}
	else if (playmenu)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
	}
	else if (optionsMenu)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
	}
	else if (durationSelection)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
	}
	else if (typing)
	{
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_GREEN, TEXTCOLOR_BLUE);
		iText(line_x_pos, 680, "Start typing the sentence below");
		iText(line_x_pos, 560, sentence, MYFONT);
		displayInputText();
		iText(900, 680, durationText, CLOCKFONT);
		iText(900, 660, "WPM: ", CLOCKFONT);
		iText(960, 660, displayWPM, CLOCKFONT);
		iText(900, 640, "Errors: ", CLOCKFONT);
		iText(960, 640, errorDisplay, CLOCKFONT);
	}
	else if (result)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
		// iText(500, 600, numToStr(grossWPM), GLUT_BITMAP_TIMES_ROMAN_24);
	}
	else if (profilemenu)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
	}
	else if (createProfileMenu)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
		iText(200, 415, username_display, GLUT_BITMAP_TIMES_ROMAN_24);
	}
	else if (leaderboard)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
	}
	else if (loadProfileMenu)
	{
		hwnd = GetForegroundWindow();
		GetCursorPos(&cursorPos);
		GetWindowRect(hwnd, &rect);
		setMouse();
		iSetColor(TEXTCOLOR_RED, TEXTCOLOR_BLUE, TEXTCOLOR_GREEN);
		createMenu();
	}
}

/*
	function iMouseMove() is called when the user presses and drags the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouseMove(int mx, int my)
{
}

/*
	function iMouse() is called when the user presses/releases the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouse(int button, int state, int mx, int my)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		std::cout << mousePos.x << ", " << mousePos.y << "\n";
	}
	if (menu)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			if (!keyboardControl && menuButton == 1)
			{
				menu = false;
				durationSelection = true;
				menuButton = 0;
			}
			else if (!keyboardControl && menuButton == 2)
			{
				menu = false;
				profilemenu = true;
			}
			else if (!keyboardControl && menuButton == 3)
			{
				menu = false;
				optionsMenu = true;
			}
			else if (!keyboardControl && menuButton == 4)
			{
				menu = false;
				leaderboard = true;
				loadUsers();
				list_netWPM();
			}
			else if (!keyboardControl && menuButton == 5)
			{
				exit(0);
			}
		}
	}
	else if (optionsMenu)
	{
		if (!keyboardControl && menuButton == 2)
		{
			optionsMenu = false;
			menu = true;
			menuButton = 0;
		}
		else if (!keyboardControl && menuButton == 1)
		{
			if (!sound)
			{
				sound = 1;
				PlaySoundA(".\\assets\\audio\\bgmusic", NULL, SND_LOOP | SND_ASYNC);
			}
			else
			{
				sound = 0;
				PlaySound(0, 0, 0);
			}
			menuButton = 0;
		}
	}
	else if (playmenu)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			if (!keyboardControl && menuButton == 5)
			{
				playmenu = false;
				menu = true;
				menuButton = 0;
			}
			else if (!keyboardControl && menuButton == 1)
			{
				playmenu = false;
				durationSelection = true;
				menuButton = 0;
			}
		}
	}
	else if (optionsMenu)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			if (!keyboardControl && menuButton == 2)
			{
				optionsMenu = false;
				menu = true;
				menuButton = 0;
			}
		}
	}
	else if (durationSelection)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			if (!keyboardControl && menuButton == 3)
			{
				durationSelection = false;
				menu = true;
			}
			else if (!keyboardControl && menuButton == 4)
			{
				durationSelection = false;
				typing = true;
			}
			else if (!keyboardControl && menuButton == 1)
			{
				if (durationStatus < 6)
				{
					durationStatus++;
				}
				setDuration();
			}
			else if (!keyboardControl && menuButton == 2)
			{
				if (durationStatus > 0)
				{
					durationStatus--;
				}
				setDuration();
			}
		}
	}
	else if (result)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			if (!keyboardControl && menuButton == 1) // Here the values get reset.
			{
				result = false;
				menu = true;
				totalTypedEntries = 0;
				duration = 30;
				grossWPM = 0;
				netWPM = 0;
				accuracy = 0;
				errorCount = 0;
				strcpy(durationText, "");
				strcpy(displayWPM, "");
				strcpy(errorDisplay, "");
				timerStatus = false;
				words = 0;
				accurateClock.centisecond = 0;
				accurateClock.second = 0;
				randomizeSentence();
				currentCursorIndex = 0;
				for (int i = 0; i < 1024; i++)
				{
					paragraph[i] = '\0';
				}
			}
		}
	}
	else if (profilemenu)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			if (!keyboardControl && menuButton == 3)
			{
				profilemenu = false;
				menu = true;
				menuButton = 0;
			}
			else if (!keyboardControl && menuButton == 1)
			{
				profilemenu = false;
				createProfileMenu = true;
				menuButton = 0;
			}
			else if (!keyboardControl && menuButton == 2)
			{
				profilemenu = false;
				loadProfileMenu = true;
				loadUsers();
				menuButton = 0;
			}
		}
	}
	else if (createProfileMenu)
	{
		if (userCount < 8)
		{
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			{
				if (!keyboardControl && menuButton == 1)
				{
					createProfileMenu = false;
					profilemenu = true;
					menuButton = 0;
				}
				else if (!keyboardControl && menuButton == 2)
				{
					createUser(username_display);
				}
			}
		}
		else
		{
			for (int i = 0; i < 32; i++)
			{
				username_display[i] = '\0';
			}
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			{
				if (!keyboardControl && menuButton == 1)
				{
					createProfileMenu = false;
					profilemenu = true;
					menuButton = 0;
				}
			}
		}
	}
	else if (leaderboard)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			if (!keyboardControl && menuButton == 1)
			{
				leaderboard = false;
				menu = true;
				menuButton = 0;
			}
		}
	}
	else if (loadProfileMenu)
	{
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{
			std::cout << "x= " << mx << ", y= " << my << "\n";
			if (my <= 533 && my >= 133 && mx >= 80 && mx <= 520)
			{
				selectedProfileNo = (583 - my) / 50;
				std::cout << "selectedProfileNo = " << selectedProfileNo << "\n";
			}
			else if (menuButton != 2 && menuButton != 3)
			{
				selectedProfileNo = 0;
			}
			if (!keyboardControl && menuButton == 1)
			{
				loadProfileMenu = false;
				profilemenu = true;
				menuButton = 0;
			}
			else if (!keyboardControl && menuButton == 2)
			{
				if (selectedProfileNo >= 1)
				{
					loggedInProfileNo = selectedProfileNo;
					logInStatus = true;
				}
			}
			else if (!keyboardControl && menuButton == 3)
			{
				if (selectedProfileNo >= 1)
				{
					deleteProfile(selectedProfileNo);
				}
			}
		}
	}
}

void iKeyboard(unsigned char key)
{
	if (menu)
	{
		if (key == '\r')
		{
			if (menuButton == 1)
			{
				menu = false;
				durationSelection = true;
				menuButton = 0;
			}
			else if (menuButton == 2)
			{
				menu = false;
				profilemenu = true;
				menuButton = 0;
			}
			else if (menuButton == 4)
			{
				loadUsers();
				list_netWPM();
				menu = false;
				leaderboard = true;
				menuButton = 0;
			}
			else if (menuButton == 5)
			{
				exit(0);
			}
		}
	}
	else if (playmenu)
	{
		if (key == '\r')
		{
			keyboardControl = true;
			if (menuButton == 5)
			{
				playmenu = false;
				menu = true;
				menuButton = 0;
			}
			else if (menuButton == 1)
			{
				playmenu = false;
				durationSelection = true;
				menuButton = 0;
			}
		}
	}
	else if (profilemenu)
	{
		if (key == '\r')
		{
			keyboardControl = true;
			if (menuButton == 3)
			{
				playmenu = false;
				menu = true;
				menuButton = 0;
			}
		}
	}
	else if (createProfileMenu)
	{
		if (((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) && (strlen(username_display) < 32))
		{
			username_display[strlen(username_display)] = key;
		}
		else if (key == (char)8 && strlen(username_display) > 0)
		{
			username_display[strlen(username_display) - 1] = '\0';
		}
	}
	else if (leaderboard)
	{
		if (key == '\r')
		{
			keyboardControl = true;
			if (menuButton == 1)
			{
				leaderboard = false;
				menu = true;
				menuButton = 0;
			}
		}
	}
	else if (typing)
	{
		if (!timerStatus)
		{
			timerStatus = true;
			iResumeTimer(timer[1]);
			iResumeTimer(timer[2]);
		}
		if (key == ' ' || key == '\r')
		{
			if (timerStatus)
			{
				words++;
				wpm = 60 * words / accurateClock.second;
				displayWPM[2] = wpm % 10 + '0';
				displayWPM[1] = (wpm / 10) % 10 + '0';
				displayWPM[0] = (wpm / 100) % 10 + '0';
			}
			else
			{
				timerStatus = true;
				iResumeTimer(timer[1]);
				iResumeTimer(timer[2]);
			}
		}
		if (duration > 0)
		{
			if (key >= 32 && key < 127)
			{
				paragraph[currentCursorIndex] = key;
				std::cout << currentCursorIndex << " " << sentence[currentCursorIndex] << std::endl;
				if (key != sentence[currentCursorIndex])
				{
					errorCount++;
					displayError();
				}
				currentCursorIndex++;
				for (int i = 0; i < strlen(paragraph) && paragraph[i] != '\0'; i += LINE_LENGTH)
				{
					strncpy(screenText[i / LINE_LENGTH], paragraph + i, LINE_LENGTH);
				}
				totalTypedEntries++;
			}
			else if (key == '\r')
			{
				if (!timerStatus)
				{
					timerStatus = true;
				}
				randomizeSentence();
				// std::cout << sentence << "\n";
				for (int i = 0; i < 1024; i++)
				{
					paragraph[i] = '\0';
				}
				displayInputText();
				currentCursorIndex = 0;
				timerRecord[num_of_lines] = timerClock.centisecond;
				num_of_lines++;
			}
			else if (key == 8 && currentCursorIndex > 0)
			{
				currentCursorIndex--;
				paragraph[currentCursorIndex] = '\0';
				for (int i = 0; i < strlen(paragraph); i += LINE_LENGTH)
				{
					strncpy(screenText[i / LINE_LENGTH], paragraph + i, LINE_LENGTH);
				}
			}
		}
		else
		{
			typing = false;
			result = true;
		}
	}
}

/*
	function iSpecialKeyboard() is called whenver user hits special keys like-
	function keys, home, end, pg up, pg down, arraows etc. you have to use
	appropriate constants to detect them. A list is:
	GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
	GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
	GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,
	GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
	*/

void iSpecialKeyboard(unsigned char key)
{
	if (menu)
	{
		if (key == GLUT_KEY_DOWN)
		{
			keyboardControl = true;
			menuButton = (menuButton) % 5 + 1;
		}
		else if (key == GLUT_KEY_UP)
		{
			keyboardControl = true;
			menuButton = (menuButton + 3) % 5 + 1;
		}
	}
	else if (playmenu)
	{
		if (key == GLUT_KEY_DOWN)
		{
			keyboardControl = true;
			menuButton = (menuButton) % 5 + 1;
		}
		else if (key == GLUT_KEY_UP)
		{
			keyboardControl = true;
			menuButton = (menuButton + 3) % 5 + 1;
		}
	}
	else if (typing)
	{
		if (key == GLUT_KEY_END)
		{
			exit(0);
		}
	}
	else if (profilemenu)
	{
		if (key == GLUT_KEY_DOWN)
		{
			keyboardControl = true;
			menuButton = (menuButton) % 3 + 1;
		}
		else if (key == GLUT_KEY_UP)
		{
			keyboardControl = true;
			menuButton = (menuButton + 1) % 3 + 1;
		}
	}
}

void loadWords()
{
	FILE *fptr = fopen(".\\assets\\google-10000-english-no-swears.txt", "r");
	if (fptr != NULL)
	{
		for (int i = 0; i < NUM_OF_WORDS; i++)
		{
			fgets(meaningfulWord[i], 20, fptr);
			meaningfulWord[i][strlen(meaningfulWord[i]) - 1] = '\0';
		}
	}
	fclose(fptr);
}

int main(int argc, char *argv[])
{
	hwnd = GetForegroundWindow();
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	loadWords(); // Loads the words into the array
	if (sound)
	{
		srand(time(NULL));
		PlaySound(".\\assets\\audio\\bgmusic", NULL, SND_LOOP | SND_ASYNC);
	}
	loadUsers();
	// for (int i = 0; i < userCount; i++)
	// {
	// 	std::cout << user[i].username << "\n"
	// 			  << user[i].gross_wpm << "\n";
	// }
	timerClock.second = 0;
	timerClock.minute = 0;
	timerClock.hour = 0;
	accurateClock.second = 0;
	randomizeSentence();
	timer[0] = iSetTimer(10, updateTime);
	timer[1] = iSetTimer(1000, updateAccurateTime);
	timer[2] = iSetTimer(1000, handleDuration);
	iPauseTimer(timer[0]);
	iPauseTimer(timer[1]);
	iPauseTimer(timer[2]);
	iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Typing Master");
	return 0;
}

void createUser(char *username)
{
	FILE *fp = fopen(countfilename, "r+");
	fscanf(fp, "%d", &userCount);
	userCount++;
	rewind(fp);
	fprintf(fp, "%d", userCount);
	fclose(fp);
	char userFilename[40];
	sprintf(userFilename, "user\\user_%d.txt", userCount - 1);
	fp = fopen(userFilename, "w");
	fprintf(fp, "%s\n%d\n%d\n%d\n%d", username, 0, 0, 0, 0);
	fclose(fp);
}

void sort_users()
{
	int temp;
	for (int i = 0; i < userCount-1; i++)
	{
		for (int j = i + 1; j < userCount; j++)
		{
			if (user[user_sortedList[i]].best_net_wpm < user[user_sortedList[j]].best_net_wpm)
			{
				temp = user_sortedList[i];
				user_sortedList[i] = user_sortedList[j];
				user_sortedList[j] = temp;
			}
		}
	}
	for(int i = 0; i < userCount; i++)
	{
		std::cout << user[user_sortedList[i]].best_net_wpm << "  ";
	}
	std::cout << "\n";
}

char displayNetWPM_leaderboard[8][10];
void list_netWPM()
{
	sort_users();
	for (int i = 0; i < userCount; i++)
	{
		sprintf(displayNetWPM_leaderboard[i], "%d", user[user_sortedList[i]].best_net_wpm);
	}
}

void showLeaderboard()
{
	iText(550, 550, "Net WPM", GLUT_BITMAP_TIMES_ROMAN_24);
	for (int i = 0; i < userCount; i++)
	{
		iText(100, 500 - 50 * i, user[user_sortedList[i]].username, GLUT_BITMAP_TIMES_ROMAN_24);
		iText(550, 500 - 50 * i, displayNetWPM_leaderboard[i], GLUT_BITMAP_TIMES_ROMAN_24);
	}
}

void loadProfileList()
{
	for (int i = 0; i < userCount; i++)
	{
		char slNo[3];
		itoa(i + 1, slNo, 10);
		if (i + 1 == selectedProfileNo)
		{
			iSetColor(255, 237, 0);
		}
		else
		{
			iSetColor(255, 255, 255);
		}
		iText(100, 500 - 50 * i, slNo, GLUT_BITMAP_TIMES_ROMAN_24);
		iText(150, 500 - 50 * i, user[i].username, GLUT_BITMAP_TIMES_ROMAN_24);
	}
}

void filecpy(char src[], char dest[])
{
	FILE *srcFile = fopen(src, "r");
	FILE *destFile = fopen(dest, "w");
	char buffer;
	while (buffer != EOF)
	{
		buffer = fgetc(srcFile);
		if (buffer != EOF)
		{
			fprintf(destFile, "%c", buffer);
		}
		else
		{
			break;
		}
	}
	fclose(srcFile);
	fclose(destFile);
}

void deleteProfile(int selectedProfile)
{
	char profile[50], oldFileName[50], newFileName[50];
	sprintf(profile, "user\\user_%d.txt", selectedProfile - 1);
	remove(profile);
	int i = selectedProfile;
	while (i < userCount)
	{
		sprintf(oldFileName, "user\\user_%d.txt", i);
		sprintf(newFileName, "user\\user_%d.txt", i - 1);
		rename(oldFileName, newFileName);
		++i;
	}
	remove(oldFileName);
	userCount--;
	FILE *usercount_fp = fopen("user\\usercount.txt", "w");
	fprintf(usercount_fp, "%d", userCount);
	fclose(usercount_fp);
	loadUsers();
}