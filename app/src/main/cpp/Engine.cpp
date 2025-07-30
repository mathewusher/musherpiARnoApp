//
// Created by JW on 25/06/2022.
//

#include "Engine.h"
#include <vector>
#include <openxr/openxr.h>
#include <string>
#include "GLES3Loader.h"

// Added for AI and JSON parsing
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <iostream>
#include <cstring> // for memcmp
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iterator>

// Global static pointers
Scene* global::scene = nullptr;
Engine* global::engine = nullptr;
Piarno* global::piarno = nullptr;

void log(std::string s) {
    LOGE("%s", s.c_str());
}

// Minimal MIDI event struct for parsing
struct MidiEvent {
    uint32_t startTick;
    uint8_t note;
    uint32_t durationTicks;
};

static bool isMidiFormat(const std::string& data) {
    return data.size() > 4 && memcmp(data.data(), "MThd", 4) == 0;
}

static bool isJsonFormat(const std::string& data) {
    if (data.empty()) return false;
    char first = data.front();
    return first == '{' || first == '[';
}

// Placeholder MIDI parser stub
static std::vector<MidiEvent> parseMidi(const std::string& data) {
    std::vector<MidiEvent> events;
    std::cout << "[WARN] MIDI parsing not implemented yet.\n";
    return events;
}

Engine::Engine(Scene *scene) : scene(scene) {
    global::scene = scene;
    global::engine = this;
    global::piarno = &piarno;

#define register_io(button) buttonStates[(size_t) IO::button] = &scene-> button##Pressed;
    register_io(leftTrigger);
    register_io(rightTrigger);
    register_io(leftSqueeze);
    register_io(rightSqueeze);
    register_io(xButton);
    register_io(yButton);
    register_io(aButton);
    register_io(bButton);

    for(int i = 0; i < 2; i++) {
        auto &c = scene->trackedController[i*2];
        Rigid r{getGeometry(Mesh::cube)};
        r.pos = c.pose.Translation;
        r.rot = vec3{c.pose.Rotation.x, c.pose.Rotation.y, c.pose.Rotation.z};
        r.scl = vec3{0.01f, 0.01f, 0.01f};
        r.col = color{255, 255, 255, 255};
        r.radius = 0.03;
        controllers.push_back(std::move(r));
    }

    piarno.init();
    if (!LoadGLES3Extensions()) {
        LOGE("Failed to load GLES3 symbols!");
    }

    voiceInput.SetInputCallback([this](const std::string& input) {
        this->HandleUserInput(input);
    });

    // Simulated voice commands for testing
    voiceInput.SimulateInput("play Fur Elise");
    voiceInput.SimulateInput("rewind 10");
    voiceInput.SimulateInput("loop last part");
    voiceInput.SimulateInput("speed up");
}

uint64_t Engine::getFrame() {
    return frame;
}

const std::vector<Rigid>& Engine::getControllers() {
    return controllers;
}

bool Engine::isButtonPressed(IO button) {
    return *buttonStates[(size_t) button] == XR_TRUE;
}

float Engine::getRightTriggerHoldLevel() {
    return scene->rightTriggerHoldLevel;
}

Geometry* Engine::getGeometry(Mesh mesh) {
    return &scene->geometries[(size_t) mesh];
}

float Engine::textWidth(const std::string &text) {
    float xOff = 0;
    for (const auto &c: text) {
        if (isspace(c))
            xOff += fontWidth[0];
        else if (auto alpha = toupper(c) - 'A'; 0 <= alpha && alpha < 26)
            xOff += fontWidth[alpha] + 0.1;
        else if (auto num = c - '0'; 0 <= num && num < 10) {
            xOff += fontWidth[26+num] + 0.1;
        } else if (c == '.') {
            xOff += fontWidth[36] + 0.1;
        } else if (c == ':') {
            xOff += fontWidth[37] + 0.1;
        }
    }
    return xOff - (text.size() == 0 ? 0 : 0.1);
}

void Engine::renderText(const std::string &text, vec3 pos, vec3 scl, vec3 rot, const color &col, bool centered) {
    scene->geometries[0].updateColors(col.data);

    float xOff = centered ? -textWidth(text)/2 : 0;
    float yOff = centered ? -0.4 : 0;
    for (const auto &c: text) {
        if (isspace(c)) {
            xOff += fontWidth[0];
            continue;
        }

        mat4 trans = translate(pos) * rotate(rot) * scale(scl) * translate(vec3{xOff, yOff, 0});
        if (auto alpha = toupper(c) - 'A'; 0 <= alpha && alpha < 26) {
            scene->geometries[alpha].render(trans);
            xOff += fontWidth[alpha] + 0.1;
        } else if (auto num = c - '0'; 0 <= num && num < 10) {
            scene->geometries[26+num].render(trans);
            xOff += fontWidth[26+num] + 0.1;
        } else if (c == '.') {
            scene->geometries[36].render(trans);
            xOff += fontWidth[36] + 0.1;
        } else if (c == ':') {
            scene->geometries[37].render(trans);
            xOff += fontWidth[37] + 0.1;
        }
    }
}

void Engine::update() {
    frame++;

    for(int i=0; i<controllers.size(); i++) {
        auto &c = scene->trackedController[i*2];
        if(c.active) {
            auto &r = controllers[i];
            r.pos = c.pose.Translation;
            r.rot = vec3{c.pose.Rotation.x, c.pose.Rotation.y, c.pose.Rotation.z};
        }
    }

    piarno.update();
}

void Engine::render() {
    piarno.render();

    //render controllers
    for(auto &c : controllers)
        c.render();

    //DEBUG render all loaded meshes
    /*
    float x = -1, y = 0, z = -1;
    getGeometry(Mesh::axes)->render(mat4::Translation(x, y, z));
    int i = 0;
    for (auto &g: scene->geometries) {
        if (i > 25) {
            g.render(mat4::Translation(x, y, z));
            x += i > 25 ? 1 : fontWidth[i] + 0.01;
        }
        i++;
    }
    */
}

std::array<float, 38> Engine::fontWidth;

std::vector<Geometry> Engine::loadGeometries() {
    std::vector<Geometry> g((size_t) Mesh::NUM);

    {
#include "models/alphanum.h"
        //split the alphabets into 26 individuals + numbers + dot and colon = 38
        double yMin = vertices[1], yMax = vertices[1];
        for (size_t i = 1; i < vertices.size(); i += 3) {
            if (vertices[i] < yMin)
                yMin = vertices[i];
            if (yMax < vertices[i])
                yMax = vertices[i];
        }
        double margin = (yMax - yMin) * 0.001;
        yMin -= margin;
        yMax += margin;

        const size_t numChars = 38;
        double charHeight = (yMax - yMin);
        std::array<std::vector<vertex_t>, numChars> allVertices;
        std::vector<uint8_t> vertexCharLookup(vertices.size() / 3);
        std::array<std::vector<index_t>, numChars> allIndices;
        std::array<int, numChars> firstVertexIndexPerChar;
        firstVertexIndexPerChar.fill(-1);

        //determine and partition vertices according to its y
        for (size_t i = 1; i < vertices.size(); i += 3) {
            auto alpha = numChars - 1 - (size_t) floor((vertices[i] - yMin) / charHeight * numChars);
            allVertices[alpha].push_back(vertices[i - 1]);
            allVertices[alpha].push_back(vertices[i]);
            allVertices[alpha].push_back(vertices[i + 1]);
            vertexCharLookup[i / 3] = alpha;
            if (firstVertexIndexPerChar[alpha] == -1)
                firstVertexIndexPerChar[alpha] = i / 3;
        }

        //align the alphabet to its lower left corner (0, 0, 0) and scale to 1m height and measure width
        float scale = 1 / (charHeight / numChars);
        for (size_t i = 0; i < numChars; i++) {
            //measure sizes/pos
            vec3 min{std::numeric_limits<float>::max()}, max{std::numeric_limits<float>::lowest()};

            for (size_t j = 0; j < allVertices[i].size(); j += 3) {
                vec3 v{allVertices[i][j], allVertices[i][j+1], allVertices[i][j+2]};
                min = vec3::Min(min, v);
                max = vec3::Max(max, v);
            }
            //align to lower left corner
            for (size_t j = 0; j < allVertices[i].size(); j += 3) {
                allVertices[i][j + 0] -= min.x;
                allVertices[i][j + 1] -= min.y;
                allVertices[i][j + 2] -= min.z;
            }
            //scale to 1m height
            for (auto &v: allVertices[i])
                v *= scale;

            fontWidth[i] = (max.x - min.x) * scale;
        }

        //find which indices belong to which alphabet
        for (size_t i = 0; i < indices.size(); i += 3) {
            //this assumes all 3 indices of a face belong to the same alphabet
            auto alpha = vertexCharLookup[indices[i]];
            auto offset = firstVertexIndexPerChar[alpha];

            allIndices[alpha].push_back(indices[i] - offset);
            allIndices[alpha].push_back(indices[i + 1] - offset);
            allIndices[alpha].push_back(indices[i + 2] - offset);
        }

        for (size_t i = 0; i < numChars; i++)
            g[i] = Geometry(allVertices[i], allIndices[i]);
    }

    {
#include "models/axes.h"

        g[(size_t) Mesh::axes] = Geometry(vertices, colors, indices, GL_LINES);
    }

    {
#include "models/cube.h"

        g[(size_t) Mesh::cube] = Geometry(vertices, indices);
    }

    {
#include "models/rect.h"

        g[(size_t) Mesh::rect] = Geometry(vertices, indices);
    }

    {
#include "models/rect.h"

        g[(size_t) Mesh::rectGradient] = Geometry(vertices, colors, indices);
    }

    {
#include "models/piano_wireframe.h"

        g[(size_t) Mesh::wireframe] = Geometry(vertices, indices, GL_LINES);
    }

    {
#include "models/teapot.h"

        for (auto &v: vertices)
            v /= 100.0;

        g[(size_t) Mesh::teapot] = Geometry(vertices, indices);
    }

    return g;
}

// AI voice/text command handler
void Engine::HandleUserInput(const std::string& input) {
    Intent intent = parseIntent(input);

    if (intent.id == "speed_up") {
        playbackSpeed *= 1.25f;
        std::cout << "[INFO] Playback speed increased to " << playbackSpeed << std::endl;
    } else if (intent.id == "slow_down") {
        playbackSpeed *= 0.8f;
        std::cout << "[INFO] Playback speed decreased to " << playbackSpeed << std::endl;
    } else if (intent.id == "rewind") {
        currentPlaybackTime = std::max(0.0f, currentPlaybackTime - intent.seconds);
        std::cout << "[INFO] Rewound " << intent.seconds << " seconds, new time: " << currentPlaybackTime << std::endl;
    } else if (intent.id == "fast_forward") {
        currentPlaybackTime += intent.seconds;
        std::cout << "[INFO] Fast forwarded " << intent.seconds << " seconds, new time: " << currentPlaybackTime << std::endl;
    } else if (intent.id == "loop") {
        loopStart = currentPlaybackTime - 20.0f;
        if (loopStart < 0) loopStart = 0;
        loopEnd = currentPlaybackTime;
        isLooping = true;
        std::cout << "[INFO] Looping last 20 seconds from " << loopStart << " to " << loopEnd << std::endl;
    } else if (intent.id == "load_song") {
        std::cout << "[INFO] Loading song: " << intent.songName << std::endl;
        LoadSongByName(intent.songName);
    } else {
        std::cout << "[WARN] Unknown intent: " << intent.id << std::endl;
    }
}

void Engine::LoadSongByName(const std::string& name) {
    std::string basePath = "/sdcard/Android/data/com.oculus.xrpassthrough/files/songs/";
    std::string matchedFile;

    for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
        std::string fname = entry.path().filename().string();
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::string lowerFname = fname;
        std::transform(lowerFname.begin(), lowerFname.end(), lowerFname.begin(), ::tolower);

        if (lowerFname.find(lowerName) != std::string::npos) {
            matchedFile = entry.path().string();
            break;
        }
    }

    if (!matchedFile.empty()) {
        std::ifstream file(matchedFile);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        ParseAndLoadSong(content);
    } else {
        std::cerr << "[WARN] Song not found: " << name << std::endl;
    }
}

void Engine::ParseAndLoadSong(const std::string& data) {
    if (isMidiFormat(data)) {
        std::cout << "[INFO] Detected MIDI format.\n";
        auto midiEvents = parseMidi(data);

        for (const auto& ev : midiEvents) {
            // TODO: add tile spawning using your existing tile system, e.g.:
            // piarno.AddTile(ev.note, ev.startTick, ev.durationTicks);
        }
    }
    else if (isJsonFormat(data)) {
        std::cout << "[INFO] Detected JSON format.\n";
        try {
            auto j = json::parse(data);

            for (auto& tile : j) {
                int pitch = tile["pitch"];
                float start = tile["start"];
                float duration = tile["duration"];
                // TODO: add tile spawning using your existing tile system, e.g.:
                // piarno.AddTile(pitch, start, duration);
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] JSON parsing failed: " << e.what() << std::endl;
        }
    }
    else {
        std::cerr << "[ERROR] Unknown song format.\n";
    }
}
