#include "VoiceInputManager.h"

void VoiceInputManager::SetInputCallback(std::function<void(const std::string&)> callback) {
    onInput = callback;
}

void VoiceInputManager::SimulateInput(const std::string& voiceText) {
    if (onInput) onInput(voiceText);
}
