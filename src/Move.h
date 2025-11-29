#ifndef MOVE_H
#define MOVE_H

#include <string>

class Move {
public:
    int fromSquare;  // 0-63 (a1=0, b1=1, ..., h8=63)
    int toSquare;    // 0-63
    char promotion;  // 'q', 'r', 'b', 'n', or '\0' for no promotion

    Move();
    Move(int from, int to, char promo = '\0');

    static Move fromUci(const std::string& uci);
    std::string toUci() const;

    bool isValid() const;
    bool isPromotion() const;

private:
    static int squareFromString(const std::string& sq);
    static std::string squareToString(int square);
};

#endif // MOVE_H
