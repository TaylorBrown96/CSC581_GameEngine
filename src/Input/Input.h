#pragma once
#include <SDL3/SDL.h>

#include <unordered_map>
#include <string>
#include <vector>

/**
 * InputManager - Handles both raw keyboard input and action mapping
 * 
 * This class provides both low-level keyboard input handling and high-level
 * action mapping. Actions are semantic names that can be mapped to multiple keys,
 * making it easier to handle different input configurations and send clean
 * action data to the server.
 */

typedef struct ActionTrigger {
  bool isKeyChord;
  std::vector<SDL_Scancode> keys;
} ActionTrigger;

class InputManager {
 private:
  const bool *keyboardState;
  std::unordered_map<SDL_Scancode, bool> previousKeyState;

  // Action mapping functionality
  std::unordered_map<std::string, ActionTrigger> actionToKeys;
  std::unordered_map<SDL_Scancode, std::string> keyToAction;

 public:
  InputManager();
  void Update();

  // Raw keyboard input
  bool IsKeyPressed(SDL_Scancode scancode) const;
  bool IsKeyJustPressed(SDL_Scancode scancode) const;
  bool IsKeyJustReleased(SDL_Scancode scancode) const;

  // Action mapping
  void AddAction(const std::string& actionName, SDL_Scancode key);
  void AddAction(const std::string& actionName, const std::vector<SDL_Scancode>& keys);
  void AddChordAction(const std::string& actionName, const std::vector<SDL_Scancode>& keys);
  bool IsActionActive(const std::string& actionName) const;
  std::vector<std::string> GetActiveActions() const;

  void SetupDefaultActions();
};
