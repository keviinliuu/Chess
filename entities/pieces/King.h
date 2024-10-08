#ifndef KING_H
#define KING_H

#include "../Piece.h"
#include "../Board.h"

class King : public Piece {
public:
    King(Colour colour);
    King(const King& other);
    std::shared_ptr<Piece> clone() const override;
    const char getType() const override;
    bool isValidMove(const Board& board, int startRow, int startCol, int endRow, int endCol) const override;
    bool canCastle() const { return !hasMoved(); }
    bool isInCheck(const Board& board, int kingRow, int kingCol) const;
};

#endif
