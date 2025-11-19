#include "AISongGenerator.h"
#include <curl/curl.h>
#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <tuple>
#include <cstdlib>

using json = nlohmann::json;

// Callback for curl to write response data
struct WriteCallbackData {
    std::string data;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    WriteCallbackData* data = static_cast<WriteCallbackData*>(userp);
    data->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

AISongGenerator::AISongGenerator() 
    : status(Status::Idle), shouldCancel(false) {
    // Default to OpenAI API
    apiEndpoint = "https://api.openai.com/v1/chat/completions";
}

AISongGenerator::~AISongGenerator() {
    Cancel();
    if (generationThread && generationThread->joinable()) {
        generationThread->join();
    }
}

void AISongGenerator::Cancel() {
    shouldCancel = true;
    if (generationThread && generationThread->joinable()) {
        generationThread->join();
    }
    shouldCancel = false;
}

void AISongGenerator::GenerateSong(const std::string& prompt,
                                   std::function<void(bool, const std::string&, const std::string&)> callback) {
    if (status == Status::Generating) {
        callback(false, "", "Generation already in progress");
        return;
    }

    status = Status::Generating;
    lastError = "";

    // Start generation in a separate thread
    if (generationThread && generationThread->joinable()) {
        generationThread->join();
    }

    generationThread = std::make_unique<std::thread>(
        &AISongGenerator::GenerationThread, this, prompt, callback);
}

void AISongGenerator::GenerationThread(const std::string& prompt,
                                      std::function<void(bool, const std::string&, const std::string&)> callback) {
    try {
        // Try OpenAI first, fallback to simple MIDI generation
        std::string midiData;
        std::string songName = prompt;
        
        // Extract song name from prompt (remove "create", "generate", etc.)
        std::string lowerPrompt = prompt;
        std::transform(lowerPrompt.begin(), lowerPrompt.end(), lowerPrompt.begin(), ::tolower);
        
        if (lowerPrompt.find("create") != std::string::npos) {
            size_t pos = lowerPrompt.find("create");
            songName = prompt.substr(pos + 7);
        } else if (lowerPrompt.find("generate") != std::string::npos) {
            size_t pos = lowerPrompt.find("generate");
            songName = prompt.substr(pos + 9);
        } else if (lowerPrompt.find("make") != std::string::npos) {
            size_t pos = lowerPrompt.find("make");
            songName = prompt.substr(pos + 5);
        }
        
        // Trim whitespace
        songName.erase(0, songName.find_first_not_of(" \t\n\r"));
        songName.erase(songName.find_last_not_of(" \t\n\r") + 1);
        
        if (songName.empty()) {
            songName = "AI Generated Song";
        }

        // Try to get AI-generated notes
        std::string aiResponse = MakeOpenAIRequest(prompt);
        
        if (!aiResponse.empty() && !shouldCancel) {
            midiData = GenerateMIDIFromJSON(aiResponse);
        }
        
        // Fallback: Generate a simple MIDI if AI request failed
        if (midiData.empty() && !shouldCancel) {
            // Generate a simple melody based on the prompt
            // This is a fallback that creates a basic MIDI file
            std::vector<std::tuple<int, float, float>> notes;
            
            // Simple happy birthday-like melody as example
            // C4, C4, D4, C4, F4, E4
            int baseNote = 60; // C4
            float tempo = 0.5f; // seconds per note
            float currentTime = 0.0f;
            
            // Generate a simple scale-based melody
            int scale[] = {0, 2, 4, 5, 7, 9, 11, 12}; // C major scale
            for (int i = 0; i < 16 && !shouldCancel; ++i) {
                int note = baseNote + scale[i % 8];
                notes.push_back(std::make_tuple(note, currentTime, tempo * 0.8f));
                currentTime += tempo;
            }
            
            midiData = CreateSimpleMIDI(songName, notes);
        }

        if (shouldCancel) {
            status = Status::Idle;
            callback(false, "", "Generation cancelled");
            return;
        }

        if (midiData.empty()) {
            status = Status::Error;
            lastError = "Failed to generate MIDI data";
            callback(false, "", lastError);
            return;
        }

        // Save MIDI file
        std::string filename = SanitizeFilename(songName) + ".mid";
        std::string filepath = std::string(SONGS_BASE_PATH) + filename;

        // Ensure directory exists
        std::filesystem::create_directories(SONGS_BASE_PATH);

        if (SaveMIDIFile(midiData, filepath)) {
            status = Status::Success;
            callback(true, filepath, "");
        } else {
            status = Status::Error;
            lastError = "Failed to save MIDI file";
            callback(false, "", lastError);
        }

    } catch (const std::exception& e) {
        status = Status::Error;
        lastError = std::string("Exception: ") + e.what();
        callback(false, "", lastError);
    }
}

std::string AISongGenerator::MakeOpenAIRequest(const std::string& prompt) {
    if (apiKey.empty()) {
        // Try to get from environment or use a default
        const char* envKey = std::getenv("OPENAI_API_KEY");
        if (!envKey) {
            std::cerr << "[AISongGenerator] No API key set, using fallback generation" << std::endl;
            return "";
        }
        apiKey = envKey;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[AISongGenerator] Failed to initialize curl" << std::endl;
        return "";
    }

    WriteCallbackData responseData;
    
    // Build JSON request
    json request;
    request["model"] = "gpt-4o-mini"; // Use a cheaper model
    request["messages"] = json::array({
        {
            {"role", "system"},
            {"content", "You are a music composition assistant. Generate piano sheet music in JSON format. "
                       "Return ONLY a JSON array of notes, each with 'pitch' (MIDI note 0-127), 'start' (seconds), and 'duration' (seconds). "
                       "Example: [{\"pitch\":60,\"start\":0.0,\"duration\":0.5},{\"pitch\":62,\"start\":0.5,\"duration\":0.5}]. "
                       "Keep it simple, 30-60 seconds of music, suitable for piano learning."}
        },
        {
            {"role", "user"},
            {"content", "Create a piano arrangement for: " + prompt}
        }
    });
    request["temperature"] = 0.7;
    request["max_tokens"] = 2000;

    std::string jsonStr = request.dump();

    // Set up curl
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, apiEndpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || responseCode != 200) {
        std::cerr << "[AISongGenerator] Request failed: " << curl_easy_strerror(res) 
                  << ", Response code: " << responseCode << std::endl;
        return "";
    }

    // Parse response
    try {
        json response = json::parse(responseData.data);
        if (response.contains("choices") && response["choices"].is_array() && !response["choices"].empty()) {
            std::string content = response["choices"][0]["message"]["content"];
            
            // Try to extract JSON from the response (might have markdown code blocks)
            size_t jsonStart = content.find('[');
            size_t jsonEnd = content.rfind(']');
            if (jsonStart != std::string::npos && jsonEnd != std::string::npos && jsonEnd > jsonStart) {
                return content.substr(jsonStart, jsonEnd - jsonStart + 1);
            }
            return content;
        }
    } catch (const std::exception& e) {
        std::cerr << "[AISongGenerator] Failed to parse response: " << e.what() << std::endl;
    }

    return "";
}

std::string AISongGenerator::GenerateMIDIFromJSON(const std::string& jsonResponse) {
    try {
        json notes = json::parse(jsonResponse);
        if (!notes.is_array()) {
            return "";
        }

        std::vector<std::tuple<int, float, float>> noteList;
        for (const auto& note : notes) {
            if (note.contains("pitch") && note.contains("start") && note.contains("duration")) {
                int pitch = note["pitch"];
                float start = note["start"];
                float duration = note["duration"];
                noteList.push_back(std::make_tuple(pitch, start, duration));
            }
        }

        if (!noteList.empty()) {
            return CreateSimpleMIDI("AI Generated", noteList);
        }
    } catch (const std::exception& e) {
        std::cerr << "[AISongGenerator] Failed to parse JSON: " << e.what() << std::endl;
    }

    return "";
}

std::string AISongGenerator::CreateSimpleMIDI(const std::string& songName,
                                              const std::vector<std::tuple<int, float, float>>& notes) {
    // Create a simple MIDI file
    // This is a minimal MIDI file structure
    std::ostringstream midi;
    
    // MIDI file header
    midi << "MThd";                    // Chunk type
    midi.write("\x00\x00\x00\x06", 4); // Chunk length (6 bytes)
    midi.write("\x00\x01", 2);         // Format (single track)
    midi.write("\x00\x01", 2);         // Number of tracks
    midi.write("\x00\x60", 2);         // Ticks per quarter note (96)
    
    // Track chunk
    midi << "MTrk";                    // Track chunk type
    
    // Calculate track length (we'll write it later)
    std::streampos trackLengthPos = midi.tellp();
    midi.write("\x00\x00\x00\x00", 4); // Placeholder for track length
    
    // Track events
    uint32_t ticksPerQuarter = 96;
    uint32_t currentTick = 0;
    
    // Set tempo (120 BPM)
    uint32_t tempo = 500000; // microseconds per quarter note
    uint32_t deltaTime = 0;
    
    // Write tempo meta event
    midi.put(0x00); // Delta time (variable length encoded, 0 = 0x00)
    midi.put(0xFF); // Meta event
    midi.put(0x51); // Set tempo
    midi.put(0x03); // Length
    midi.put((tempo >> 16) & 0xFF);
    midi.put((tempo >> 8) & 0xFF);
    midi.put(tempo & 0xFF);
    
    // Write notes
    for (const auto& note : notes) {
        int pitch = std::get<0>(note);
        float startTime = std::get<1>(note);
        float duration = std::get<2>(note);
        
        // Convert time to ticks (assuming 120 BPM = 2 beats per second)
        uint32_t startTick = static_cast<uint32_t>(startTime * ticksPerQuarter * 2);
        uint32_t durationTicks = static_cast<uint32_t>(duration * ticksPerQuarter * 2);
        
        // Delta time from previous event
        uint32_t delta = startTick - currentTick;
        
        // Write variable-length delta time
        if (delta < 0x80) {
            midi.put(delta & 0x7F);
        } else if (delta < 0x4000) {
            midi.put(0x80 | ((delta >> 7) & 0x7F));
            midi.put(delta & 0x7F);
        } else {
            midi.put(0x80 | ((delta >> 14) & 0x7F));
            midi.put(0x80 | ((delta >> 7) & 0x7F));
            midi.put(delta & 0x7F);
        }
        
        // Note on (channel 0)
        midi.put(0x90); // Note on, channel 0
        midi.put(pitch & 0x7F);
        midi.put(0x64); // Velocity
        
        currentTick = startTick;
        
        // Note off
        delta = durationTicks;
        if (delta < 0x80) {
            midi.put(delta & 0x7F);
        } else if (delta < 0x4000) {
            midi.put(0x80 | ((delta >> 7) & 0x7F));
            midi.put(delta & 0x7F);
        } else {
            midi.put(0x80 | ((delta >> 14) & 0x7F));
            midi.put(0x80 | ((delta >> 7) & 0x7F));
            midi.put(delta & 0x7F);
        }
        
        midi.put(0x80); // Note off, channel 0
        midi.put(pitch & 0x7F);
        midi.put(0x00); // Velocity
        
        currentTick = startTick + durationTicks;
    }
    
    // End of track
    midi.put(0x00); // Delta time
    midi.put(0xFF); // Meta event
    midi.put(0x2F); // End of track
    midi.put(0x00); // Length
    
    // Write track length
    std::streampos endPos = midi.tellp();
    uint32_t trackLength = static_cast<uint32_t>(endPos - trackLengthPos - 4);
    midi.seekp(trackLengthPos);
    midi.put((trackLength >> 24) & 0xFF);
    midi.put((trackLength >> 16) & 0xFF);
    midi.put((trackLength >> 8) & 0xFF);
    midi.put(trackLength & 0xFF);
    midi.seekp(endPos);
    
    return midi.str();
}

bool AISongGenerator::SaveMIDIFile(const std::string& midiData, const std::string& filepath) {
    try {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[AISongGenerator] Failed to open file: " << filepath << std::endl;
            return false;
        }
        
        file.write(midiData.data(), midiData.size());
        file.close();
        
        std::cout << "[AISongGenerator] Saved MIDI file: " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[AISongGenerator] Exception saving file: " << e.what() << std::endl;
        return false;
    }
}

std::string AISongGenerator::SanitizeFilename(const std::string& name) {
    std::string sanitized = name;
    // Replace invalid filename characters
    std::replace(sanitized.begin(), sanitized.end(), ' ', '_');
    std::replace(sanitized.begin(), sanitized.end(), '/', '_');
    std::replace(sanitized.begin(), sanitized.end(), '\\', '_');
    std::replace(sanitized.begin(), sanitized.end(), ':', '_');
    std::replace(sanitized.begin(), sanitized.end(), '*', '_');
    std::replace(sanitized.begin(), sanitized.end(), '?', '_');
    std::replace(sanitized.begin(), sanitized.end(), '"', '_');
    std::replace(sanitized.begin(), sanitized.end(), '<', '_');
    std::replace(sanitized.begin(), sanitized.end(), '>', '_');
    std::replace(sanitized.begin(), sanitized.end(), '|', '_');
    
    // Limit length
    if (sanitized.length() > 100) {
        sanitized = sanitized.substr(0, 100);
    }
    
    return sanitized;
}

