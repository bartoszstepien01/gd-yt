#include "pch.h"
#include "Utils.h"

// Source: https://stackoverflow.com/questions/7063859/c-popen-command-without-console
int runCmd(const char* cmd, std::string& outOutput) {
    if (strlen(cmd) >= 128) return 1;

    wchar_t wtext[128];
    size_t outSize;
    mbstowcs_s(&outSize, wtext, 128, cmd, strlen(cmd));
    LPWSTR ptr = wtext;

    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    HANDLE g_hChildStd_ERR_Rd = NULL;
    HANDLE g_hChildStd_ERR_Wr = NULL;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &sa, 0)) { return 1; }
    if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0)) { return 1; }
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0)) { return 1; }
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) { return 1; }

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    bool bSuccess = FALSE;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_ERR_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcess(
        NULL,
        ptr,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    CloseHandle(g_hChildStd_ERR_Wr);
    CloseHandle(g_hChildStd_OUT_Wr);

#define BUFSIZE 128
    DWORD dwRead;
    CHAR chBuf[BUFSIZE];
    bool bSuccess2 = FALSE;
    for (;;) {
        bSuccess2 = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess2 || dwRead == 0) break;
        std::string s(chBuf, dwRead);
        outOutput += s;
    }
    dwRead = 0;
    for (;;) {
        bSuccess2 = ReadFile(g_hChildStd_ERR_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess2 || dwRead == 0) break;
        std::string s(chBuf, dwRead);
        outOutput += s;
    }

    return 0;
}

std::string getYoutubeIDFromDescription(std::string description) {
    std::string decoded;
    macaron::Base64::Decode(description, decoded);

    size_t begin = decoded.find('$');
    size_t end = decoded.find('$', begin + 1);

    if (begin == std::string::npos || end == std::string::npos) return "";

    std::string youtubeID = decoded.substr(begin + 1, end - begin - 1);

    if (!YouTube::isYoutubeID(youtubeID)) return "";
    return youtubeID;
}

gd::SongInfoObject* videoDataToSongInfoObject(YouTube::VideoData data) {
    gd::SongInfoObject* info = new gd::SongInfoObject;
    info->m_nSongID = 42069;
    info->m_sSongName = data.title;
    info->m_sArtistName = data.channel;
    info->m_sSongLink = data.url;
    info->m_sYouTubeChannelURL = data.id;
    info->m_nArtistID = -1;
    info->m_fFileSize = 21.37;
    info->m_bIsVerified = true;
    info->m_nSongPriority = -1;

    return info;
}