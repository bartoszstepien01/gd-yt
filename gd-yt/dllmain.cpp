// dllmain.cpp : Definiuje punkt wejścia dla aplikacji DLL.
#include "pch.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

std::ofstream file;

typedef void(__fastcall* UpdateBGColor)(gd::LevelCell*, void*, unsigned int index);
UpdateBGColor functionCopy;

std::set<std::pair<int, int>> songNamePositions {
    {52, 33}, {98, 20}, {86, 20}, {74, 20}, {88, 20}, {103, 20},
};


// TODO: Credit the author
int runCmd(const char* cmd, std::string& outOutput) {
    wchar_t wtext[128];
    size_t outSize;
    mbstowcs_s(&outSize, wtext, 128, cmd, strlen(cmd));//includes null
    LPWSTR ptr = wtext;

    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    HANDLE g_hChildStd_ERR_Rd = NULL;
    HANDLE g_hChildStd_ERR_Wr = NULL;

    SECURITY_ATTRIBUTES sa;
    // Set the bInheritHandle flag so pipe handles are inherited.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &sa, 0)) { return 1; } // Create a pipe for the child process's STDERR.
    if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0)) { return 1; } // Ensure the read handle to the pipe for STDERR is not inherited.
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0)) { return 1; } // Create a pipe for the child process's STDOUT.
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) { return 1; } // Ensure the read handle to the pipe for STDOUT is not inherited

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    bool bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure.
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDERR and STDOUT handles for redirection.
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_ERR_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process.
    bSuccess = CreateProcess(
        NULL,             // program name
        ptr,       // command line
        NULL,             // process security attributes
        NULL,             // primary thread security attributes
        TRUE,             // handles are inherited
        CREATE_NO_WINDOW, // creation flags (this is what hides the window)
        NULL,             // use parent's environment
        NULL,             // use parent's current directory
        &siStartInfo,     // STARTUPINFO pointer
        &piProcInfo       // receives PROCESS_INFORMATION
    );

    CloseHandle(g_hChildStd_ERR_Wr);
    CloseHandle(g_hChildStd_OUT_Wr);

    // read output
#define BUFSIZE 128
    DWORD dwRead;
    CHAR chBuf[BUFSIZE];
    bool bSuccess2 = FALSE;
    for (;;) { // read stdout
        bSuccess2 = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess2 || dwRead == 0) break;
        std::string s(chBuf, dwRead);
        outOutput += s;
    }
    dwRead = 0;
    for (;;) { // read stderr
        bSuccess2 = ReadFile(g_hChildStd_ERR_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess2|| dwRead == 0) break;
        std::string s(chBuf, dwRead);
        outOutput += s;
    }

    // The remaining open handles are cleaned up when this process terminates.
    // To avoid resource leaks in a larger application,
    // close handles explicitly.
    return 0;
}

void __fastcall LevelCell_updateBGColor(gd::LevelCell* This, void*, unsigned int index) {
    functionCopy(This, 0, index);

    std::string description;
    macaron::Base64::Decode(This->m_pLevel->m_sLevelDesc, description);

    size_t begin = description.find('$');
    size_t end = description.find('$', begin + 1);

    if (begin == std::string::npos || end == std::string::npos) return;
    std::string youtubeID = description.substr(begin + 1, end - begin - 1);
    std::string title;

    // TODO: Sanitize input
    std::string command = "gd-yt\\yt-dlp -e " + youtubeID + " --no-warnings";
    // TODO: Move to another thread
    int result = runCmd(command.c_str(), title);

    cocos2d::CCLayer* layer = reinterpret_cast<cocos2d::CCLayer*>(This->getChildren()->objectAtIndex(1));

    for (int i = 0; i < layer->getChildrenCount(); i++) {
        auto bmFont = dynamic_cast<cocos2d::CCLabelBMFont*>(layer->getChildren()->objectAtIndex(i));

        if (!bmFont) continue;

        std::pair<int, int> position = { bmFont->getPositionX(), bmFont->getPositionY() };
        if (songNamePositions.count(position) > 0) bmFont->setString(title.c_str());
    }
}

DWORD WINAPI thread(void* hModule) {
    file.open("debug.txt");

    MH_Initialize();

    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    MH_CreateHook(reinterpret_cast<LPVOID*>(base + 0x5c6b0), &LevelCell_updateBGColor, reinterpret_cast<LPVOID*>(&functionCopy));

    MH_EnableHook(MH_ALL_HOOKS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, thread, hModule, 0, 0);

    return TRUE;
}
