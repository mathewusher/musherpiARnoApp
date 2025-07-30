#pragma once
#include <string>

struct Intent {
    std::string id;
    std::string songName;
    float seconds;
};

Intent parseIntent(const std::string& input);
