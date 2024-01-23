//***********************************
//** We programmed this in Visual Studio.
//** We have tested this code in Visual Studio, Visual Studio Code and DevC++.
//** It didn't work in DevC++ due to compiler not having latest C++ features which it has in Visual Studio.
//** It might or might not work in Visual Studio Code depending on your build version.
//***********************************

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
using namespace std;

// The screen height should be equal to the launch size of the console (CMD), which differs in every computer.
// Go to your cmd settings to see your launch size and set it accordingly. Otherwise the whole game would be jaggy in display.
// Please check the 'READ ME!' file in the project to better understand the process.

int ScreenHeight; // Screen height of the CMD (Rows)
int ScreenWidth;  // Screen width of the CMD (Columns)

wstring tetromino[7];
string name;
int ID;
int FieldWidth = 12;
int FieldHeight = 18;
int Score = 0;
unsigned char *pField = nullptr;
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); // To add colors to make it more "Fancier"

void InitializeConsoleSize()
{
	// Get handles to the console input and output
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get the console screen buffer info
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hOut, &csbi);

	// Use the current console size to set the initial screen dimensions
	ScreenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	ScreenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0:
		return py * 4 + px; // 0 degree
	case 1:
		return 12 + py - (px * 4); // 90 degree
	case 2:
		return 15 - (py * 4) - px; // 180 degree
	case 3:
		return 3 - py + (px * 4); // 270 degree
	}
	return 0;
}
bool ValidMove(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * FieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosX + px < FieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < FieldHeight)
				{
					// In Bounds so do collision check
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)
					{
						return false;
					} // fail on first hit
				}
			}
		}
	}

	return true;
}
void Saving(string name, int ID) // File Handling and saving player data
{
	ofstream textIn("GameHistory.txt", ios::app);

	if (textIn.is_open())
	{
		textIn << "Name of Player: " << name << endl;
		textIn << "Game ID: " << ID << endl;
		textIn << "Score: " << Score << endl;

		textIn.close();
	}
}
void OutSave() // Showing stored player info
{
	ifstream textOut("GameHistory.txt");
	string line;
	if (!textOut.is_open())
	{
		cerr << "Error Opening the file.";
	}
	int count = 0; // Really ineffecient way to add a line after each player's data.
	while (getline(textOut, line))
	{
		count++;
		cout << line << endl;
		if (count == 3)
		{
			count = 0;
			cout << "------------------------------" << endl;
		}
	}
	textOut.close();
}
void PlayerInfo() // Getting player info input
{
	cout << "Enter Player's Name: ";
	getline(cin, name);
	srand(time(0));
	ID = rand() % 100 + 177000;
}
void MainMenu() // Making it fancier
{
	cout << endl;
	cout << "                                           ||================================||" << endl;
	cout << "                                           ||   T    E    T    R    I    S   ||" << endl;
	cout << "                                           ||================================||" << endl;
	cout << "                                           ||           MAIN MENU            ||" << endl;
	cout << "                                           ||================================||" << endl;
	cout << "                                           || Press '1' To Play Game.        ||" << endl;
	cout << "                                           || Press '2' To Show Game History.||" << endl;
	cout << "                                           || Press '3' To Learn How to Play.||" << endl;
	cout << "                                           || Press '4' To Exit              ||" << endl;
	cout << "                                           ||================================||" << endl;
}
void Instructions()
{
	cout << "Instructions:" << endl
		 << endl;
	cout << "==========================================================================" << endl;
	cout << "                           Welcome to Tetris!" << endl;
	cout << "==========================================================================" << endl;
	cout << "The objective is to complete horizontal lines by filling them with blocks." << endl;
	cout << "Use the following keys to control the game:" << endl;
	cout << "  - 'A' or 'a': Move the current block left." << endl;
	cout << "  - 'D' or 'd': Move the current block right." << endl;
	cout << "  - 'S' or 's': Move the current block down (or drop it faster)." << endl;
	cout << "  - 'W' or 'w': Rotate the current block." << endl;
	cout << "You can press 'Escape' to end the game whenever while playing." << endl
		 << endl;
	cout << "Try to strategically place the blocks to complete lines and score points." << endl;
	cout << "The game gets faster as you complete more pieces. Good luck!" << endl
		 << endl;
}
void Tetris() // The actual whole game
{
	// Creating the Screen Buffer
	wchar_t *screen = new wchar_t[ScreenWidth * ScreenHeight];
	for (int i = 0; i < ScreenWidth * ScreenHeight; i++)
	{
		screen[i] = L' ';
	}
	// Manipulating the console display
	HANDLE Console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(Console);
	DWORD Bytes = 0;

	// Creating tetris blocks (assets)

	tetromino[0].append(L"..X."); // I Shape
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X."); // J Shape
	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L"....");

	tetromino[2].append(L"..X."); // S Shape
	tetromino[2].append(L".XX.");
	tetromino[2].append(L".X..");
	tetromino[2].append(L"....");

	tetromino[3].append(L".X.."); // L Shape
	tetromino[3].append(L".X..");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L".X.."); // Z Shape
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"...."); // O Shape
	tetromino[5].append(L".XX.");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"....");

	tetromino[6].append(L"..X."); // T Shape
	tetromino[6].append(L".XX.");
	tetromino[6].append(L"..X.");
	tetromino[6].append(L"....");

	pField = new unsigned char[FieldWidth * FieldHeight]; // Create play field buffer
	for (int x = 0; x < FieldWidth; x++)				  // Board Boundary
	{
		for (int y = 0; y < FieldHeight; y++)
		{
			pField[y * FieldWidth + x] = (x == 0 || x == FieldWidth - 1 || y == FieldHeight - 1) ? 9 : 0;
		}
	}

	int CurrentBlock = 0;
	int CurrentRotation = 0;
	int CurrentX = FieldWidth / 2;
	int CurrentY = 0;
	int Speed = 20;
	int SpeedCount = 0;
	bool SpeedUp = false;
	bool StopRotation = true;
	int PieceCount = 0;
	vector<int> vLines;
	bool GameOver = false;

	while (!GameOver) // Main game loop, where the magic happens.
	{
		// Setting the timing
		this_thread::sleep_for(50ms); // Small Step = 1 Game Tick
		SpeedCount++;
		SpeedUp = (SpeedCount == Speed);

		// Getting user input
		if (_kbhit()) // Checking if a key is pressed
		{
			char key = _getch(); // Getting the pressed key

			// Processing the user input
			if (key == 'a' || key == 'A') // Move left
			{
				if (ValidMove(CurrentBlock, CurrentRotation, CurrentX - 1, CurrentY))
				{
					CurrentX--;
				}
			}

			if (key == 'd' || key == 'D') // Move right
			{
				if (ValidMove(CurrentBlock, CurrentRotation, CurrentX + 1, CurrentY))
				{
					CurrentX++;
				}
			}

			if (key == 's' || key == 'S') // Move down (or drop faster)
			{
				if (ValidMove(CurrentBlock, CurrentRotation, CurrentX, CurrentY + 1))
				{
					CurrentY++;
				}
			}
			if (key == 'w' || key == 'W') // Rotate
			{
				CurrentRotation += (StopRotation && ValidMove(CurrentBlock, CurrentRotation + 1, CurrentX, CurrentY)) ? 1 : 0;
				StopRotation = false;
			}
			if (key == 27) // (Escape Button) In case you wanna end the game right there.
			{
				GameOver = true;
			}
		}
		else
		{
			StopRotation = true;
		}

		// Force the piece down the playfield with more speed
		if (SpeedUp)
		{
			// Update difficulty every 50 pieces
			SpeedCount = 0;
			PieceCount++;
			if (PieceCount % 50 == 0)
			{
				if (Speed >= 10)
				{
					Speed--;
				}
			}

			// Test if piece can be moved down
			if (ValidMove(CurrentBlock, CurrentRotation, CurrentX, CurrentY + 1))
			{
				CurrentY++;
			}
			else
			{
				// It can't! Lock the piece in place for once and all
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[CurrentBlock][Rotate(px, py, CurrentRotation)] != L'.')
						{
							pField[(CurrentY + py) * FieldWidth + (CurrentX + px)] = CurrentBlock + 1;
						}
					}
				}

				// Check for completed horizontal lines
				for (int py = 0; py < 4; py++)
				{
					if (CurrentY + py < FieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < FieldWidth - 1; px++)
						{
							bLine &= (pField[(CurrentY + py) * FieldWidth + px]) != 0;
						}

						if (bLine)
						{
							// Remove Line, set to "="
							for (int px = 1; px < FieldWidth - 1; px++)
							{
								pField[(CurrentY + py) * FieldWidth + px] = 8;
							}
							vLines.push_back(CurrentY + py);
						}
					}
				}
				Score += 25;
				if (!vLines.empty())
				{
					Score += (1 << vLines.size()) * 100;
				}

				// Generate the next random block
				CurrentX = FieldWidth / 2;
				CurrentY = 0;
				CurrentRotation = 0;
				CurrentBlock = rand() % 7;

				// If piece does not fit at all, game over.
				GameOver = !ValidMove(CurrentBlock, CurrentRotation, CurrentX, CurrentY);
			}
		}
		// Drawing the field
		for (int x = 0; x < FieldWidth; x++)
		{
			for (int y = 0; y < FieldHeight; y++)
			{
				screen[(y + 2) * ScreenWidth + (x + 2)] = L" ABCDEFG=*"[pField[y * FieldWidth + x]];
			}
		}
		// Displaying the current block
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				if (tetromino[CurrentBlock][Rotate(px, py, CurrentRotation)] != L'.')
				{
					screen[(CurrentY + py + 2) * ScreenWidth + (CurrentX + px + 2)] = CurrentBlock + 65;
				}
			}
		}
		// Displaying the score, we can't use cout for now, cuz we are manipulating the console ouput ourselves.
		swprintf_s(&screen[1 * ScreenWidth + FieldWidth + 5], 16, L"SCORE: %8d", Score);

		// Animation of line completion
		if (!vLines.empty())
		{
			// Display Frame (cheekily to draw lines)
			WriteConsoleOutputCharacterW(Console, screen, ScreenWidth * ScreenHeight, {0, 0}, &Bytes);
			this_thread::sleep_for(400ms); // Delay a bit

			for (auto &v : vLines)
			{
				for (int px = 1; px < FieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						pField[py * FieldWidth + px] = pField[(py - 1) * FieldWidth + px];
					}
					pField[px] = 0;
				}
			}
			vLines.clear();
		}
		// Display all of the frame
		WriteConsoleOutputCharacterW(Console, screen, ScreenWidth * ScreenHeight, {0, 0}, &Bytes);
	}
	// Packing things up.
	CloseHandle(Console);
	cout << "   ______                        ____                 \n"
			"  / ________ _____ ___  ___     / __ \\_   _____  _____\n"
			" / / __/ __ `/ __ `__ \\/ _ \\   / / / | | / / _ \\/ ___/\n"
			"/ /_/ / /_/ / / / / / /  __/  / /_/ /| |/ /  __/ /    \n"
			"\\____/\\__,_/_/ /_/ /_/\\___/   \\____/ |___/\\___/_/     \n"; // Picked this ASCII art from www.patorjk.com (It looks cool)
	SetConsoleTextAttribute(h, 11);											 // Cyan color
	cout << endl
		 << "Player Name: " << name;
	cout << endl
		 << "Score: " << Score;
	cout << endl
		 << "Game ID: " << ID << endl;
	Saving(name, ID);
}

int main()
{

	InitializeConsoleSize();
	bool Exit = false;
	do // Main Menu loop
	{
		system("cls");
		SetConsoleTextAttribute(h, 11); // Cyan color
		MainMenu();
		char choice = _getch(); // Getting key input from user
		switch (choice)
		{
		case '1':
			system("cls");
			SetConsoleTextAttribute(h, 5); // Purple color
			PlayerInfo();
			system("cls");				   // Added this cuz when we hit escape button while playing, it shows the name screen, which is just irritating.
			SetConsoleTextAttribute(h, 4); // Red color
			Tetris();
			system("pause");
			break;
		case '2':
			SetConsoleTextAttribute(h, 14); // Yellow color
			system("cls");
			OutSave();
			system("pause");
			break;
		case '3':
			SetConsoleTextAttribute(h, 3); // Light Blue color
			system("cls");
			Instructions();
			system("pause");
			break;
		case '4':
			Exit = true;
			system("cls");
			break;
		default:
			system("cls");
			cout << endl
				 << "Invalid input. Please re-enter your choice.";
			this_thread::sleep_for(1.5s);
			break;
		}
	} while (!Exit);
	SetConsoleTextAttribute(h, 12);
	cout << " _____ _                 _           __            ______ _             _               _  \n"
			"|_   _| |               | |         / _|           | ___ | |           (_)             | | \n"
			"  | | | |__   __ _ _ __ | | _____  | |_ ___  _ __  | |_/ | | __ _ _   _ _ _ __   __ _  | | \n"
			"  | | | '_ \\ / _` | '_ \\| |/ / __| |  _/ _ \\| '__| |  __/| |/ _` | | | | | '_ \\ / _` | | | \n"
			"  | | | | | | (_| | | | |   <\\__ \\ | || (_) | |    | |   | | (_| | |_| | | | | | (_| | |_| \n"
			"  \\_/ |_| |_|\\__,_|_| |_|_|\\_|___/ |_| \\___/|_|    \\_|   |_|\\__,_|\\__, |_|_| |_|\\__, | (_) \n"
			"                                                                   __/ |         __/ |      \n"
			"                                                                  |___/         |___/       \n";
	// Again, I just copied this cool ASCII art from a site and had to align it for cout.
	system("pause");
	return 0;
}