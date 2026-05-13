#include "game.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>

using namespace ftxui;

// ──────────────────────────────────────────────
//  Color mapping
// ──────────────────────────────────────────────

static Color pieceColor(PieceType type) {
    switch (type) {
        case PieceType::I: return Color::Cyan;
        case PieceType::O: return Color::Yellow;
        case PieceType::T: return Color::Magenta;
        case PieceType::S: return Color::Green;
        case PieceType::Z: return Color::Red;
        case PieceType::J: return Color::Blue;
        case PieceType::L: return Color(255, 165, 0); // orange
    }
    return Color::White;
}

// ──────────────────────────────────────────────
//  Rendering helpers
// ──────────────────────────────────────────────

static bool isActiveCell(const Game& g, int r, int c) {
    for (auto& off : pieceCells(g.currentPiece(), g.pieceRot())) {
        if (g.pieceY() + off.first == r && g.pieceX() + off.second == c)
            return true;
    }
    return false;
}

static bool isGhostCell(const Game& g, int r, int c) {
    int gy = g.ghostY();
    for (auto& off : pieceCells(g.currentPiece(), g.pieceRot())) {
        if (gy + off.first == r && g.pieceX() + off.second == c)
            return true;
    }
    return false;
}

static Element renderBoard(const Game& g) {
    Elements rows;
    for (int r = Game::VISIBLE_ROW; r < Game::BOARD_HEIGHT; ++r) {
        Elements cells;
        for (int c = 0; c < Game::BOARD_WIDTH; ++c) {
            int val = g.cell(r, c);
            bool active = isActiveCell(g, r, c);
            bool ghost  = isGhostCell(g, r, c) && !active;

            if (val != 0 || active) {
                Color clr = pieceColor(val != 0
                    ? static_cast<PieceType>(val - 1)
                    : g.currentPiece());
                cells.push_back(text("██") | bgcolor(clr) | color(clr));
            } else if (ghost) {
                Color clr = pieceColor(g.currentPiece());
                cells.push_back(text("░░") | color(clr) | dim);
            } else {
                cells.push_back(text(" ·") | color(Color::GrayDark));
            }
        }
        rows.push_back(hbox(std::move(cells)));
    }
    return vbox(std::move(rows));
}

static Element renderNextPiece(const Game& g) {
    auto& cells = pieceCells(g.nextPiece(), 0);
    int w = pieceWidth(g.nextPiece(), 0);
    int h = pieceHeight(g.nextPiece(), 0);
    Color nxtColor = pieceColor(g.nextPiece());

    Elements rows;
    for (int r = 0; r < h; ++r) {
        Elements row;
        for (int c = 0; c < w; ++c) {
            bool filled = false;
            for (auto& off : cells)
                if (off.first == r && off.second == c) { filled = true; break; }
            if (filled)
                row.push_back(text("██") | bgcolor(nxtColor) | color(nxtColor));
            else
                row.push_back(text("  "));
        }
        rows.push_back(hbox(std::move(row)));
    }
    return vbox({
        text(" NEXT ") | bold | center,
        separator(),
        vbox(std::move(rows)) | center,
    });
}

static Element renderStats(const Game& g) {
    return vbox({
        text(" SCORE ") | bold | center,
        text(" " + std::to_string(g.score())) | center,
        separator(),
        text(" LEVEL ") | bold | center,
        text(" " + std::to_string(g.level())) | center,
        separator(),
        text(" LINES ") | bold | center,
        text(" " + std::to_string(g.lines())) | center,
    });
}

static Element renderControls() {
    return vbox({
        text(" CONTROLS ") | bold | center,
        separator(),
        text(" \xE2\x86\x90 \xE2\x86\x92  Move  "),
        text(" \xE2\x86\x91     Rotate"),
        text(" \xE2\x86\x93     SoftDrop"),
        text(" Space  HardDrop"),
        text(" P      Pause"),
        text(" Q / Esc  Quit"),
    });
}

static Element renderGame(const Game& g) {
    auto left = renderBoard(g) | border;
    auto next = renderNextPiece(g) | border;
    auto stats = renderStats(g) | border;
    auto controls = renderControls() | border;

    auto right = vbox({next, text(""), stats, text(""), controls});

    auto main = hbox({left, text("  "), right});

    if (g.isPaused() && !g.isGameOver()) {
        main = hbox({
            left,
            text("  "),
            vbox({
                text("") | size(HEIGHT, EQUAL, 4),
                text("  PAUSED  ") | bold | center | border | color(Color::Yellow),
                text("") | size(HEIGHT, EQUAL, 2),
                right,
            }),
        });
    }

    if (g.isGameOver()) {
        main = hbox({
            left,
            text("  "),
            vbox({
                text("") | size(HEIGHT, EQUAL, 4),
                text("  GAME OVER  ") | bold | center | border | color(Color::Red) | bgcolor(Color::Black),
                text("  Press Q to quit  ") | center | dim,
                text("") | size(HEIGHT, EQUAL, 2),
                right,
            }),
        });
    }

    return main | center;
}

// ──────────────────────────────────────────────
//  Main
// ──────────────────────────────────────────────

int main() {
    Game game;
    auto screen = ScreenInteractive::Fullscreen();

    // Gravity timer thread
    std::atomic<bool> running{true};
    std::thread timer([&]() {
        using namespace std::chrono;
        while (running.load(std::memory_order_relaxed)) {
            auto ms = game.dropIntervalMs();
            std::this_thread::sleep_for(milliseconds(ms));
            if (running.load(std::memory_order_relaxed))
                screen.PostEvent(Event::Custom);
        }
    });

    // Main component
    auto component = Renderer([&] { return renderGame(game); });

    component |= CatchEvent([&](Event event) -> bool {
        // Custom tick event from timer
        if (event == Event::Custom) {
            game.tick();
            return true;
        }

        // Quit
        if (event == Event::Character('q') || event == Event::Character('Q')
            || event == Event::Escape) {
            running.store(false, std::memory_order_relaxed);
            screen.Exit();
            return true;
        }

        // Pause
        if (event == Event::Character('p') || event == Event::Character('P')) {
            game.togglePause();
            return true;
        }

        // Skip input if paused or game over
        if (game.isPaused() || game.isGameOver()) return false;

        // Movement
        if (event == Event::ArrowLeft)  { game.moveLeft();  return true; }
        if (event == Event::ArrowRight) { game.moveRight(); return true; }
        if (event == Event::ArrowDown)  { game.moveDown();  return true; }
        if (event == Event::ArrowUp)    { game.rotateCW();  return true; }

        // Hard drop
        if (event == Event::Character(' ')) { game.hardDrop(); return true; }

        return false;
    });

    screen.Loop(component);

    running.store(false, std::memory_order_relaxed);
    if (timer.joinable()) timer.join();

    return 0;
}
