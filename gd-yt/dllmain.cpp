// dllmain.cpp : Definiuje punkt wejścia dla aplikacji DLL.
#include "pch.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <string>
#include "YouTube.h"
#include "Utils.h"
#include "Callbacks.h"
#include "MappedHook.h"

std::ofstream debugFile;

std::set<std::pair<int, int>> songNamePositions {
    {52, 33}, {98, 20}, {86, 20}, {74, 20}, {88, 20}, {103, 20},
};

YouTube* youTube = NULL;

void __fastcall LevelCell_updateBGColor(gd::LevelCell* This, void*, unsigned int index) {
    getOriginal(LevelCell_updateBGColor)(This, 0, index);

    if (This->m_pLevel->m_nSongID != 42069) return;

    std::string youtubeID = getYoutubeIDFromDescription(This->m_pLevel->m_sLevelDesc);
    if (youtubeID == "") return;
   
    YouTube::VideoData data = youTube->getData(youtubeID);
    if (data.id == "") return;

    cocos2d::CCLayer* layer = reinterpret_cast<cocos2d::CCLayer*>(This->getChildren()->objectAtIndex(1));

    for (int i = 0; i < layer->getChildrenCount(); i++) {
        auto bmFont = dynamic_cast<cocos2d::CCLabelBMFont*>(layer->getChildren()->objectAtIndex(i));

        if (!bmFont) continue;

        std::pair<int, int> position = { bmFont->getPositionX(), bmFont->getPositionY() };
        if (songNamePositions.count(position) > 0) bmFont->setString(data.title.c_str());
    }
}

bool __fastcall CustomSongLayer_Init(gd::CustomSongLayer* This, void*, gd::LevelSettingsObject* obj) {
    using namespace cocos2d;
    if (!getOriginal(CustomSongLayer_Init)(This, 0, obj)) return false;

    Callbacks::level = This->m_levelSettings->m_pLevel;

    Callbacks::songWidget = This->m_songWidget;
    Callbacks::input = This->m_songIDInput;

    gd::CCMenuItemSpriteExtra* button = gd::CCMenuItemSpriteExtra::create(gd::ButtonSprite::create("Search", 0, false, "goldFont.fnt", "GJ_button_04.png", 35, 0.529f), This, menu_selector(Callbacks::buttonCallback));
    button->setPosition(323, -155);
    button->setVisible(false);

    Callbacks::ytSearchButton = button;

    gd::CCMenuItemToggler* toggle = gd::CCMenuItemToggler::createWithStandardSprites(This, menu_selector(Callbacks::toggleCallback));
    toggle->setPosition(40, -155);

    cocos2d::CCLayer* layer = reinterpret_cast<cocos2d::CCLayer*>(This->getChildren()->objectAtIndex(0));

    for (int i = 0; i < layer->getChildrenCount(); i++) {
        CCNode* node = reinterpret_cast<CCNode*>(layer->getChildren()->objectAtIndex(i));

        if (node->getZOrder() != 10) continue;

        CCObject* obj;
        CCARRAY_FOREACH(node->getChildren(), obj) {
            CCLayer* layer = reinterpret_cast<CCLayer*>(obj);

            if (layer->getPositionX() == 323 && layer->getPositionY() == -155) {
                Callbacks::ngSearchButton = reinterpret_cast<gd::CCMenuItemSpriteExtra*>(layer);
                break;
            }
        };

        node->addChild(toggle);
        node->addChild(button);

        break;
    }

    toggle->toggle(true);
    toggle->toggleWithCallback(false);

    std::string youtubeID = getYoutubeIDFromDescription(This->m_levelSettings->m_pLevel->m_sLevelDesc);
    if(youtubeID != "") toggle->toggleWithCallback(true);

    return true;
}

bool __fastcall EditLevelLayer_Init(gd::EditLevelLayer* This, void*, gd::GJGameLevel* level) {
    if (!getOriginal(EditLevelLayer_Init)(This, 0, level)) return false;

    std::string youtubeID = getYoutubeIDFromDescription(level->m_sLevelDesc);
    if (youtubeID == "" || level->m_nSongID != 42069) return true;

    try {
        std::filesystem::remove("Resources/42069.mp3");
        std::filesystem::copy("gd-yt/downloads/" + youtubeID + ".mp3", "Resources/42069.mp3");
    }
    catch (std::exception e) {
        // do nothing;
    }

    return true;
}

bool __fastcall LevelSettingsLayer_Init(gd::LevelSettingsLayer* This, void*, gd::LevelSettingsObject* obj, gd::LevelEditorLayer* lvlLayer) {
    if (!getOriginal(LevelSettingsLayer_Init)(This, 0, obj, lvlLayer)) return false;

    std::string youtubeID = getYoutubeIDFromDescription(obj->m_pLevel->m_sLevelDesc);
    if (youtubeID == "" || obj->m_pLevel->m_nSongID != 42069) return true;

    YouTube::VideoData data = youTube->getData(youtubeID);
    if (data.id == "") return true;

    gd::SongInfoObject* info = videoDataToSongInfoObject(data);

    cocos2d::CCLayer* layer = reinterpret_cast<cocos2d::CCLayer*>(This->getChildren()->objectAtIndex(0));

    using namespace cocos2d;
    CCObject* object;

    gd::CustomSongWidget* widget = NULL;

    CCARRAY_FOREACH(layer->getChildren(), object) {
        CCNode* node = dynamic_cast<CCNode*>(object);
        if (!node) continue;
        if (node->getChildrenCount() != 7) continue;

        widget = reinterpret_cast<gd::CustomSongWidget*>(object);
    };

    widget->updateSongObject(info);

    return true;
}

DWORD WINAPI thread(void* hModule) {
    debugFile.open("debug.txt");
    
    youTube = YouTube::getInstance();
    youTube->loadCache("gd-yt/cache.json");

    MH_Initialize();

    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    registerHook(base + 0x5c6b0, LevelCell_updateBGColor);
    registerHook(base + 0x65c10, CustomSongLayer_Init);
    registerHook(base + 0x170e50, LevelSettingsLayer_Init);
    registerHook(base + 0x6f5d0, EditLevelLayer_Init);

    MH_EnableHook(MH_ALL_HOOKS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, thread, hModule, 0, 0);

    return TRUE;
}
