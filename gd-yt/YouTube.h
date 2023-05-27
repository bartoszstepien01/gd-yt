#pragma once
#include "pch.h"
#include <mutex>

class YouTube {
public:
	struct VideoData {
        std::string id;
        std::string channelID;
        std::string title;
        std::string channel;
        std::string url;
	};
private:
    std::map<std::string, VideoData> cache;
    std::string cacheFilePath;
    bool saveCacheToFile();

    static YouTube* instance;
    static std::mutex mutex_;

    YouTube();
    ~YouTube();
public:
    YouTube(YouTube& other) = delete;
    void operator=(const YouTube&) = delete;

    static const std::string LEGAL_YOUTUBE_ID_CHARACTERS;
    static bool isYoutubeID(std::string str);
    bool loadCache(std::string filePath);
    VideoData getData(std::string id);
    bool downloadVideo(std::string id);

    static YouTube* getInstance();
};