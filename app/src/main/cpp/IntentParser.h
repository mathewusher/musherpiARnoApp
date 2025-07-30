#pragma once
#include <string>

struct Intent {
    std::string id;
    std::string songName;
    float seconds = 0.0f;
};

Intent parseIntent(const std::string& input);
