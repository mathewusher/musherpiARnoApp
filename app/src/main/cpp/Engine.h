#include <cstdint>
//
// Created by JW on 25/06/2022.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "Global.h"
#include "XrPassthroughGl.h"
#include "Piarno.h"
#include "VoiceInputManager.h"

//DEBUG LOGGING
#include "android/log.h"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PIARNO", __VA_ARGS__)
void log(std::string s);

enum class Mesh : size_t {
    axes = 38, //ranges 0-25 are for alphabets A-Z, 26-35 for 0-9, 36 for . and 37 for :
    cube,
    rect,
    rectGradient,
    wireframe,
    teapot,
    NUM
};

enum class IO : size_t {
    leftTrigger,
    rightTrigger,
    leftSqueeze,
    rightSqueeze,
    xButton,
    yButton,
    aButton,
    bButton,
    NUM
};

//a layer that abstracts away a lot of the confusing OpenXR implementation details
//use this class to communicate with the rest of the openxr stuffs
class Engine {

public:
    /// API calls for higher level stuff (piARno)

    // General
    uint64_t getFrame();

    // Input
    const std::vector<Rigid>& getControllers();
    bool isButtonPressed(IO button);
    float getRightTriggerHoldLevel();

    // Render related
    Geometry* getGeometry(Mesh mesh);
    float textWidth(const std::string &text);
    void renderText(const std::string &text, vec3 pos, vec3 scl, vec3 rot, const color &col, bool centered = true);

    /**************** YOU ARE NOW ENTERING LOW LEVEL ****************/

    //API calls for lower level stuff (OpenXR and OpenGL)
    Engine(Scene *scene);
    void update();
    void render();
    static std::vector<Geometry> loadGeometries();

    // AI and voice integration additions
    void HandleUserInput(const std::string& input);
    void LoadSongByName(const std::string& name);
    void ParseAndLoadSong(const std::string& data);

    float playbackSpeed = 1.0f;
    float currentPlaybackTime = 0.0f;
    float loopStart = 0.0f;
    float loopEnd = 0.0f;
    bool isLooping = false;

protected:
    Scene *scene;
    Piarno piarno;
    std::vector<Rigid> controllers;

    uint64_t frame = 0;
    std::array<XrBool32*, (size_t) IO::NUM> buttonStates;

    static std::array<float, 38> fontWidth;

    VoiceInputManager voiceInput;
};
