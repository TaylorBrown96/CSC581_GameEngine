# GameEngine – Skelly Idle Demo

## Setup

1. **Download this repo**

   ```bash
   git clone https://github.com/TaylorBrown96/CSC581_GameEngine.git
   cd GameEngine
   ```

2. **Clone SDL3 into the vendored folder**

   ```bash
   git clone https://github.com/libsdl-org/SDL.git vendored/SDL
   ```

---

## Build

```bash
cmake -S . -B build
cmake --build build
```

---

## Run

From the `build` folder:

```bash
./build/Debug/SkellyIdle   # Linux / macOS
.\build\Debug\GameEngine.exe # Windows
```

---

## Notes

- Make sure `assets/skelly_idle.bmp` exists in the correct location.  

### Controls
```
UP: speed up animation
DOWN: slow down animation
RIGHT: increase size
LEFT: decrease size
SPACE: pause/resume
ESC or window close: quit
```
