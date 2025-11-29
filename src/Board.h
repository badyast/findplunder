#ifndef BOARD_H
#define BOARD_H

#include "Move.h"
#include <string>
#include <vector>

// Piece types
enum Piece {
    EMPTY = 0,
    WHITE_PAWN = 1,
    WHITE_KNIGHT = 2,
    WHITE_BISHOP = 3,
    WHITE_ROOK = 4,
    WHITE_QUEEN = 5,
    WHITE_KING = 6,
    BLACK_PAWN = 7,
    BLACK_KNIGHT = 8,
    BLACK_BISHOP = 9,
    BLACK_ROOK = 10,
    BLACK_QUEEN = 11,
    BLACK_KING = 12
};

// Castling rights bits
const int CASTLE_WK = 1;  // White kingside
const int CASTLE_WQ = 2;  // White queenside
const int CASTLE_BK = 4;  // Black kingside
const int CASTLE_BQ = 8;  // Black queenside

class Board {
public:
    Board();

    void setFromFen(const std::string& fen);
    std::string toFen() const;

    bool makeMove(const Move& move);
    void unmakeMove();  // Undo last move

    bool isMoveLegal(const Move& move) const;

    Piece getPieceAt(int square) const;
    bool isWhiteToMove() const { return whiteToMove; }

    // Helper functions
    static bool isWhitePiece(Piece p);
    static bool isBlackPiece(Piece p);
    static char pieceToChar(Piece p);
    static Piece charToPiece(char c);

private:
    // Mailbox representation (0-63)
    Piece board[64];

    // Game state
    bool whiteToMove;
    int castlingRights;  // Bitmask
    int enPassantSquare; // -1 if none
    int halfMoveClock;
    int fullMoveNumber;

    // Move history for unmake
    struct MoveRecord {
        Move move;
        Piece capturedPiece;
        int oldCastlingRights;
        int oldEnPassantSquare;
        int oldHalfMoveClock;
    };
    std::vector<MoveRecord> moveHistory;

    // Helper functions
    void clearBoard();
    void setStartingPosition();

    bool isSquareAttacked(int square, bool byWhite) const;
    bool isInCheck(bool white) const;
    bool wouldBeInCheck(const Move& move, bool white) const;

    void updateCastlingRights(const Move& move);
};

#endif // BOARD_H
