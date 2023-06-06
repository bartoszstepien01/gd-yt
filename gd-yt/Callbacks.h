#pragma once


class Callbacks
{
    static gd::CCMenuItemSpriteExtra* ytDownloadButton;
    static gd::CCMenuItemSpriteExtra* ytUseButton;
    static cocos2d::CCSprite* ytUsed;
    static cocos2d::CCMenu* songWidgetMenu;
public:
    static gd::GJGameLevel* level;
    static gd::CCTextInputNode* input;
    static gd::CCMenuItemSpriteExtra* ytSearchButton;
    static gd::CCMenuItemSpriteExtra* ngSearchButton;
    static gd::CustomSongWidget* songWidget;
    static gd::CustomSongWidget* levelSettingsSongWidget;

    void toggleCallback(gd::CCMenuItemToggler* sender);
    void buttonCallback(gd::CCMenuItemSpriteExtra* sender);
    void downloadCallback(gd::CCMenuItemSpriteExtra* button);
    void useCallback(gd::CCMenuItemSpriteExtra* button);
};

