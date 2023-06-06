#include "pch.h"
#include "Callbacks.h"
#include "YouTube.h"
#include "Utils.h"

gd::GJGameLevel* Callbacks::level;
gd::CCTextInputNode* Callbacks::input;
gd::CCMenuItemSpriteExtra* Callbacks::ytSearchButton;
gd::CCMenuItemSpriteExtra* Callbacks::ngSearchButton;
gd::CustomSongWidget* Callbacks::songWidget;
gd::CustomSongWidget* Callbacks::levelSettingsSongWidget;
gd::CCMenuItemSpriteExtra* Callbacks::ytDownloadButton = NULL;
gd::CCMenuItemSpriteExtra* Callbacks::ytUseButton = NULL;
cocos2d::CCSprite* Callbacks::ytUsed = NULL;
cocos2d::CCMenu* Callbacks::songWidgetMenu;

void Callbacks::toggleCallback(gd::CCMenuItemToggler* sender) {
    input->setString("");

    if (sender->isOn()) {
        input->setAllowedChars("0123456789");
        input->setMaxLabelLength(999);
        ytSearchButton->setVisible(false);
        ngSearchButton->setVisible(true);
        /*if (ytDownloadButton) ytDownloadButton->setVisible(false);
        if (ytUseButton) ytUseButton->setVisible(false);
        if (ytUsed) ytUsed->setVisible(false);*/
        ytDownloadButton = NULL;
        ytUseButton = NULL;
        ytUsed = NULL;
    }
    else {
        input->setAllowedChars(YouTube::LEGAL_YOUTUBE_ID_CHARACTERS);
        input->setMaxLabelLength(11);
        ytSearchButton->setVisible(true);
        ngSearchButton->setVisible(false);
    }

    if (sender->isOn()) return;

    std::string youtubeID = getYoutubeIDFromDescription(level->m_sLevelDesc);
    if (youtubeID == "" || level->m_nSongID != 42069) return;

    input->setString(youtubeID.c_str());
    buttonCallback(ytSearchButton);
    input->setString("");
}

void Callbacks::buttonCallback(gd::CCMenuItemSpriteExtra* sender) {
    using namespace cocos2d;

    std::string youtubeID = input->getString();
    YouTube::VideoData data = YouTube::getInstance()->getData(youtubeID);

    if (data.id == "") return;

    gd::SongInfoObject* info = videoDataToSongInfoObject(data);

    level->m_nSongID = 42069;
    songWidget->updateSongObject(info);
    if(levelSettingsSongWidget) levelSettingsSongWidget->updateSongObject(info);

    if (!ytDownloadButton) {
        cocos2d::CCObject* obj;

        CCARRAY_FOREACH(songWidget->getChildren(), obj) {
            CCObject* child;
            int childCount = 0;

            CCARRAY_FOREACH(reinterpret_cast<CCNode*>(obj)->getChildren(), child) {
                CCNode* node = reinterpret_cast<CCNode*>(child);
                if (node->getZOrder() != 21) {
                    childCount++;
                    continue;
                }

                node->removeFromParent();
                delete node;
            };
            if (childCount != 6) continue;

            songWidgetMenu = reinterpret_cast<CCMenu*>(obj);
            break;
        }

        CCSprite* sprite = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        ytDownloadButton = gd::CCMenuItemSpriteExtra::create(sprite, songWidget, menu_selector(Callbacks::downloadCallback));
        ytDownloadButton->setPosition(-157, -180);
        ytDownloadButton->setVisible(false);
        ytDownloadButton->setZOrder(21);

        CCSprite* sprite2 = CCSprite::createWithSpriteFrameName("GJ_selectSongBtn_001.png");
        ytUseButton = gd::CCMenuItemSpriteExtra::create(sprite2, songWidget, menu_selector(Callbacks::useCallback));
        ytUseButton->setPosition(-157, -180);
        ytUseButton->setVisible(false);
        ytUseButton->setZOrder(21);

        ytUsed = CCSprite::createWithSpriteFrameName("GJ_selectSongOnBtn_001.png");
        ytUsed->setPosition({ -157, -180 });
        ytUsed->setVisible(false);
        ytUsed->setZOrder(21);

        songWidgetMenu->addChild(ytDownloadButton);
        songWidgetMenu->addChild(ytUseButton);
        songWidgetMenu->addChild(ytUsed);
    }

    cocos2d::CCObject* obj;

    CCARRAY_FOREACH(songWidgetMenu->getChildren(), obj) {
        CCNode* node = reinterpret_cast<CCNode*>(obj);
        if (node->getZOrder() != 21) node->setVisible(false);
    }

    bool currentlyUsed = false;

    std::string currentYoutubeID = getYoutubeIDFromDescription(level->m_sLevelDesc);
    currentlyUsed = currentYoutubeID == youtubeID;

    if (std::filesystem::exists("gd-yt/downloads/" + youtubeID + ".mp3") && currentlyUsed) {
        ytUsed->setVisible(true);
        ytDownloadButton->setVisible(false);
        ytUseButton->setVisible(false);
    }
    else if (std::filesystem::exists("gd-yt/downloads/" + youtubeID + ".mp3")) {
        ytUseButton->setVisible(true);
        ytDownloadButton->setVisible(false);
        ytUsed->setVisible(false);
    }
    else {
        ytDownloadButton->setVisible(true);
        ytUseButton->setVisible(false);
        ytUsed->setVisible(false);
    }
}

void Callbacks::downloadCallback(gd::CCMenuItemSpriteExtra* button) {
    YouTube::getInstance()->downloadVideo(songWidget->m_songInfo->m_sYouTubeChannelURL);

    ytDownloadButton->setVisible(false);
    ytUseButton->setVisible(true);
}

void Callbacks::useCallback(gd::CCMenuItemSpriteExtra* button) {
    level->m_sLevelDesc = macaron::Base64::Encode("$" + songWidget->m_songInfo->m_sYouTubeChannelURL + "$");
    ytUseButton->setVisible(false);
    ytUsed->setVisible(true);
    try {
        std::filesystem::remove("Resources/42069.mp3");
        std::filesystem::copy("gd-yt/downloads/" + songWidget->m_songInfo->m_sYouTubeChannelURL + ".mp3", "Resources/42069.mp3");
    }
    catch (std::exception e) {

    }
}