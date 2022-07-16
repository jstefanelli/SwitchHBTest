#pragma once
#include <iterator>

namespace ttt {
	struct Coord {
		unsigned int x, y;

		void Normalize() {
			x %= 3;
			y %= 3;
		}
	};

	enum class TileState {
		Invalid,
		Empty,
		Circle,
		Cross
	};

	enum class BoardState {
		Regular,
		Tied,
		CircleWin,
		CrossWin
	};

	class Board {
	protected:
		TileState tiles[3][3];
		BoardState state;

		void Update();
	public:

		Board();
		TileState Get(unsigned int x, unsigned int y) const;
		TileState Get(Coord c) const;
		bool Set(unsigned int x, unsigned int y, TileState value);
		bool Set(Coord c, TileState value);
		void Reset();
		
		BoardState GetState();
	};

}