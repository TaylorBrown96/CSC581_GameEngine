#pragma once
#include <SDL3/SDL.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
/**
 * InputManager - Handles both raw keyboard input and action mapping
 * 
 * This class provides both low-level keyboard input handling and high-level
 * action mapping. Actions are semantic names that can be mapped to multiple keys,
 * making it easier to handle different input configurations and send clean
 * action data to the server.
 * 
 * Example usage:
 *   inputManager->AddAction("MOVE_UP", SDL_SCANCODE_W);
 *   bool isMoving = inputManager->IsActionActive("MOVE_UP");
 *   auto actions = inputManager->GetActiveActions();
 */
typedef struct Action {

  std::string name;
  SDL_Scancode key;
  uint16_t id;

} Action;

class InputManager {
 private:
  const bool *keyboardState;  // SDL3 returns const bool*, not const Uint8*
  std::unordered_map<SDL_Scancode, bool> previousKeyState;
  
  // Action mapping functionality
  std::unordered_map<std::string, std::vector<SDL_Scancode>> actionToKeys;
    // std::unordered_map<std::string, std::vector<SDL_Scancode>> actionToKeys;
  std::vector<std::string> allActions;

  std::unordered_map<SDL_Scancode, std::string> keyToAction;

 public:
  InputManager();
  void Update();

  // Raw keyboard input methods
  bool IsKeyPressed(SDL_Scancode scancode) const;
  bool IsKeyJustPressed(SDL_Scancode scancode) const;
  bool IsKeyJustReleased(SDL_Scancode scancode) const;
  
  // Action mapping methods
  void AddAction(const std::string& actionName, SDL_Scancode key);
  void AddAction(const std::string& actionName, const std::vector<SDL_Scancode>& keys);
  bool IsActionActive(const std::string& actionName) const;
  bool IsActionActive(int actionId) const;

  std::vector<std::string> GetActiveActions() const;
  std::vector<int> GetActiveActionIndices();
  std::vector<std::string> GetAllActions();
  void SetAllActions(std::vector<std::string> actions);
  // Setup default actions
  void SetupDefaultActions();

  std::mutex actionsMutex;
};
