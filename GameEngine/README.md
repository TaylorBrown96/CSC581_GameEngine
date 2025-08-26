# GameEngine – Skelly Idle Demo

A simple SDL3 project that animates a sprite sheet.

---

## Setup

1. **Download this repo**

   ```bash
   git clone https://github.com/yourusername/GameEngine.git
   cd GameEngine
   ```

2. **Clone SDL3 into the vendored folder**

   ```bash
   git clone https://github.com/libsdl-org/SDL.git vendored/SDL
   ```

---

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## Run

From the `build` folder:

```bash
./SkellyIdle   # Linux / macOS
SkellyIdle.exe # Windows
```

---

## Notes

- Make sure `assets/skelly_idle.bmp` exists in the correct location.  
- Resize the window and use keyboard controls (arrows, space, escape) to interact with the animation.
