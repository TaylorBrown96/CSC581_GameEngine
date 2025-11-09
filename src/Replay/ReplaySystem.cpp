#include "ReplaySystem.h"

#include <SDL3/SDL.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

// TODO: Check if it can be moved to a utils or some other class.
namespace {
std::string MakeDirectoryAbsolute(const std::string &directory) {
  std::filesystem::path dirPath(directory);
  if (dirPath.is_relative()) {
    dirPath = std::filesystem::current_path() / dirPath;
  }
  return dirPath.lexically_normal().string();
}
} // namespace

std::string ReplayRecorder::EncodeMessage(const std::string &message) {
  std::string encoded;
  encoded.reserve(message.size());
  for (char ch : message) {
    switch (ch) {
    case '\\':
      encoded += "\\\\";
      break;
    case '\n':
      encoded += "\\n";
      break;
    case '\r':
      encoded += "\\r";
      break;
    default:
      encoded += ch;
      break;
    }
  }
  return encoded;
}

std::string ReplayRecorder::DecodeMessage(const std::string &encodedLine) {
  std::string decoded;
  decoded.reserve(encodedLine.size());
  for (size_t i = 0; i < encodedLine.size(); ++i) {
    char ch = encodedLine[i];
    if (ch == '\\' && i + 1 < encodedLine.size()) {
      char next = encodedLine[++i];
      switch (next) {
      case 'n':
        decoded += '\n';
        break;
      case 'r':
        decoded += '\r';
        break;
      case '\\':
        decoded += '\\';
        break;
      default:
        decoded += next;
        break;
      }
    } else {
      decoded += ch;
    }
  }
  return decoded;
}

ReplayRecorder::ReplayRecorder(EntityManager *entityManagerRef,
                               const std::string &defaultDirectory)
    : entityManager(entityManagerRef),
      replayDirectory(MakeDirectoryAbsolute(defaultDirectory)) {
  try {
    std::filesystem::create_directories(replayDirectory);
  } catch (const std::exception &ex) {
    SDL_Log("ReplayRecorder: Failed to create directory %s: %s",
            replayDirectory.c_str(), ex.what());
  }
}

ReplayRecorder::~ReplayRecorder() {
  StopRecording();
  StopPlayback();
}

void ReplayRecorder::SetReplayDirectory(const std::string &directory) {
  std::lock_guard<std::mutex> lock(ioMutex);
  replayDirectory = MakeDirectoryAbsolute(directory);
  try {
    std::filesystem::create_directories(replayDirectory);
  } catch (const std::exception &ex) {
    SDL_Log("ReplayRecorder: Failed to create directory %s: %s",
            replayDirectory.c_str(), ex.what());
  }
}

std::string ReplayRecorder::GenerateTimestampedFilename() const {
  auto now = std::chrono::system_clock::now();
  std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
  std::tm tm {};
#if defined(_WIN32)
  localtime_s(&tm, &nowTime);
#else
  localtime_r(&nowTime, &tm);
#endif
  std::ostringstream oss;
  oss << "replay_";
  oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
  oss << ".txt";
  std::filesystem::path filePath = std::filesystem::path(replayDirectory) / oss.str();
  return filePath.lexically_normal().string();
}

bool ReplayRecorder::OpenRecordingStream(const std::string &filePath) {
  recordingStream.close();
  recordingStream.clear();
  recordingStream.open(filePath, std::ios::out | std::ios::trunc);
  if (!recordingStream.is_open()) {
    SDL_Log("ReplayRecorder: Unable to open recording file %s", filePath.c_str());
    return false;
  }
  recordingStream << "# Replay recording started" << std::endl;
  return true;
}

bool ReplayRecorder::OpenPlaybackStream(const std::string &filePath) {
  playbackStream.close();
  playbackStream.clear();
  playbackStream.open(filePath, std::ios::in);
  if (!playbackStream.is_open()) {
    SDL_Log("ReplayRecorder: Unable to open playback file %s", filePath.c_str());
    return false;
  }
  return true;
}

void ReplayRecorder::CloseRecordingStream() {
  if (recordingStream.is_open()) {
    recordingStream << "# Replay recording stopped" << std::endl;
    recordingStream.flush();
    recordingStream.close();
  }
}

void ReplayRecorder::ClosePlaybackStream() {
  if (playbackStream.is_open()) {
    playbackStream.close();
  }
}

void ReplayRecorder::ResetRecordingStateLocked() {
  CloseRecordingStream();
  recording = false;
  recordingFilePath.clear();
}

void ReplayRecorder::ResetPlaybackStateLocked() {
  ClosePlaybackStream();
  std::queue<std::string> empty;
  std::swap(playbackQueue, empty);
  playing = false;
  playbackFilePath.clear();
}

bool ReplayRecorder::StartRecording() {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (recording) {
    SDL_Log("ReplayRecorder: Recording already active.");
    return true;
  }
  if (playing) {
    SDL_Log("ReplayRecorder: Stopping active playback before recording.");
    ResetPlaybackStateLocked();
  }

  recordingFilePath = GenerateTimestampedFilename();
  try {
    std::filesystem::create_directories(std::filesystem::path(recordingFilePath).parent_path());
  } catch (const std::exception &ex) {
    SDL_Log("ReplayRecorder: Failed to ensure recording directory: %s", ex.what());
  }

  if (!OpenRecordingStream(recordingFilePath)) {
    recordingFilePath.clear();
    return false;
  }

  recording = true;
  SDL_Log("ReplayRecorder: Recording started at %s", recordingFilePath.c_str());
  return true;
}

bool ReplayRecorder::StartRecording(const std::string &filePath) {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (recording) {
    SDL_Log("ReplayRecorder: Recording already active.");
    return true;
  }
  if (playing) {
    SDL_Log("ReplayRecorder: Stopping active playback before recording.");
    ResetPlaybackStateLocked();
  }

  std::filesystem::path desiredPath(filePath);
  if (desiredPath.is_relative()) {
    desiredPath = std::filesystem::path(replayDirectory) / desiredPath;
  }
  try {
    std::filesystem::create_directories(desiredPath.parent_path());
  } catch (const std::exception &ex) {
    SDL_Log("ReplayRecorder: Failed to ensure recording directory: %s", ex.what());
  }

  std::string absolutePath = desiredPath.lexically_normal().string();
  if (!OpenRecordingStream(absolutePath)) {
    return false;
  }

  recordingFilePath = absolutePath;
  recording = true;
  SDL_Log("ReplayRecorder: Recording started at %s", recordingFilePath.c_str());
  return true;
}

void ReplayRecorder::StopRecording() {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (!recording) {
    return;
  }
  lastCompletedRecordingFilePath = recordingFilePath;
  ResetRecordingStateLocked();
  SDL_Log("ReplayRecorder: Recording stopped.");
}

bool ReplayRecorder::StartPlayback(const std::string &filePath) {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (recording) {
    SDL_Log("ReplayRecorder: Stopping active recording before playback.");
    ResetRecordingStateLocked();
  }
  if (playing) {
    SDL_Log("ReplayRecorder: Playback already active.");
    return true;
  }

  std::filesystem::path desiredPath(filePath);
  if (desiredPath.is_relative()) {
    desiredPath = std::filesystem::path(replayDirectory) / desiredPath;
  }

  std::string absolutePath = desiredPath.lexically_normal().string();
  if (!OpenPlaybackStream(absolutePath)) {
    return false;
  }

  playbackFilePath = absolutePath;
  std::queue<std::string> empty;
  std::swap(playbackQueue, empty);
  playing = true;
  SDL_Log("ReplayRecorder: Playback started from %s", playbackFilePath.c_str());
  return true;
}

void ReplayRecorder::StopPlayback() {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (!playing) {
    return;
  }
  ResetPlaybackStateLocked();
  SDL_Log("ReplayRecorder: Playback stopped.");
}

void ReplayRecorder::RecordMessage(const std::string &message) {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (!recording || !recordingStream.is_open()) {
    return;
  }
  recordingStream << EncodeMessage(message) << std::endl;
  if (!recordingStream.good()) {
    SDL_Log("ReplayRecorder: Failed to write message to recording file.");
  }
}

bool ReplayRecorder::TryGetNextPlaybackMessage(std::string &outMessage) {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (playbackQueue.empty()) {
    return false;
  }
  outMessage = playbackQueue.front();
  playbackQueue.pop();
  return true;
}

void ReplayRecorder::Record() {
  std::lock_guard<std::mutex> lock(ioMutex);
  if (recording && recordingStream.is_open()) {
    recordingStream.flush();
  }
}

void ReplayRecorder::Play() {
  std::string line;
  {
    std::lock_guard<std::mutex> lock(ioMutex);
    if (!playing || !playbackStream.is_open()) {
      return;
    }

    while (std::getline(playbackStream, line)) {
      if (line.empty() || line.front() == '#') {
        continue;
      }
      playbackQueue.push(DecodeMessage(line));
      return;
    }

    ResetPlaybackStateLocked();
  }
  SDL_Log("ReplayRecorder: Reached end of playback file.");
}

std::string ReplayRecorder::GetActiveRecordingFile() const {
  std::lock_guard<std::mutex> lock(ioMutex);
  return recordingFilePath;
}

std::string ReplayRecorder::GetActivePlaybackFile() const {
  std::lock_guard<std::mutex> lock(ioMutex);
  return playbackFilePath;
}

std::string ReplayRecorder::GetLastCompletedRecordingFile() const {
  std::lock_guard<std::mutex> lock(ioMutex);
  return lastCompletedRecordingFilePath;
}