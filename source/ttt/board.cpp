#include "board.hpp"

namespace ttt {
    Board::Board() {
        Reset();
    }

    void Board::Reset() {
        for(int x = 0; x < 3; x++) {
            for(int y = 0; y < 3; y++) {
                tiles[y][x] = TileState::Empty;
            }
        }
        state = BoardState::Regular;
    }

    TileState Board::Get(unsigned int x, unsigned int y) const {
        if (x >= 3 || x < 0 || y >= 3 || y < 0) {
            return TileState::Invalid;
        }

        return tiles[y][x];
    }

    TileState Board::Get(Coord c) const {
        return Get(c.x, c.y);
    }

    bool Board::Set(unsigned int x, unsigned int y, TileState value) {
        if (x >= 3 || x < 0 || y >= 3 || y < 0) {
            return false;
        }

        if (tiles[y][x] != TileState::Empty) {
            return false;
        }

        tiles[y][x] = value;

        Update();
        return true;
    }

    bool Board::Set(Coord c, TileState value) {
        return Set(c.x, c.y, value);
    }

    BoardState Board::GetState() {
        return state;
    }

    void Board::Update() {
        for(int y = 0; y < 3; y++) {
            if((tiles[y][0] == TileState::Circle || tiles[y][0] == TileState::Cross) && tiles[y][0] == tiles[y][1] && tiles[y][1] == tiles[y][2]) {
                state = tiles[y][0] == TileState::Circle ? BoardState::CircleWin : BoardState::CrossWin;
                return;
            }
        }

        for(int x = 0; x < 3; x++) {
            if((tiles[0][x] == TileState::Circle || tiles[0][x] == TileState::Cross) && tiles[0][x] == tiles[1][x] && tiles[1][x] == tiles[2][x]) {
                state = tiles[0][x] == TileState::Circle ? BoardState::CircleWin : BoardState::CrossWin;
                return;
            }
        }

        if ((tiles[0][0] == TileState::Circle || tiles[0][0] == TileState::Cross) && tiles[1][1] == tiles[0][0] && tiles[2][2] == tiles[0][0]) {
            state = tiles[0][0] == TileState::Circle ? BoardState::CircleWin : BoardState::CrossWin;
            return;
        }

        
        if ((tiles[0][2] == TileState::Circle || tiles[0][2] == TileState::Cross) && tiles[1][1] == tiles[0][2] && tiles[2][0] == tiles[0][2]) {
            state = tiles[0][2] == TileState::Circle ? BoardState::CircleWin : BoardState::CrossWin;
            return;
        }

        for(int x = 0; x < 3; x++) {
            for(int y = 0; y < 3; y++) {
                if (tiles[y][x] == TileState::Empty) {
                    state = BoardState::Regular;
                    return;
                }
            }
        }

        state = BoardState::Tied;
    }
}