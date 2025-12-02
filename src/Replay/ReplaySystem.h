#pragma once

#include "Events/EventSystem.h"
#include "Events/EventTypes.h"
#include "Events/ReplayEvent.h"
#include "Entities/Entity.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>

class ReplayRecorder {
  EntityManager *entityManager = nullptr;
  std::string replayDirectory;
  std::string recordingFilePath;
  std::string playbackFilePath;
  std::string lastCompletedRecordingFilePath;
  std::ofstream recordingStream;
  std::ifstream playbackStream;
  std::queue<std::string> playbackQueue;
  mutable std::mutex ioMutex;

  std::string GenerateTimestampedFilename() const;
  bool OpenRecordingStream(const std::string &filePath);
  bool OpenPlaybackStream(const std::string &filePath);
  void CloseRecordingStream();
  void ClosePlaybackStream();
  void ResetRecordingStateLocked();
  void ResetPlaybackStateLocked();
  static std::string EncodeMessage(const std::string &message);
  static std::string DecodeMessage(const std::string &encodedLine);

public:
  bool recording = false;
  bool playing = false;

  explicit ReplayRecorder(EntityManager *entityManagerRef,
                          const std::string &defaultDirectory = "Replays");
  ~ReplayRecorder();

  bool StartRecording();
  bool StartRecording(const std::string &filePath);
  void StopRecording();

  bool StartPlayback(const std::string &filePath);
  void StopPlayback();

  void RecordMessage(const std::string &message);
  bool TryGetNextPlaybackMessage(std::string &outMessage);

  void Record();
  void Play();

  bool IsRecording() const { return recording; }
  bool IsPlaying() const { return playing; }
  std::string GetActiveRecordingFile() const;
  std::string GetActivePlaybackFile() const;
  std::string GetLastCompletedRecordingFile() const;
  void SetReplayDirectory(const std::string &directory);
};

