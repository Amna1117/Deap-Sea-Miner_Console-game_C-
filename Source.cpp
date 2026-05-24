#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <ctime>
#include <conio.h>
#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib")
using namespace std;

struct Inventory {
    int gold;
    int oxygen;
};
struct Diver {
    int x, y;
    Inventory status;
};

const int MAP_SIZE = 15;
char ocean[MAP_SIZE][MAP_SIZE];
bool visible[MAP_SIZE][MAP_SIZE];
bool collected[MAP_SIZE][MAP_SIZE];

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void playLoop(const char* alias, const char* file) {
    char cmd[200];
    sprintf(cmd, "open \"%s\" type waveaudio alias %s", file, alias);
    MCIERROR err = mciSendStringA(cmd, NULL, 0, NULL);
    if (err != 0) {
        cout << ">> Sound Error (" << alias << "): " << err << endl;
        Sleep(2000);
        return;
    }
    sprintf(cmd, "play %s repeat", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
}

void playSoundOnce(const char* alias, const char* file) {
    char cmd[200];
    sprintf(cmd, "close %s", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
    sprintf(cmd, "open \"%s\" type waveaudio alias %s", file, alias);
    mciSendStringA(cmd, NULL, 0, NULL);
    sprintf(cmd, "play %s", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
}

void stopSound(const char* alias) {
    char cmd[200];
    sprintf(cmd, "close %s", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
}
void generateOcean() {
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            visible[i][j] = false;
            collected[i][j] = false;
            if (i == 0) ocean[i][j] = '~';
            else {
                int r = rand() % 10;
                if (r == 0) ocean[i][j] = '$';
                else if (r == 1) ocean[i][j] = 'X';
                else ocean[i][j] = '.';
            }
        }
    }
}

void drawScreen(Diver d) {
    setColor(11);
    cout << "=*=*= DEEP SEA MINER: OXYGEN ALERT =*=*=" << endl;

    setColor(15);
    cout << "OXYGEN~: ";
    if (d.status.oxygen < 25) setColor(12); else setColor(10);
    cout << d.status.oxygen << "%   ";

    setColor(6);
    cout << "GOLD$: " << d.status.gold << "/100" << endl;

    setColor(9);
    cout << "------------------------------------" << endl;

    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if (i == d.y && j == d.x) {
                setColor(11); cout << "D ";
            }
            else if (visible[i][j]) {
                if (ocean[i][j] == '$') { setColor(8);  cout << "$ "; }
                else if (ocean[i][j] == 'X') { setColor(12); cout << "X "; }
                else if (ocean[i][j] == '~') { setColor(3);  cout << "~ "; }
                else { setColor(8); cout << ". "; }
            }
            else {
                if (i == 0) { setColor(3); cout << "~ "; }
                else { setColor(7); cout << "# "; }
            }
        }
        cout << endl;
    }

    setColor(9);
    cout << "------------------------------------" << endl;
    setColor(15);
    cout << "[W/w] Up  [S/s] Down  [A/a] Left  [D/d] Right" << endl;
    cout << "Arrow keys can also be used for movement." << endl;
    cout << "Surface (Top Row) Refills Oxygen" << endl;
    cout << "Collect 100 Gold to Win!" << endl;
    cout << "Avoid Sharp Rocks and Sharks!" << endl;
    setColor(9);
    cout << "------------------------------------";
}

void processMove(Diver& d, char input) {
    int oldX = d.x, oldY = d.y;

    if (input == 'w' || input == 'W') d.y--;
    else if (input == 's' || input == 'S') d.y++;
    else if (input == 'a' || input == 'A') d.x--;
    else if (input == 'd' || input == 'D') d.x++;

    if (d.x < 0 || d.x >= MAP_SIZE || d.y < 0 || d.y >= MAP_SIZE) {
        d.x = oldX; d.y = oldY;
        return;
    }

    playSoundOnce("step", "step.wav");
    Sleep(500);
    d.status.oxygen -= 2;
    visible[d.y][d.x] = true;

    if (d.y == 0) {
        d.status.oxygen = 100;
        playSoundOnce("refill", "refill.wav");
        setColor(10);
        cout << "\n>> TANKS REFILLED! (Oxygen back to 100%)" << endl;
        setColor(15);
        Sleep(1200);
    }

    if (ocean[d.y][d.x] == '$') {
        if (!collected[d.y][d.x]) {
            playSoundOnce("gold", "gold.wav");

            d.status.gold += 10;
            collected[d.y][d.x] = true;
            setColor(6);
            cout << "\n>> TREASURE FOUND! +10 Gold" << endl;
            setColor(15);
            Sleep(1200);
        }
        else {
            setColor(12);
            cout << "\n>> Already Collected Here!" << endl;
            setColor(15);
            Sleep(800);
        }
    }
    else if (ocean[d.y][d.x] == 'X') {
        playSoundOnce("rock", "rock.wav");

        d.status.oxygen -= 15;
        setColor(12);
        cout << "\n>> CRUNCH! Hit sharp rocks! -15 Oxygen" << endl;
        setColor(15);
        Sleep(1200);
    }
}

int main() {
    srand((unsigned)time(0));

    PlaySound(TEXT("ocean.wav"), NULL, SND_ASYNC | SND_LOOP);

    Diver player;
    player.x = 5; player.y = 0;
    player.status.gold = 0; player.status.oxygen = 100;

    generateOcean();
    visible[player.y][player.x] = true;

    while (player.status.oxygen > 0) {
        system("cls");
        drawScreen(player);

        if (player.status.gold >= 100) {
            PlaySound(NULL, 0, 0);
            PlaySound(TEXT("victory.wav"), NULL, SND_SYNC);
            setColor(10);
            cout << "\nMISSION SUCCESS! You found enough gold!" << endl;
            system("pause");
            return 0;
        }
        unsigned char choice = _getch();

        if (choice == 0 || choice == 224) {
            unsigned char arrow = _getch();
            if (arrow == 72) choice = 'w';
            else if (arrow == 80) choice = 's';
            else if (arrow == 75) choice = 'a';
            else if (arrow == 77) choice = 'd';
            else {
                setColor(12);
                cout << "\n>> INVALID INPUT! Use W/A/S/D or Arrow Keys." << endl;
                setColor(15);
                Sleep(1200);
                continue;
            }
        }

        if (choice == '\t' || choice == '\r' || choice < 32) {
            setColor(12);
            cout << "\n>> INVALID INPUT! Use W/A/S/D or Arrow Keys." << endl;
            setColor(15);
            Sleep(1200);
            continue;
        }

        if (choice != 'w' && choice != 'W' &&
            choice != 'a' && choice != 'A' &&
            choice != 's' && choice != 'S' &&
            choice != 'd' && choice != 'D')
        {
            setColor(12);
            cout << "\n>> INVALID INPUT! Use W/A/S/D only." << endl;
            setColor(15);
            Sleep(1200);
            continue;
        }

        processMove(player, choice);
    }

    system("cls");
    drawScreen(player);
    PlaySound(NULL, 0, 0); 
    PlaySound(TEXT("gameover.wav"), NULL, SND_ASYNC);
    setColor(12);
    cout << "\nOxygen Depleted. Game Over." << endl;
    setColor(15);
    system("pause");
    return 0;
}