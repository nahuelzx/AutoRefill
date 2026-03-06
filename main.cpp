#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <limits>
#include <cctype>

struct Point {
    int x;
    int y;
};

struct SlotCoord {
    int x;
    int y;
};

int g_screenWidth  = 1920;
int g_screenHeight = 1080;
char g_hotkey      = 'F';

std::vector<SlotCoord> g_slots;

enum ConsoleColor {
    NEGRO=0, AZUL=1, VERDE=2, CIAN=3, ROJO=4,
    MAGENTA=5, MARRON=6, GRIS=7,
    GRIS_OSCURO=8, AZUL_CLARO=9, VERDE_CLARO=10,
    CIAN_CLARO=11, ROJO_CLARO=12, MAGENTA_CLARO=13,
    AMARILLO=14, BLANCO=15
};

void SetColor(WORD c)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void Clear()
{
    system("cls");
}

void Separator()
{
    SetColor(GRIS_OSCURO);
    std::cout << "────────────────────────────────────────────────────────────\n";
    SetColor(BLANCO);
}

void TypeText(const std::string& txt, int delay = 4)
{
    for (char c : txt)
    {
        std::cout << c;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

bool IsKeyDown(WORD vk)
{
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

std::string GetActiveWindowTitle()
{
    char buff[256]{};

    HWND h = GetForegroundWindow();

    if (GetWindowTextA(h, buff, sizeof(buff)))
        return buff;

    return "";
}

Point GetCursorNormalized()
{
    POINT p{};
    GetCursorPos(&p);

    Point r;
    r.x = (p.x * 65535) / g_screenWidth;
    r.y = (p.y * 65535) / g_screenHeight;

    return r;
}

void MouseMove(int x, int y)
{
    INPUT input{};
    input.type = INPUT_MOUSE;

    input.mi.dx = x;
    input.mi.dy = y;

    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

    SendInput(1, &input, sizeof(INPUT));
}

void LeftClick()
{
    INPUT inputs[2]{};

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, inputs, sizeof(INPUT));
}

void KeyPress(WORD key)
{
    INPUT inputs[2]{};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

void KeyDown(WORD key)
{
    INPUT i{};
    i.type = INPUT_KEYBOARD;
    i.ki.wVk = key;

    SendInput(1, &i, sizeof(INPUT));
}

void KeyUp(WORD key)
{
    INPUT i{};
    i.type = INPUT_KEYBOARD;
    i.ki.wVk = key;
    i.ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &i, sizeof(INPUT));
}

bool SaveConfig(const std::string& path)
{
    std::ofstream out(path);

    if (!out)
        return false;

    out << "resolution_width=" << g_screenWidth << "\n";
    out << "resolution_height=" << g_screenHeight << "\n";
    out << "hotkey=" << g_hotkey << "\n";

    out << "slot_count=" << g_slots.size() << "\n";

    for (size_t i = 0; i < g_slots.size(); i++)
    {
        out << "slot" << i << "_x=" << g_slots[i].x << "\n";
        out << "slot" << i << "_y=" << g_slots[i].y << "\n";
    }

    return true;
}

bool LoadConfig(const std::string& path)
{
    std::ifstream in(path);

    if (!in)
        return false;

    std::string line;

    g_slots.clear();

    while (std::getline(in, line))
    {
        if (line.rfind("resolution_width=",0)==0)
            g_screenWidth = std::stoi(line.substr(17));

        else if (line.rfind("resolution_height=",0)==0)
            g_screenHeight = std::stoi(line.substr(18));

        else if (line.rfind("hotkey=",0)==0)
            g_hotkey = line[7];

        else if (line.rfind("slot_count=",0)==0)
            g_slots.resize(std::stoi(line.substr(11)));

        else if (line.rfind("slot",0)==0)
        {
            size_t u = line.find('_');
            size_t e = line.find('=');

            if (u==std::string::npos || e==std::string::npos)
                continue;

            int index = std::stoi(line.substr(4, u-4));
            int val   = std::stoi(line.substr(e+1));

            std::string tag = line.substr(u+1, e-u-1);

            if (tag=="x") g_slots[index].x = val;
            if (tag=="y") g_slots[index].y = val;
        }
    }

    return !g_slots.empty();
}

void Banner()
{
    Clear();

    SetColor(VERDE_CLARO);

    std::cout <<
R"(╔══════════════════════════════════════════════╗
║                AUTO REFILL v1                   ║
║           minecraft bedrock utility             ║
╠═════════════════════════════════════════════════╣
)";

    SetColor(GRIS);

    TypeText(" booting modules...\n",8);
    TypeText(" loading config...\n",8);

    Separator();
}

void StatusPanel()
{
    Clear();

    SetColor(VERDE_CLARO);

    std::cout <<
R"(╔══════════════════════════════════════════════╗
║                     STATUS                      ║
╚═════════════════════════════════════════════════╝
)";

    SetColor(BLANCO);

    std::cout << " resolution : " << g_screenWidth << " x " << g_screenHeight << "\n";
    std::cout << " hotkey     : " << g_hotkey << "\n";
    std::cout << " slots      : " << g_slots.size() << "\n";

    Separator();

    SetColor(CIAN_CLARO);

    std::cout << " target window : Minecraft\n";
    std::cout << " press hotkey inside minecraft\n";
}

void AskCustomResolution()
{
    std::cout << "\ncustom width: ";
    std::cin >> g_screenWidth;
    std::cout << "custom height: ";
    std::cin >> g_screenHeight;
}

void CaptureSlots()
{
    Clear();

    std::cout <<
R"(SLOT CAPTURE

1) abre minecraft
2) abre inventario
3) pone mouse sobre slot
4) presiona C

)";

    int count;

    std::cout << "slots a capturar: ";
    std::cin >> count;

    g_slots.clear();
    g_slots.reserve(count);

    std::cout << "\nCambia a minecraft ahora...\n";

    std::this_thread::sleep_for(std::chrono::seconds(2));

    for (int i=0;i<count;i++)
    {
        std::cout << "slot " << i+1 << " -> presiona C\n";

        while (!IsKeyDown('C'))
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        Point p = GetCursorNormalized();

        g_slots.push_back({p.x,p.y});

        Beep(900,100);

        std::cout << "capturado ("<<p.x<<","<<p.y<<")\n";
    }
}


void Loading()
{
    Clear();

    std::cout << "initializing...\n\n";

    const int width = 40;

    for (int i=0;i<=width;i++)
    {
        std::cout << "\r[";

        for (int j=0;j<width;j++)
            std::cout << (j<i ? '#' : ' ');

        std::cout << "] " << std::setw(3) << (i*100/width) << "%";

        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    std::cout << "\n";
}

void AutoRefillLoop()
{
    StatusPanel();

    WORD hotkeyVK = std::toupper(g_hotkey);

    while (true)
    {
        std::string title = GetActiveWindowTitle();

        if (title.find("Minecraft") == std::string::npos)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!IsKeyDown(hotkeyVK))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        KeyPress('E');
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        KeyDown(VK_LSHIFT);

        for (const auto& s : g_slots)
        {
            MouseMove(s.x,s.y);
            LeftClick();
            std::this_thread::sleep_for(std::chrono::milliseconds(6));
        }

        KeyUp(VK_LSHIFT);
        KeyPress('E');
    }
}

int main()
{
    CONSOLE_CURSOR_INFO ci{};
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);

    const std::string config = "config.txt";

    Banner();

    bool loaded = LoadConfig(config);

    if (!loaded)
    {
        std::cout << "config no encontrado\n\n";

        std::cout << "1) 1920x1080\n";
        std::cout << "2) 1600x900\n";
        std::cout << "3) 1280x720\n";
        std::cout << "4) custom\n";

        int opt;
        std::cin >> opt;

        if (opt==1){ g_screenWidth=1920; g_screenHeight=1080; }
        if (opt==2){ g_screenWidth=1600; g_screenHeight=900; }
        if (opt==3){ g_screenWidth=1280; g_screenHeight=720; }
        if (opt==4){ AskCustomResolution(); }

        std::cout << "\nhotkey: ";
        std::cin >> g_hotkey;

        CaptureSlots();

        SaveConfig(config);
    }
    else
    {
        std::cout << "config cargado\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(900));
    }

    Loading();
    AutoRefillLoop();

    return 0;
}
