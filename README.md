# Tetris TUI

A terminal-based Tetris game written in C++17, powered by [FTXUI](https://github.com/ArthurSonzogni/FTXUI).

![Demonstration animation](example.gif)

## Features

- All 7 standard pieces (I, O, T, S, Z, J, L) with 4 rotations each
- Wall-kick rotation system
- Ghost piece (hard-drop preview)
- Score, level, and line counter — level speeds up as you clear more lines
- Pause / resume
- Unicode block-drawing graphics

## Controls

| Key            | Action           |
| -------------- | ---------------- |
| ← →           | Move left/right  |
| ↑              | Rotate clockwise |
| ↓              | Soft drop (+1pt) |
| Space          | Hard drop (+2pt/cell) |
| P              | Pause            |
| Q / Esc        | Quit             |

## Scoring

| Lines cleared | Points (× level) |
| ------------- | ---------------- |
| 1             | 100              |
| 2             | 300              |
| 3             | 500              |
| 4 (Tetris)    | 800              |

Level increases every 10 lines cleared. Drop speed increases from 800ms down to a minimum of 50ms.

## Build

### Requirements

| Tool       | Minimum Version |
| ---------- | --------------- |
| CMake      | 3.20            |
| C++ compiler | C++17 (GCC 8+, Clang 7+, MSVC 2019 16.8+) |

FTXUI is fetched automatically via CMake FetchContent — no manual dependency installation needed.

### Compile & Run

**macOS / Linux**

```bash
# 1. Configure (generate build system)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compile
cmake --build build

# 3. Run
./build/tetris
```

**Windows (Developer Command Prompt / PowerShell)**

```powershell
# 1. Configure
cmake -B build

# 2. Compile
cmake --build build --config Release

# 3. Run
.\build\Release\tetris.exe
```

> **Note:** On Windows, MSVC places the executable in a `Release` (or `Debug`) subdirectory by default. Use `--config Release` with `cmake --build` and run from the matching folder.

### Build Options

```bash
# Debug build with Address Sanitizer (GCC/Clang)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build
```

## Project Structure

```
show/
├── CMakeLists.txt      # Build configuration
├── src/
│   ├── game.hpp        # Game logic header
│   ├── game.cpp        # Piece data, collision, line clearing
│   └── main.cpp        # FTXUI rendering and input handling
└── README.md
```
