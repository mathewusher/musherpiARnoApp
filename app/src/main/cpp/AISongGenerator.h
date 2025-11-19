#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <tuple>

// Forward declaration for curl
struct curl_slist;

/**
 * AISongGenerator - Generates MIDI music sheets using AI services
 * 
 * This class handles:
 * - HTTP requests to AI APIs (OpenAI, music generation services)
 * - Converting AI responses to MIDI format
 * - Saving generated MIDI files to the songs directory
 * - Async processing to avoid blocking the main thread
 */
class AISongGenerator {
public:
    enum class Status {
        Idle,
        Generating,
        Success,
        Error
    };

    AISongGenerator();
    ~AISongGenerator();

    /**
     * Generate a MIDI song from a text prompt
     * @param prompt User's request (e.g., "Create a happy birthday song")
     * @param callback Called when generation completes (success or error)
     */
    void GenerateSong(const std::string& prompt, 
                     std::function<void(bool success, const std::string& filePath, const std::string& errorMsg)> callback);

    /**
     * Get current generation status
     */
    Status GetStatus() const { return status; }

    /**
     * Get the last error message
     */
    std::string GetLastError() const { return lastError; }

    /**
     * Set API key for OpenAI (optional, can use environment variable)
     */
    void SetApiKey(const std::string& key) { this->apiKey = key; }

    /**
     * Set custom API endpoint (for custom music generation services)
     */
    void SetApiEndpoint(const std::string& endpoint) { this->apiEndpoint = endpoint; }

    /**
     * Cancel any ongoing generation
     */
    void Cancel();

private:
    // HTTP request helpers
    std::string MakeOpenAIRequest(const std::string& prompt);
    
    // MIDI generation helpers
    std::string GenerateMIDIFromJSON(const std::string& jsonResponse);
    std::string CreateSimpleMIDI(const std::string& songName, const std::vector<std::tuple<int, float, float>>& notes);
    
    // File operations
    bool SaveMIDIFile(const std::string& midiData, const std::string& filename);
    std::string SanitizeFilename(const std::string& name);
    
    // Thread management
    void GenerationThread(const std::string& prompt, 
                         std::function<void(bool, const std::string&, const std::string&)> callback);

    std::atomic<Status> status;
    std::string lastError;
    std::string apiKey;
    std::string apiEndpoint;
    std::unique_ptr<std::thread> generationThread;
    std::atomic<bool> shouldCancel;
    
    // Base path for saving songs
    static constexpr const char* SONGS_BASE_PATH = "/sdcard/Android/data/com.oculus.xrpassthrough/files/songs/";
};

