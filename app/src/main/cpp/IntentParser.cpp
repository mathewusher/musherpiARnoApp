#include "IntentParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

float extractNumber(const std::string& input) {
    std::istringstream iss(input);
    float num;
    while (iss >> num) return num;
    return 10.0f; // default if not found
}

std::string extractSongName(const std::string& input) {
    size_t pos = input.find("play ");
    if (pos == std::string::npos) pos = input.find("learn ");
    if (pos != std::string::npos) {
        std::string name = input.substr(pos + 5);
        return name;
    }
    return "";
}

Intent parseIntent(const std::string& input) {
    std::string lower = toLower(input);
    Intent result;

    if (lower.find("faster") != std::string::npos || lower.find("speed up") != std::string::npos) {
        result.id = "speed_up";
    } else if (lower.find("slower") != std::string::npos || lower.find("slow down") != std::string::npos) {
        result.id = "slow_down";
    } else if (lower.find("rewind") != std::string::npos || lower.find("back") != std::string::npos) {
        result.id = "rewind";
        result.seconds = extractNumber(lower);
    } else if (lower.find("skip") != std::string::npos || lower.find("forward") != std::string::npos) {
        result.id = "fast_forward";
        result.seconds = extractNumber(lower);
    } else if (lower.find("loop") != std::string::npos || lower.find("again") != std::string::npos) {
        result.id = "loop";
    } else if (lower.find("play") != std::string::npos || lower.find("learn") != std::string::npos) {
        result.id = "load_song";
        result.songName = extractSongName(lower);
    }

    return result;
}
