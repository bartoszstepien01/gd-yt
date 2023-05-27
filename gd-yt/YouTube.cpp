#include "pch.h"
#include "YouTube.h"
#include "Utils.h"
#include <fstream>

YouTube* YouTube::instance = nullptr;
std::mutex YouTube::mutex_;
const std::string YouTube::LEGAL_YOUTUBE_ID_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

bool YouTube::saveCacheToFile() {
    try {
        nlohmann::json serialized;

        for (auto& [id, data] : cache) {
            serialized[id] = {
                { "id", id },
                { "name", data.title },
                { "artistID", data.channelID },
                { "artist", data.channel },
                { "url", data.url }
            };
        }

        std::ofstream cacheFile(cacheFilePath);
        cacheFile << std::setw(4) << serialized;
        cacheFile.close();
    }
    catch (std::exception e) {
        return false;
    }
}

YouTube::YouTube() {

}

YouTube::~YouTube() {
}

bool YouTube::isYoutubeID(std::string id) {
    if (id.length() != 11) return false;
    for (char c : id) {
        if (LEGAL_YOUTUBE_ID_CHARACTERS.find(c) == std::string::npos) return false;
    }

    return true;
}

bool YouTube::loadCache(std::string filePath) {
    try {
        cacheFilePath = filePath;
        std::ifstream cacheFile(filePath);

        nlohmann::json cacheJSON = nlohmann::json::parse(cacheFile);
        cacheFile.close();

        for (auto& [key, val] : cacheJSON.items())
        {
            cache[key] = {
                key,
                val["artistID"],
                val["name"],
                val["artist"],
                val["url"]
            };
        }

        return true;
    }
    catch (std::exception e) {
        return false;
    }
}

YouTube::VideoData YouTube::getData(std::string id) {
    if (!isYoutubeID(id)) return {""};
    if (cache.count(id)) return cache[id];

    try {
        std::string command = "gd-yt\\yt-dlp -J \"" + id + "\" --no-warnings";
        std::string output;

        int returnCode = runCmd(command.c_str(), output);
        if (returnCode != 0) return { "" };

        nlohmann::json outputJSON = nlohmann::json::parse(output);

        cache[id] = {
            id,
            outputJSON["uploader_id"],
            outputJSON["title"],
            outputJSON["uploader"],
            outputJSON["webpage_url"]
        };

        saveCacheToFile();

        return cache[id];
    }
    catch (std::exception e) {
        return { "" };
    }
}

bool YouTube::downloadVideo(std::string id) {
    try {
        if (!isYoutubeID(id)) return false;

        std::string command = "gd-yt\\yt-dlp -x --audio-format mp3 \"" + id + "\" -o \"gd-yt/downloads/%(id)s.%(ext)s\"";
        std::string output;

        int returnCode = runCmd(command.c_str(), output);
        if (returnCode != 0) return false;

        return std::filesystem::exists("gd-yt/downloads/" + id + ".mp3");
    }
    catch (std::exception e) {
        return false;
    }
}

YouTube* YouTube::getInstance() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance == nullptr) {
        instance = new YouTube();
    }
    return instance;
}
