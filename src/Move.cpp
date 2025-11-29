#include "Move.h"

Move::Move()
    : fromSquare(-1)
    , toSquare(-1)
    , promotion('\0')
{
}

Move::Move(int from, int to, char promo)
    : fromSquare(from)
    , toSquare(to)
    , promotion(promo)
{
}

Move Move::fromUci(const std::string& uci) {
    if (uci.length() < 4) {
        return Move(); // Invalid
    }

    int from = squareFromString(uci.substr(0, 2));
    int to = squareFromString(uci.substr(2, 2));

    char promo = '\0';
    if (uci.length() == 5) {
        promo = uci[4];
    }

    return Move(from, to, promo);
}

std::string Move::toUci() const {
    if (!isValid()) {
        return "";
    }

    std::string result = squareToString(fromSquare) + squareToString(toSquare);

    if (promotion != '\0') {
        result += promotion;
    }

    return result;
}

bool Move::isValid() const {
    return fromSquare >= 0 && fromSquare < 64 &&
           toSquare >= 0 && toSquare < 64;
}

bool Move::isPromotion() const {
    return promotion != '\0';
}

int Move::squareFromString(const std::string& sq) {
    if (sq.length() != 2) {
        return -1;
    }

    int file = sq[0] - 'a';  // 0-7
    int rank = sq[1] - '1';  // 0-7

    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return -1;
    }

    return rank * 8 + file;  // 0-63
}

std::string Move::squareToString(int square) {
    if (square < 0 || square > 63) {
        return "";
    }

    int file = square % 8;
    int rank = square / 8;

    std::string result;
    result += char('a' + file);
    result += char('1' + rank);

    return result;
}
