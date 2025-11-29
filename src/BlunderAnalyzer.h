#ifndef BLUNDER_ANALYZER_H
#define BLUNDER_ANALYZER_H

#include "Config.h"
#include "Game.h"
#include "StockfishEngine.h"
#include <vector>

class BlunderAnalyzer {
public:
    BlunderAnalyzer(const Config& config);
    ~BlunderAnalyzer();

    // Analyze all games and populate analysis data
    void analyzeGames(std::vector<Game>& games);

    // Output blunders found in all games
    void outputBlunders(const std::vector<Game>& games);

private:
    Config config;
    StockfishEngine* engine;

    void analyzeGame(Game& game);
};

#endif // BLUNDER_ANALYZER_H
