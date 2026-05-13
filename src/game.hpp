#pragma once
#include <array>
#include <utility>
#include <vector>

enum class PieceType { I, O, T, S, Z, J, L };
constexpr int PIECE_COUNT = 7;

// Returns the cell offsets (row, col) for a piece type and rotation.
// Bounding box varies per piece/rotation.
const std::vector<std::pair<int, int>>& pieceCells(PieceType type, int rotation);

// Returns the bounding-box width of a piece in a given rotation.
int pieceWidth(PieceType type, int rotation);
int pieceHeight(PieceType type, int rotation);

class Game {
public:
    static constexpr int BOARD_WIDTH = 10;
    static constexpr int BOARD_HEIGHT = 22;   // 2 hidden rows + 20 visible
    static constexpr int VISIBLE_ROW = 2;      // first visible row index

    Game();

    // --- actions ---
    void moveLeft();
    void moveRight();
    void moveDown();    // soft drop (adds 1 point per cell)
    void hardDrop();    // instant drop (adds 2 points per cell)
    void rotateCW();    // clockwise rotation with basic wall-kick
    void tick();        // gravity tick — called by the timer

    void togglePause();

    // --- queries ---
    bool isGameOver() const { return gameOver_; }
    bool isPaused() const    { return paused_; }

    PieceType currentPiece()  const { return pieceType_; }
    PieceType nextPiece()     const { return nextType_; }
    int pieceX() const         { return pieceX_; }
    int pieceY() const         { return pieceY_; }
    int pieceRot() const       { return pieceRot_; }

    int ghostY() const;  // y-coordinate of hard-drop preview

    int score() const { return score_; }
    int level() const { return level_; }
    int lines() const { return lines_; }
    int dropIntervalMs() const;

    // Returns the board cell. 0 = empty, 1..7 = (PieceType+1).
    int cell(int row, int col) const { return board_[row][col]; }

    // Can a piece be placed at (x, y) with given rotation?
    bool canPlace(PieceType type, int rot, int x, int y) const;

private:
    void spawnPiece();
    void lockPiece();
    void clearLines();
    void updateLevel();

    std::array<std::array<int, BOARD_WIDTH>, BOARD_HEIGHT> board_{};

    PieceType pieceType_;
    PieceType nextType_;
    int pieceX_, pieceY_;
    int pieceRot_;

    int score_ = 0;
    int level_ = 1;
    int lines_ = 0;
    bool gameOver_ = false;
    bool paused_ = false;
};
