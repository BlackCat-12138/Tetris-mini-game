# Tetris TUI

A terminal-based Tetris game written in C++17, powered by [FTXUI](https://github.com/ArthurSonzogni/FTXUI).

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

Requirements:
- CMake ≥ 3.20
- A C++17 compiler (GCC, Clang, MSVC)

```bash
cmake -B build
cmake --build build
./build/tetris
```

FTXUI is fetched automatically via CMake FetchContent — no manual dependency installation needed.

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
