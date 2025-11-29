#include "Board.h"
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <iostream>

Board::Board()
    : whiteToMove(true)
    , castlingRights(0)
    , enPassantSquare(-1)
    , halfMoveClock(0)
    , fullMoveNumber(1)
{
    setStartingPosition();
}

void Board::clearBoard() {
    for (int i = 0; i < 64; i++) {
        board[i] = EMPTY;
    }
}

void Board::setStartingPosition() {
    clearBoard();

    // White pieces
    board[0] = WHITE_ROOK;   board[1] = WHITE_KNIGHT;
    board[2] = WHITE_BISHOP; board[3] = WHITE_QUEEN;
    board[4] = WHITE_KING;   board[5] = WHITE_BISHOP;
    board[6] = WHITE_KNIGHT; board[7] = WHITE_ROOK;
    for (int i = 8; i < 16; i++) {
        board[i] = WHITE_PAWN;
    }

    // Black pieces
    for (int i = 48; i < 56; i++) {
        board[i] = BLACK_PAWN;
    }
    board[56] = BLACK_ROOK;   board[57] = BLACK_KNIGHT;
    board[58] = BLACK_BISHOP; board[59] = BLACK_QUEEN;
    board[60] = BLACK_KING;   board[61] = BLACK_BISHOP;
    board[62] = BLACK_KNIGHT; board[63] = BLACK_ROOK;

    whiteToMove = true;
    castlingRights = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
    enPassantSquare = -1;
    halfMoveClock = 0;
    fullMoveNumber = 1;
}

void Board::setFromFen(const std::string& fen) {
    clearBoard();
    std::istringstream iss(fen);

    // Parse piece placement
    std::string piecePlacement;
    iss >> piecePlacement;

    int square = 56; // Start at a8 (rank 8, file a)
    for (size_t i = 0; i < piecePlacement.length(); i++) {
        char c = piecePlacement[i];

        if (c == '/') {
            square -= 16; // Go to next rank down
        } else if (isdigit(c)) {
            square += (c - '0'); // Skip empty squares
        } else {
            board[square] = charToPiece(c);
            square++;
        }
    }

    // Parse active color
    std::string color;
    iss >> color;
    whiteToMove = (color == "w");

    // Parse castling rights
    std::string castling;
    iss >> castling;
    castlingRights = 0;
    if (castling.find('K') != std::string::npos) castlingRights |= CASTLE_WK;
    if (castling.find('Q') != std::string::npos) castlingRights |= CASTLE_WQ;
    if (castling.find('k') != std::string::npos) castlingRights |= CASTLE_BK;
    if (castling.find('q') != std::string::npos) castlingRights |= CASTLE_BQ;

    // Parse en passant square
    std::string epSquare;
    iss >> epSquare;
    if (epSquare == "-") {
        enPassantSquare = -1;
    } else {
        int file = epSquare[0] - 'a';
        int rank = epSquare[1] - '1';
        enPassantSquare = rank * 8 + file;
    }

    // Parse halfmove clock
    iss >> halfMoveClock;

    // Parse fullmove number
    iss >> fullMoveNumber;
}

std::string Board::toFen() const {
    std::ostringstream oss;

    // Piece placement
    for (int rank = 7; rank >= 0; rank--) {
        int emptyCount = 0;

        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            Piece p = board[square];

            if (p == EMPTY) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    oss << emptyCount;
                    emptyCount = 0;
                }
                oss << pieceToChar(p);
            }
        }

        if (emptyCount > 0) {
            oss << emptyCount;
        }

        if (rank > 0) {
            oss << '/';
        }
    }

    // Active color
    oss << ' ' << (whiteToMove ? 'w' : 'b');

    // Castling rights (validate before outputting to prevent invalid FENs)
    oss << ' ';
    int validCastlingRights = castlingRights;

    // Validate white castling rights (king on e1, rooks on a1/h1)
    if ((validCastlingRights & CASTLE_WK) && (board[4] != WHITE_KING || board[7] != WHITE_ROOK)) {
        validCastlingRights &= ~CASTLE_WK;
    }
    if ((validCastlingRights & CASTLE_WQ) && (board[4] != WHITE_KING || board[0] != WHITE_ROOK)) {
        validCastlingRights &= ~CASTLE_WQ;
    }

    // Validate black castling rights (king on e8, rooks on a8/h8)
    if ((validCastlingRights & CASTLE_BK) && (board[60] != BLACK_KING || board[63] != BLACK_ROOK)) {
        validCastlingRights &= ~CASTLE_BK;
    }
    if ((validCastlingRights & CASTLE_BQ) && (board[60] != BLACK_KING || board[56] != BLACK_ROOK)) {
        validCastlingRights &= ~CASTLE_BQ;
    }

    if (validCastlingRights == 0) {
        oss << '-';
    } else {
        if (validCastlingRights & CASTLE_WK) oss << 'K';
        if (validCastlingRights & CASTLE_WQ) oss << 'Q';
        if (validCastlingRights & CASTLE_BK) oss << 'k';
        if (validCastlingRights & CASTLE_BQ) oss << 'q';
    }

    // En passant square
    oss << ' ';
    if (enPassantSquare == -1) {
        oss << '-';
    } else {
        int file = enPassantSquare % 8;
        int rank = enPassantSquare / 8;
        oss << char('a' + file) << char('1' + rank);
    }

    // Halfmove clock and fullmove number
    oss << ' ' << halfMoveClock << ' ' << fullMoveNumber;

    return oss.str();
}

bool Board::makeMove(const Move& move) {
    if (!move.isValid()) {
        return false;
    }

    // Basic legality check can be added here
    // For now, we assume PGN moves are legal

    Piece movingPiece = board[move.fromSquare];
    Piece capturedPiece = board[move.toSquare];

    // Save move for unmake
    MoveRecord record;
    record.move = move;
    record.capturedPiece = capturedPiece;
    record.oldCastlingRights = castlingRights;
    record.oldEnPassantSquare = enPassantSquare;
    record.oldHalfMoveClock = halfMoveClock;
    moveHistory.push_back(record);

    // Execute move
    board[move.toSquare] = movingPiece;
    board[move.fromSquare] = EMPTY;

    // Handle promotion
    if (move.isPromotion()) {
        if (whiteToMove) {
            switch (move.promotion) {
                case 'q': board[move.toSquare] = WHITE_QUEEN; break;
                case 'r': board[move.toSquare] = WHITE_ROOK; break;
                case 'b': board[move.toSquare] = WHITE_BISHOP; break;
                case 'n': board[move.toSquare] = WHITE_KNIGHT; break;
            }
        } else {
            switch (move.promotion) {
                case 'q': board[move.toSquare] = BLACK_QUEEN; break;
                case 'r': board[move.toSquare] = BLACK_ROOK; break;
                case 'b': board[move.toSquare] = BLACK_BISHOP; break;
                case 'n': board[move.toSquare] = BLACK_KNIGHT; break;
            }
        }
    }

    // Handle en passant capture
    if (enPassantSquare != -1 && move.toSquare == enPassantSquare) {
        if (movingPiece == WHITE_PAWN || movingPiece == BLACK_PAWN) {
            // Capture the pawn
            int capturedPawnSquare = whiteToMove ? (enPassantSquare - 8) : (enPassantSquare + 8);
            board[capturedPawnSquare] = EMPTY;
        }
    }

    // Handle castling
    if (movingPiece == WHITE_KING && move.fromSquare == 4) {
        if (move.toSquare == 6) { // Kingside castle
            board[5] = WHITE_ROOK;
            board[7] = EMPTY;
        } else if (move.toSquare == 2) { // Queenside castle
            board[3] = WHITE_ROOK;
            board[0] = EMPTY;
        }
    } else if (movingPiece == BLACK_KING && move.fromSquare == 60) {
        if (move.toSquare == 62) { // Kingside castle
            board[61] = BLACK_ROOK;
            board[63] = EMPTY;
        } else if (move.toSquare == 58) { // Queenside castle
            board[59] = BLACK_ROOK;
            board[56] = EMPTY;
        }
    }

    // Update castling rights
    updateCastlingRights(move);

    // Update en passant square
    enPassantSquare = -1;
    if (movingPiece == WHITE_PAWN && move.fromSquare / 8 == 1 && move.toSquare / 8 == 3) {
        enPassantSquare = move.fromSquare + 8;
    } else if (movingPiece == BLACK_PAWN && move.fromSquare / 8 == 6 && move.toSquare / 8 == 4) {
        enPassantSquare = move.fromSquare - 8;
    }

    // Update halfmove clock
    if (movingPiece == WHITE_PAWN || movingPiece == BLACK_PAWN || capturedPiece != EMPTY) {
        halfMoveClock = 0;
    } else {
        halfMoveClock++;
    }

    // Update fullmove number
    if (!whiteToMove) {
        fullMoveNumber++;
    }

    // Switch side to move
    whiteToMove = !whiteToMove;

    return true;
}

void Board::unmakeMove() {
    if (moveHistory.empty()) {
        return;
    }

    MoveRecord record = moveHistory.back();
    moveHistory.pop_back();

    // Restore position
    whiteToMove = !whiteToMove;

    Piece movingPiece = board[record.move.toSquare];
    board[record.move.fromSquare] = movingPiece;
    board[record.move.toSquare] = record.capturedPiece;

    // Undo promotion
    if (record.move.isPromotion()) {
        board[record.move.fromSquare] = whiteToMove ? WHITE_PAWN : BLACK_PAWN;
    }

    // Restore castling rights
    castlingRights = record.oldCastlingRights;
    enPassantSquare = record.oldEnPassantSquare;
    halfMoveClock = record.oldHalfMoveClock;

    // Undo castling
    if (movingPiece == WHITE_KING && record.move.fromSquare == 4) {
        if (record.move.toSquare == 6) {
            board[7] = WHITE_ROOK;
            board[5] = EMPTY;
        } else if (record.move.toSquare == 2) {
            board[0] = WHITE_ROOK;
            board[3] = EMPTY;
        }
    } else if (movingPiece == BLACK_KING && record.move.fromSquare == 60) {
        if (record.move.toSquare == 62) {
            board[63] = BLACK_ROOK;
            board[61] = EMPTY;
        } else if (record.move.toSquare == 58) {
            board[56] = BLACK_ROOK;
            board[59] = EMPTY;
        }
    }

    // Restore fullmove number
    if (!whiteToMove) {
        fullMoveNumber--;
    }
}

void Board::updateCastlingRights(const Move& move) {
    // Remove castling rights if king or rook moves
    if (board[move.fromSquare] == WHITE_KING) {
        castlingRights &= ~(CASTLE_WK | CASTLE_WQ);
    } else if (board[move.fromSquare] == BLACK_KING) {
        castlingRights &= ~(CASTLE_BK | CASTLE_BQ);
    }

    // Remove castling rights if rook moves from starting square
    if (move.fromSquare == 0) castlingRights &= ~CASTLE_WQ;
    if (move.fromSquare == 7) castlingRights &= ~CASTLE_WK;
    if (move.fromSquare == 56) castlingRights &= ~CASTLE_BQ;
    if (move.fromSquare == 63) castlingRights &= ~CASTLE_BK;

    // Remove castling rights if rook is captured
    if (move.toSquare == 0) castlingRights &= ~CASTLE_WQ;
    if (move.toSquare == 7) castlingRights &= ~CASTLE_WK;
    if (move.toSquare == 56) castlingRights &= ~CASTLE_BQ;
    if (move.toSquare == 63) castlingRights &= ~CASTLE_BK;
}

bool Board::isMoveLegal(const Move& move) const {
    // Simplified legality check
    // For this application, we trust PGN moves are legal
    return move.isValid();
}

Piece Board::getPieceAt(int square) const {
    if (square < 0 || square > 63) {
        return EMPTY;
    }
    return board[square];
}

bool Board::isWhitePiece(Piece p) {
    return p >= WHITE_PAWN && p <= WHITE_KING;
}

bool Board::isBlackPiece(Piece p) {
    return p >= BLACK_PAWN && p <= BLACK_KING;
}

char Board::pieceToChar(Piece p) {
    switch (p) {
        case WHITE_PAWN: return 'P';
        case WHITE_KNIGHT: return 'N';
        case WHITE_BISHOP: return 'B';
        case WHITE_ROOK: return 'R';
        case WHITE_QUEEN: return 'Q';
        case WHITE_KING: return 'K';
        case BLACK_PAWN: return 'p';
        case BLACK_KNIGHT: return 'n';
        case BLACK_BISHOP: return 'b';
        case BLACK_ROOK: return 'r';
        case BLACK_QUEEN: return 'q';
        case BLACK_KING: return 'k';
        default: return ' ';
    }
}

Piece Board::charToPiece(char c) {
    switch (c) {
        case 'P': return WHITE_PAWN;
        case 'N': return WHITE_KNIGHT;
        case 'B': return WHITE_BISHOP;
        case 'R': return WHITE_ROOK;
        case 'Q': return WHITE_QUEEN;
        case 'K': return WHITE_KING;
        case 'p': return BLACK_PAWN;
        case 'n': return BLACK_KNIGHT;
        case 'b': return BLACK_BISHOP;
        case 'r': return BLACK_ROOK;
        case 'q': return BLACK_QUEEN;
        case 'k': return BLACK_KING;
        default: return EMPTY;
    }
}

bool Board::isSquareAttacked(int square, bool byWhite) const {
    // Simplified - not fully implemented for this application
    // Would need full attack generation
    return false;
}

bool Board::isInCheck(bool white) const {
    // Simplified - not fully implemented for this application
    return false;
}

bool Board::wouldBeInCheck(const Move& move, bool white) const {
    // Simplified - not fully implemented for this application
    return false;
}
