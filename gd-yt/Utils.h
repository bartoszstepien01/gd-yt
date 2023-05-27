#pragma once
#include "pch.h"
#include "YouTube.h"

// Source: https://stackoverflow.com/questions/7063859/c-popen-command-without-console
int runCmd(const char* cmd, std::string& outOutput);

std::string getYoutubeIDFromDescription(std::string description);

gd::SongInfoObject* videoDataToSongInfoObject(YouTube::VideoData data);