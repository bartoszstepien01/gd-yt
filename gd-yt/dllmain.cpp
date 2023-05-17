// dllmain.cpp : Definiuje punkt wejścia dla aplikacji DLL.
#include "pch.h"
#include <fstream>

std::ofstream file;

typedef void(__fastcall* LoadCustomLevel)(gd::LevelCell*, void*);
LoadCustomLevel functionCopy;

void __fastcall LevelCell_loadCustomLevelCell(gd::LevelCell* This, void*) {
    MessageBox(NULL, L"woka", L"chuj", MB_OK);
    functionCopy(This, 0);

    cocos2d::CCLayer* layer = reinterpret_cast<cocos2d::CCLayer*>(This->getChildren()->objectAtIndex(1));

    for (int i = 0; i < layer->getChildrenCount(); i++) {
        auto bmFont = dynamic_cast<cocos2d::CCLabelBMFont*>(layer->getChildren()->objectAtIndex(i));

        if (!bmFont) continue;

        if (bmFont->getPositionX() == 52 && bmFont->getPositionY() == 33) bmFont->setString("JEFF");
        else if (bmFont->getPositionX() == 98 && bmFont->getPositionY() == 20) bmFont->setString("JEFF");
    }
}

DWORD WINAPI thread(void* hModule) {
    file.open("debug.txt");

    MH_Initialize();

    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    MH_CreateHook(reinterpret_cast<LPVOID*>(base + 0x5a020), &LevelCell_loadCustomLevelCell, reinterpret_cast<LPVOID*>(&functionCopy));

    MH_EnableHook(MH_ALL_HOOKS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, thread, hModule, 0, 0);

    return TRUE;
}
