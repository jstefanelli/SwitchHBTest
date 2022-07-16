#include "solver.hpp"
#include <vector>

namespace ttt {
	bool CheckWin(const Board& board, Coord* coords, TileState target, Coord& result) {
		int correct = 0;
		Coord* available = nullptr;
		for(int i = 0; i < 3; i++) {
			TileState s = board.Get(coords[i]);
			if (s == target) {
				correct++;
			} else if (s == TileState::Empty) {
				available = coords + i;
			}
		}

		if (correct == 2 && available != nullptr) {
			result = *available;
			return true;
		}

		return false;
	}

	bool CanWin(Board& board, TileState target, Coord& winCoord) {

		Coord checks[3];
		//Test for winning on horizontal
		for(unsigned int y = 0; y < 3; y++) {
			checks[0] = { 0, y};
			checks[1] = { 1, y};
			checks[2] = { 2, y};

			Coord result;
			if (CheckWin(board, checks, target, result)) {
				winCoord = result;
				return true;
			}
		}

		//Test for winning on vertical
		for (unsigned int x = 0; x < 3; x++) {
			checks[0] = { x, 0 };
			checks[1] = { x, 1 };
			checks[2] = { x, 2 };

			Coord result;
			if(CheckWin(board, checks, target, result)) {
				winCoord = result;
				return true;
			}
		}

		//Test for winning on diagonal 0
		checks[0] = { 0, 0 };
		checks[1] = { 1, 1 };
		checks[2] = { 2, 2 };
		Coord result;
		if(CheckWin(board, checks, target, result)) {
			winCoord = result;
			return true;
		}

		//Test for winning on diagonal 1
		checks[0] = { 0, 2 };
		checks[1] = { 1, 1 };
		checks[2] = { 2, 0 };
		if(CheckWin(board, checks, target, result)) {
			winCoord = result;
			return true;
		}

		return false;
	}

	void NextMove(Board& board, int turn, bool solveForCircle) {
		TileState target = solveForCircle ? TileState::Circle : TileState::Cross;
		TileState opposite = solveForCircle ? TileState::Cross : TileState::Circle;
		Coord res;
		if (CanWin(board, target, res) || CanWin(board, opposite, res)) {
			board.Set(res, target);
			return;
		}

		if (board.Get(1, 1) == TileState::Empty) {
			board.Set(1, 1, target);
			return;
		}

		if (board.Get(1, 1) == opposite) {
			if ((board.Get(0, 0) == opposite && board.Get(2, 2) == target) || (board.Get(0, 0) == target && board.Get(2, 2) == opposite)) {
				if (board.Set(2, 0, target)) {
					return;
				}

				if (board.Set(0, 2, target)) {
					return;
				}
			}

			if ((board.Get(2, 0) == opposite && board.Get(0, 2) == target) || (board.Get(2, 0) == target && board.Get(0, 2) == opposite)) {
				if (board.Set(2, 2, target)) {
					return;
				}

				if (board.Set(0, 0, target)) {
					return;
				}
			}
		}

		for(unsigned int y = 0; y < 3; y++) {
			for(unsigned int x = 0; x < 3; x++) {
				if (board.Get(x, y) == target) {
					if (board.Set(x - 1, y, target)) {
						return;
					} else if(board.Set(x + 1, y, target)) {
						return;
					} else if (board.Set(x, y - 1, target)) {
						return;
					} else if (board.Set(x, y + 1, target)) {
						return;
					} else if (board.Set(x + 1, y + 1, target)) {
						return;
					} else if (board.Set(x - 1, y - 1, target)) {
						return;
					} else if (board.Set(x + 1, y - 1, target)) {
						return;
					} else if (board.Set(x - 1, y + 1, target)) {
						return;
					}
				}
			}
		}

		for(unsigned int y = 0; y < 3; y++) {
			for(unsigned int x = 0; x < 3; x++) {
				if (board.Get(x, y) == TileState::Empty) {
					board.Set(x, y, target);
					return;
				}
			}
		}
	}
}