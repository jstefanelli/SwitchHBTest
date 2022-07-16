#pragma once
#include "board.hpp"

namespace ttt {
    void NextMove(Board& board, int turn, bool solveForCircle = false);
}