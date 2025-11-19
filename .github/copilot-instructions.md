## piARno App — Copilot instructions

Purpose: Give an AI coding agent immediate, actionable context to work productively in this repository (builds, key files, patterns, and integration points). Keep edits focused and small: prefer touching C++/JNI code under `app/jni/` or `SampleXrFramework/Src` and preserve Gradle/CMake wiring unless a clear build need exists.

Quick start (commands you can run or suggest):
- Build the Android app (invokes native CMake build):
  - ./gradlew :app:assembleDebug
  - Or run the root task: ./gradlew buildApp (delegates to :app:assembleDebug)
- Install / debug on a connected Quest / device: ./gradlew :app:installDebug (standard Gradle install)
- Note: `app/build.gradle` defines an `installNativeLibs` task that runs `cmake --install` from the Gradle build output (useful when extracting native artifacts)

Where to look first (entrypoints & important dirs):
- app/ - Android application module. `app/build.gradle` shows compileSdk 34, ndkVersion, and externalNativeBuild cmake pointing at `jni/CMakeLists.txt`.
- app/jni/ and jni/CMakeLists.txt - native C/C++ sources and cmake build rules. This is the main place for native fixes/features.
- 1stParty/OVR/Include - Oculus/OVR helper headers and small framework utilities used across native code (e.g., `OVR_*.h`).
- SampleCommon/Src and SampleXrFramework/Src - sample frameworks and `XrApp.cpp` contain application-level XR/engine code and good examples for OpenXR usage.
- OpenXR/Libs/Android - prebuilt OpenXR/OpenGL support files (integration point with platform SDKs).

Project-specific patterns and conventions
- Build: Gradle orchestrates Java and the native build via `externalNativeBuild { cmake { path "jni/CMakeLists.txt" }}`. Do not assume `ndk-build`; this project uses CMake by default.
- ABI: default ndk `abiFilters` is `arm64-v8a` in `app/build.gradle`.
- Native install: `tasks.register("installNativeLibs", Exec)` will run `cmake --install` from the build intermediates. Use this to extract compiled .so if needed.
- 3rd-party: header-only libs (e.g., `3rdParty/nlohmann/json.hpp`) and prebuilt libs live under `3rdParty/` and `OpenXR/Libs/Android` respectively—avoid rebuilding third-party unless required.

Code & data flow summary (big picture)
- Java/Android app (app/) launches and loads native libraries from `app/jni` via JNI.
- Native C++ (app/jni and SampleXrFramework/Src) implements rendering and OpenXR interactions. `SampleCommon` abstracts platform details.
- OpenXR SDK and vendor-specific runtime are linked from `OpenXR/Libs/Android` and the `1stParty/OVR` headers provide helper abstractions.

Examples to cite in edits
- When referring to rendering/XR flow, point to `SampleXrFramework/Src/XrApp.cpp` and `SampleCommon/Src/OVR_Stream.cpp` for I/O helpers.
- When changing build flags or ABI, update `app/build.gradle` (ndk/abiFilters and `externalNativeBuild` arguments) and `jni/CMakeLists.txt` for the native side.

> Contract for small changes an AI may implement
- Inputs: single-file bug report or small feature description (target file path + failing example). Output: minimal change with tests or build verification steps.
- Success criteria: `./gradlew :app:assembleDebug` still completes; native .so produced for `arm64-v8a`; Java package unchanged unless requested.

Common gotchas the agent should check before proposing a change
- Changing the NDK version or cmake flags in `app/build.gradle` can break CI or local builds — prefer small flag changes and cite the reason.
- Many example assets are pre-converted into C arrays or stored under `models`/`songs` (see README note); avoid reintroducing raw file-loading code unless also updating packaging.

If you need more context or CI commands
- Tell me whether you want the agent to modify native code, Java code, or Gradle wiring. Provide a specific target (file + short summary). I can then run a focused build and verify.

Files to reference while coding: `app/build.gradle`, `jni/CMakeLists.txt`, `app/jni/` (native sources), `SampleXrFramework/Src/XrApp.cpp`, `SampleCommon/Src`, `1stParty/OVR/Include`, and `OpenXR/Libs/Android`.

If this file existed previously: merge strategy
- Preserve any existing step-by-step commands. Add missing items above (CMake path, `installNativeLibs` task, preferred ABI). Keep the file short (20–50 lines) and repository-specific.

Last step: ask for missing details
- If there are preferred CI targets, tests to run, or specific device deployment steps (SideQuest vs ADB over USB), tell me and I'll add them.
