#include <iostream>
#include <cstdlib>
#include <windows.h>

using namespace std;

void shutdown(int ver) {
    int time = 7;
    system("taskkill /im RainbowSix_BE.exe");
    system("taskkill /im RainbowSix.exe");
    cout << "R6 must be closed\nStarting in 7 seconds...";
    for (int i = 0; i < 2; i++) {
        cout << time << endl;
        Sleep(1000);
        time -= 1;
    }
    cout << "R6 starting...\n";
    if (ver == 1) {
        ShellExecuteW(NULL, L"open", L"uplay://launch/1842/0", 0, 0, SW_HIDE);
    } if (ver == 2) {
        ShellExecuteW(NULL, L"open", L"steam://rungameid/359550", 0, 0, SW_HIDE);
    }
    else {
        cout << "\nError... Please restart...";
        Sleep(5000);
        exit(5);
    }
}

int main()
{
    int ver;

    cout << "R6 Restarter | From Yulogoth with <3\n";
    cout << "Choose your version:\n1. Uplay\n2. Steam\n";
    cin >> ver;

    shutdown(ver);

    Sleep(5000);

    cout << "R6 Started! Enjoy!\nGithub: https://github.com/Yulogoth";

    Sleep(7000);
    exit(0);
}