#include "game.hpp"
#include <algorithm>
#include <random>

// ──────────────────────────────────────────────
//  Piece shape data: 4 rotations × 7 piece types
//  Each cell is a (row, col) offset from the
//  piece's top-left bounding-box corner.
// ──────────────────────────────────────────────

using Cells = std::vector<std::pair<int, int>>;

static const std::array<std::array<Cells, 4>, PIECE_COUNT> pieceData = {{

    // I ─────────────────────────────────────────
    {{
        {{ {1,0},{1,1},{1,2},{1,3} }},        // horizontal top
        {{ {0,2},{1,2},{2,2},{3,2} }},        // vertical right
        {{ {2,0},{2,1},{2,2},{2,3} }},        // horizontal bottom
        {{ {0,1},{1,1},{2,1},{3,1} }},        // vertical left
    }},

    // O ─────────────────────────────────────────
    {{
        {{ {0,0},{0,1},{1,0},{1,1} }},
        {{ {0,0},{0,1},{1,0},{1,1} }},
        {{ {0,0},{0,1},{1,0},{1,1} }},
        {{ {0,0},{0,1},{1,0},{1,1} }},
    }},

    // T ─────────────────────────────────────────
    {{
        {{ {0,1},{1,0},{1,1},{1,2} }},
        {{ {0,1},{1,1},{1,2},{2,1} }},
        {{ {1,0},{1,1},{1,2},{2,1} }},
        {{ {0,1},{1,0},{1,1},{2,1} }},
    }},

    // S ─────────────────────────────────────────
    {{
        {{ {0,1},{0,2},{1,0},{1,1} }},
        {{ {0,1},{1,1},{1,2},{2,2} }},
        {{ {1,1},{1,2},{2,0},{2,1} }},
        {{ {0,0},{1,0},{1,1},{2,1} }},
    }},

    // Z ─────────────────────────────────────────
    {{
        {{ {0,0},{0,1},{1,1},{1,2} }},
        {{ {0,2},{1,1},{1,2},{2,1} }},
        {{ {1,0},{1,1},{2,1},{2,2} }},
        {{ {0,1},{1,0},{1,1},{2,0} }},
    }},

    // J ─────────────────────────────────────────
    {{
        {{ {0,0},{1,0},{1,1},{1,2} }},
        {{ {0,1},{0,2},{1,1},{2,1} }},
        {{ {1,0},{1,1},{1,2},{2,2} }},
        {{ {0,1},{1,1},{2,0},{2,1} }},
    }},

    // L ─────────────────────────────────────────
    {{
        {{ {0,2},{1,0},{1,1},{1,2} }},
        {{ {0,1},{1,1},{2,1},{2,2} }},
        {{ {1,0},{1,1},{1,2},{2,0} }},
        {{ {0,0},{0,1},{1,1},{2,1} }},
    }},
}};

const std::vector<std::pair<int, int>>& pieceCells(PieceType type, int rotation) {
    return pieceData[static_cast<int>(type)][rotation % 4];
}

int pieceWidth(PieceType type, int rotation) {
    int w = 0;
    for (auto& c : pieceCells(type, rotation)) w = std::max(w, c.second + 1);
    return w;
}

int pieceHeight(PieceType type, int rotation) {
    int h = 0;
    for (auto& c : pieceCells(type, rotation)) h = std::max(h, c.first + 1);
    return h;
}

// ──────────────────────────────────────────────
//  RNG helpers
// ──────────────────────────────────────────────

static std::mt19937& rng() {
    static std::mt19937 r(std::random_device{}());
    return r;
}

static PieceType randomPiece() {
    return static_cast<PieceType>(
        std::uniform_int_distribution<>(0, PIECE_COUNT - 1)(rng()));
}

// ──────────────────────────────────────────────
//  Game implementation
// ──────────────────────────────────────────────

Game::Game() {
    pieceType_ = randomPiece();
    nextType_ = randomPiece();
    spawnPiece();
}

bool Game::canPlace(PieceType type, int rot, int x, int y) const {
    for (auto& c : pieceCells(type, rot)) {
        int ny = y + c.first;
        int nx = x + c.second;
        if (nx < 0 || nx >= BOARD_WIDTH)  return false;
        if (ny >= BOARD_HEIGHT)           return false;
        if (ny < 0) continue;            // above the board is fine
        if (board_[ny][nx] != 0)         return false;
    }
    return true;
}

void Game::spawnPiece() {
    pieceType_ = nextType_;
    nextType_ = randomPiece();
    pieceRot_  = 0;
    pieceX_    = (BOARD_WIDTH - pieceWidth(pieceType_, 0)) / 2;
    pieceY_    = 0;

    if (!canPlace(pieceType_, pieceRot_, pieceX_, pieceY_)) {
        gameOver_ = true;
    }
}

void Game::moveLeft() {
    if (paused_ || gameOver_) return;
    if (canPlace(pieceType_, pieceRot_, pieceX_ - 1, pieceY_))
        pieceX_--;
}

void Game::moveRight() {
    if (paused_ || gameOver_) return;
    if (canPlace(pieceType_, pieceRot_, pieceX_ + 1, pieceY_))
        pieceX_++;
}

void Game::moveDown() {
    if (paused_ || gameOver_) return;
    if (canPlace(pieceType_, pieceRot_, pieceX_, pieceY_ + 1)) {
        pieceY_++;
        score_ += 1;            // soft-drop bonus
    }
}

void Game::hardDrop() {
    if (paused_ || gameOver_) return;
    int dropped = 0;
    while (canPlace(pieceType_, pieceRot_, pieceX_, pieceY_ + 1)) {
        pieceY_++;
        dropped++;
    }
    score_ += dropped * 2;      // hard-drop bonus
    lockPiece();
}

void Game::rotateCW() {
    if (paused_ || gameOver_) return;
    int newRot = (pieceRot_ + 1) % 4;

    // Try: identity, left-1, right-1, left-2, right-2, up-1
    static const int kicks[][2] = {
        {0,0}, {-1,0}, {1,0}, {-2,0}, {2,0}, {0,-1}
    };
    for (auto& k : kicks) {
        if (canPlace(pieceType_, newRot, pieceX_ + k[0], pieceY_ + k[1])) {
            pieceRot_ = newRot;
            pieceX_  += k[0];
            pieceY_  += k[1];
            return;
        }
    }
}

void Game::tick() {
    if (paused_ || gameOver_) return;

    if (canPlace(pieceType_, pieceRot_, pieceX_, pieceY_ + 1)) {
        pieceY_++;
    } else {
        lockPiece();
    }
}

void Game::lockPiece() {
    for (auto& c : pieceCells(pieceType_, pieceRot_)) {
        int ny = pieceY_ + c.first;
        int nx = pieceX_ + c.second;
        if (ny >= 0 && ny < BOARD_HEIGHT && nx >= 0 && nx < BOARD_WIDTH)
            board_[ny][nx] = static_cast<int>(pieceType_) + 1;
    }
    clearLines();
    spawnPiece();
}

void Game::clearLines() {
    int cleared = 0;
    for (int r = BOARD_HEIGHT - 1; r >= 0; --r) {
        bool full = true;
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            if (board_[r][c] == 0) { full = false; break; }
        }
        if (full) {
            cleared++;
            // shift everything above down by one
            for (int rr = r; rr > 0; --rr)
                for (int c = 0; c < BOARD_WIDTH; ++c)
                    board_[rr][c] = board_[rr - 1][c];
            // clear top row
            for (int c = 0; c < BOARD_WIDTH; ++c)
                board_[0][c] = 0;
            r++; // re-check this row
        }
    }

    if (cleared == 0) return;

    static const int points[] = {0, 100, 300, 500, 800};
    score_ += points[cleared] * level_;
    lines_ += cleared;
    updateLevel();
}

void Game::updateLevel() {
    level_ = 1 + lines_ / 10;
}

int Game::dropIntervalMs() const {
    return std::max(50, 800 - (level_ - 1) * 70);
}

int Game::ghostY() const {
    int gy = pieceY_;
    while (canPlace(pieceType_, pieceRot_, pieceX_, gy + 1))
        gy++;
    return gy;
}

void Game::togglePause() {
    if (gameOver_) return;
    paused_ = !paused_;
}
