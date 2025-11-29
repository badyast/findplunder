#ifndef GAME_H
#define GAME_H

#include <string>
#include <map>
#include <vector>

struct MoveAnalysis {
    int moveNumber;
    std::string playedMove;      // UCI notation
    int playedScore;             // Centipawns
    std::string bestMove;        // UCI notation
    int bestScore;               // Centipawns
    int scoreDifference;         // abs(playedScore - bestScore)
    bool isMateScore;
    int mateInN;

    MoveAnalysis()
        : moveNumber(0)
        , playedScore(0)
        , bestScore(0)
        , scoreDifference(0)
        , isMateScore(false)
        , mateInN(0)
    {}
};

class Game {
public:
    std::map<std::string, std::string> headers;
    std::vector<std::string> moves;
    std::vector<MoveAnalysis> analysis;

    Game();

    void addMove(const std::string& uci);
    void setHeader(const std::string& key, const std::string& value);
    std::string getHeader(const std::string& key) const;

    void addAnalysis(const MoveAnalysis& moveAnalysis);
    std::vector<MoveAnalysis> getBlunders(int threshold) const;
};

#endif // GAME_H
