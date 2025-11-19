# AI Song Generation Feature

## Overview
The piARno app now supports AI-powered dynamic music sheet generation. Users can request AI to create piano music sheets for any song, which are then automatically loaded into the app for teaching.

## Features

### 1. AI Song Generator (`AISongGenerator`)
- **Location**: `app/src/main/cpp/AISongGenerator.h` and `.cpp`
- **Functionality**:
  - Makes HTTP requests to OpenAI API to generate music
  - Converts AI responses (JSON format) to MIDI files
  - Saves generated MIDI files to the songs directory
  - Falls back to simple MIDI generation if API is unavailable
  - Runs asynchronously to avoid blocking the main thread

### 2. Voice/Text Commands
Users can trigger AI generation using voice commands or text input:
- "create song [description]"
- "generate music [description]"
- "make a [description] song"
- "ai create [description]"

Examples:
- "create a happy birthday song"
- "generate a calm melody"
- "make a jazz piece"

### 3. UI Integration
- **AI Button**: A purple button labeled "AI" appears in the left control panel (below the song list)
- **Status Display**: Real-time status messages show:
  - "Generating song..." (during generation)
  - "Song generated! Loading..." (after generation)
  - "Song loaded!" (when ready to play)
  - Error messages if generation fails

### 4. Automatic Loading
Once a song is generated, it is automatically:
1. Saved to `/sdcard/Android/data/com.oculus.xrpassthrough/files/songs/`
2. Loaded into the piano app
3. Ready to play immediately

## Configuration

### API Key Setup
The AI service uses OpenAI's API. To configure:

1. **Environment Variable** (recommended):
   ```bash
   export OPENAI_API_KEY="your-api-key-here"
   ```

2. **Programmatic** (in code):
   ```cpp
   engine->aiSongGenerator.SetApiKey("your-api-key-here");
   ```

3. **Custom Endpoint**:
   ```cpp
   engine->aiSongGenerator.SetApiEndpoint("https://your-custom-api.com/generate");
   ```

### Default Behavior
- If no API key is set, the system falls back to generating simple MIDI files
- The fallback creates a basic scale-based melody
- This ensures the feature works even without internet/API access

## Technical Details

### File Structure
- **AI Service**: `app/src/main/cpp/AISongGenerator.{h,cpp}`
- **Integration**: `app/src/main/cpp/Engine.{h,cpp}`
- **UI**: `app/src/main/cpp/Piarno.{h,cpp}`
- **Intent Parsing**: `app/src/main/cpp/IntentParser.cpp`

### Dependencies
- **curl**: Already included in the project for HTTP requests
- **nlohmann/json**: Already included for JSON parsing
- **OpenAI API**: Requires internet connection and API key

### MIDI Format
Generated songs are saved as standard MIDI files (`.mid`) compatible with:
- The existing MIDI parser in the app
- Standard MIDI players
- Other music software

### Thread Safety
- AI generation runs in a separate thread
- Status updates are thread-safe
- Main rendering thread is never blocked

## Usage Examples

### Via Voice Input
1. Press the "AI" button in VR
2. Say: "create a happy song"
3. Wait for generation (status will update)
4. Song automatically loads and is ready to play

### Via Code/Testing
```cpp
// In Engine.cpp or test code
engine->HandleUserInput("create a calm piano piece");
```

## Error Handling
- Network errors: Falls back to simple MIDI generation
- API errors: Shows error message in UI
- File errors: Logs error and shows status message
- Invalid responses: Attempts to parse, falls back if needed

## Future Enhancements
Potential improvements:
1. Support for other AI music generation APIs (MusicGen, etc.)
2. More sophisticated MIDI generation algorithms
3. User preferences for music style/complexity
4. Batch generation of multiple songs
5. Caching of generated songs
6. Integration with music databases

## Notes
- Generated songs are saved permanently in the songs directory
- Songs can be accessed later like any other song in the list
- The feature requires internet connectivity for full functionality
- API usage may incur costs depending on your OpenAI plan

