#include "Game.h"

Game::Game() {
}

void Game::addMove(const std::string& uci) {
    moves.push_back(uci);
}

void Game::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

std::string Game::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "?";
}

void Game::addAnalysis(const MoveAnalysis& moveAnalysis) {
    analysis.push_back(moveAnalysis);
}

std::vector<MoveAnalysis> Game::getBlunders(int threshold) const {
    std::vector<MoveAnalysis> blunders;

    for (size_t i = 0; i < analysis.size(); i++) {
        if (analysis[i].scoreDifference > threshold) {
            blunders.push_back(analysis[i]);
        }
    }

    return blunders;
}
