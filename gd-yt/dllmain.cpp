// dllmain.cpp : Definiuje punkt wejścia dla aplikacji DLL.
#include "pch.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>

using namespace cocos2d;

using json = nlohmann::json;

std::ofstream debugFile;
std::fstream cacheFile;

json cache;

typedef void(__fastcall* UpdateBGColor)(gd::LevelCell*, void*, unsigned int);
UpdateBGColor functionCopy;

typedef bool(__fastcall* CustomSongLayerInit)(gd::CustomSongLayer*, void*, gd::LevelSettingsObject*);
CustomSongLayerInit functionCopy2;

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

    if (cache.contains(youtubeID)) {
        title = cache[youtubeID]["name"];
    }
    else {
        // TODO: Sanitize input
        std::string command = "gd-yt\\yt-dlp -J " + youtubeID + " --no-warnings";
        std::string output;
        // TODO: Move to another thread and handle errors
        int result = runCmd(command.c_str(), output);

        json outputJSON = json::parse(output);
        title = outputJSON["title"];

        cache[youtubeID] = {
            { "id", youtubeID },
            { "name", title },
            { "artistID", outputJSON["uploader_id"] },
            { "artist", outputJSON["uploader"] },
            { "url", outputJSON["webpage_url"]}
        };

        cacheFile.open("gd-yt/cache.json", std::ios::out);
        cacheFile << std::setw(4) << cache;
        cacheFile.close();
    }

    cocos2d::CCLayer* layer = reinterpret_cast<cocos2d::CCLayer*>(This->getChildren()->objectAtIndex(1));

    for (int i = 0; i < layer->getChildrenCount(); i++) {
        auto bmFont = dynamic_cast<cocos2d::CCLabelBMFont*>(layer->getChildren()->objectAtIndex(i));

        if (!bmFont) continue;

        std::pair<int, int> position = { bmFont->getPositionX(), bmFont->getPositionY() };
        if (songNamePositions.count(position) > 0) bmFont->setString(title.c_str());
    }
}

class SampleClass {
public:
    static gd::GJGameLevel* level;
    static gd::CCTextInputNode* input;
    static gd::CCMenuItemSpriteExtra* ytSearchButton;
    static gd::CCMenuItemSpriteExtra* ngSearchButton;
    static gd::CustomSongWidget* songWidget;

    void callback(gd::CCMenuItemToggler* sender) {
        //gd::FLAlertLayer::create(nullptr, "Attempt Count", "OK", nullptr, !sender->isOn() ? "on" : "off")->show();
        input->setString("");

        if (sender->isOn()) {
            input->setAllowedChars("0123456789");
            input->setMaxLabelLength(999);
            ytSearchButton->setVisible(false);
            ngSearchButton->setVisible(true);
        }
        else {
            input->setAllowedChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");
            input->setMaxLabelLength(11);
            ytSearchButton->setVisible(true);
            ngSearchButton->setVisible(false);
        }
    }   

    void buttonCallback(gd::CCMenuItemSpriteExtra* sender) {
        //gd::FLAlertLayer::create(nullptr, "Test", "OK", nullptr, "Hello, world!")->show();
        std::string youtubeID = input->getString();
        if (!cache.contains(youtubeID)) {
            // TODO: Sanitize input
            std::string command = "gd-yt\\yt-dlp -J " + youtubeID + " --no-warnings";
            std::string output;
            // TODO: Move to another thread and handle errors
            int result = runCmd(command.c_str(), output);

            json outputJSON = json::parse(output);

            cache[youtubeID] = {
                { "id", youtubeID },
                { "name", outputJSON["title"] },
                { "artistID", outputJSON["uploader_id"] },
                { "artist", outputJSON["uploader"] },
                { "url", outputJSON["webpage_url"]}
            };

            cacheFile.open("gd-yt/cache.json", std::ios::out);
            cacheFile << std::setw(4) << cache;
            cacheFile.close();
        }

        // memory leak ?
        gd::SongInfoObject* info = new gd::SongInfoObject;
        info->m_nSongID = -1;
        info->m_sSongName = cache[youtubeID]["name"];
        info->m_sArtistName = cache[youtubeID]["artist"];
        info->m_sSongLink = cache[youtubeID]["url"];
        info->m_sYouTubeChannelURL = youtubeID;
        info->m_nArtistID = -1;
        info->m_fFileSize = 21.37;
        info->m_bIsVerified = true;
        info->m_nSongPriority = -1;

        songWidget->updateSongObject(info);

        cocos2d::CCObject* obj;
        cocos2d::CCMenu* menu = NULL;

        using namespace cocos2d;
        CCARRAY_FOREACH(songWidget->getChildren(), obj) {
            debugFile << reinterpret_cast<CCNode*>(obj)->getChildrenCount() << '\n';
            if (reinterpret_cast<CCNode*>(obj)->getChildrenCount() != 6) continue;

            menu = reinterpret_cast<CCMenu*>(obj);
            break;
        }

        CCARRAY_FOREACH(menu->getChildren(), obj) {
            reinterpret_cast<CCNode*>(obj)->setVisible(false);
        }

        CCSprite* sprite = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        gd::CCMenuItemSpriteExtra* downloadButton = gd::CCMenuItemSpriteExtra::create(sprite, songWidget, menu_selector(SampleClass::downloadCallback));
        downloadButton->setPosition(-157, -180);

        CCSprite* sprite2 = CCSprite::createWithSpriteFrameName("GJ_selectSongBtn_001.png");
        gd::CCMenuItemSpriteExtra* useButton = gd::CCMenuItemSpriteExtra::create(sprite2, songWidget, menu_selector(SampleClass::useCallback));
        useButton->setPosition(-157, -180);

        if (std::filesystem::exists("gd-yt/downloads/" + youtubeID + ".mp3")) downloadButton->setVisible(false);
        else useButton->setVisible(false);

        menu->addChild(downloadButton);
        menu->addChild(useButton);
    }

    void downloadCallback(gd::CCMenuItemSpriteExtra* button) {
        //gd::FLAlertLayer::create(nullptr, "Test", "OK", nullptr, )->show();
        std::string command = "gd-yt\\yt-dlp -x --audio-format mp3 " + songWidget->m_songInfo->m_sSongLink + " -o \"gd-yt/downloads/%(id)s.%(ext)s\"";
        std::string output;
        int ret = runCmd(command.c_str(), output);
    }

    void useCallback(gd::CCMenuItemSpriteExtra* button) {
        level->m_sLevelDesc = macaron::Base64::Encode("$" + songWidget->m_songInfo->m_sYouTubeChannelURL + "$");
    }
};

gd::GJGameLevel* SampleClass::level;
gd::CCTextInputNode* SampleClass::input;
gd::CCMenuItemSpriteExtra* SampleClass::ytSearchButton;
gd::CCMenuItemSpriteExtra* SampleClass::ngSearchButton;
gd::CustomSongWidget* SampleClass::songWidget;

bool __fastcall CustomSongLayer_Init(gd::CustomSongLayer* This, void*, gd::LevelSettingsObject* obj) {
    using namespace cocos2d;
    if (!functionCopy2(This, 0, obj)) return false;

    SampleClass::level = This->m_levelSettings->m_pLevel;

    SampleClass::songWidget = This->m_songWidget;
    SampleClass::input = This->m_songIDInput;

    gd::CCMenuItemSpriteExtra* button = gd::CCMenuItemSpriteExtra::create(gd::ButtonSprite::create("Search", 0, false, "goldFont.fnt", "GJ_button_04.png", 35, 0.529f), This, menu_selector(SampleClass::buttonCallback));
    button->setPosition(323, -155);
    button->setVisible(false);

    SampleClass::ytSearchButton = button;

    gd::CCMenuItemToggler* toggle = gd::CCMenuItemToggler::createWithStandardSprites(This, menu_selector(SampleClass::callback));
    toggle->setPosition(40, -155);

    cocos2d::CCLayer* layer = reinterpret_cast<cocos2d::CCLayer*>(This->getChildren()->objectAtIndex(0));

    for (int i = 0; i < layer->getChildrenCount(); i++) {
        CCNode* node = reinterpret_cast<CCNode*>(layer->getChildren()->objectAtIndex(i));

        if (node->getZOrder() != 10) continue;

        CCObject* obj;
        CCARRAY_FOREACH(node->getChildren(), obj) {
            CCLayer* layer = reinterpret_cast<CCLayer*>(obj);

            if (layer->getPositionX() == 323 && layer->getPositionY() == -155) {
                SampleClass::ngSearchButton = reinterpret_cast<gd::CCMenuItemSpriteExtra*>(layer);
                break;
            }
        };

        node->addChild(toggle);
        node->addChild(button);

        break;
    }

    //This->m_songIDInput->setAllowedChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");

    return true;
}

DWORD WINAPI thread(void* hModule) {
    debugFile.open("debug.txt");
    cacheFile.open("gd-yt/cache.json", std::ios::in);

    cache = json::parse(cacheFile);
    cacheFile.close();

    MH_Initialize();

    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    MH_CreateHook(reinterpret_cast<LPVOID*>(base + 0x5c6b0), &LevelCell_updateBGColor, reinterpret_cast<LPVOID*>(&functionCopy));
    MH_CreateHook(reinterpret_cast<LPVOID*>(base + 0x65c10), &CustomSongLayer_Init, reinterpret_cast<LPVOID*>(&functionCopy2));

    MH_EnableHook(MH_ALL_HOOKS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, thread, hModule, 0, 0);

    return TRUE;
}
