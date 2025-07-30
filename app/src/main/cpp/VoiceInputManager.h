#pragma once
#include <string>
#include <functional>

class VoiceInputManager {
public:
    void SetInputCallback(std::function<void(const std::string&)> callback);
    void SimulateInput(const std::string& voiceText); // For testing
private:
    std::function<void(const std::string&)> onInput;
};
