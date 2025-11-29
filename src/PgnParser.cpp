#include "PgnParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<Game> PgnParser::parseFile(const std::string& filename) {
    std::vector<Game> games;
    std::ifstream file(filename.c_str());

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open PGN file: " << filename << std::endl;
        return games;
    }

    Game currentGame;
    bool inHeaders = false;
    bool inMoves = false;
    std::string line;

    while (std::getline(file, line)) {
        // Skip empty lines between games
        if (line.empty()) {
            if (inMoves && !currentGame.moves.empty()) {
                // End of game
                games.push_back(currentGame);
                currentGame = Game();
                inHeaders = false;
                inMoves = false;
            }
            continue;
        }

        // Parse header lines
        if (line[0] == '[') {
            inHeaders = true;
            inMoves = false;

            std::string key, value;
            if (parseHeaderLine(line, key, value)) {
                currentGame.setHeader(key, value);
            }
        }
        // Parse move lines (UCI notation)
        else if (inHeaders || inMoves) {
            inMoves = true;
            std::vector<std::string> moves = parseMoveText(line);
            for (size_t i = 0; i < moves.size(); i++) {
                currentGame.addMove(moves[i]);
            }
        }
    }

    // Don't forget last game if file doesn't end with blank line
    if (!currentGame.moves.empty()) {
        games.push_back(currentGame);
    }

    file.close();
    return games;
}

bool PgnParser::parseHeaderLine(const std::string& line, std::string& key, std::string& value) {
    // Format: [Key "Value"]
    size_t firstQuote = line.find('"');
    size_t lastQuote = line.rfind('"');

    if (firstQuote == std::string::npos || lastQuote == std::string::npos) {
        return false;
    }

    // Extract key (between '[' and first space/quote)
    size_t keyStart = 1; // Skip '['
    size_t keyEnd = line.find(' ', keyStart);
    if (keyEnd == std::string::npos || keyEnd > firstQuote) {
        keyEnd = firstQuote;
    }

    key = line.substr(keyStart, keyEnd - keyStart);

    // Extract value (between quotes)
    value = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);

    return true;
}

std::vector<std::string> PgnParser::parseMoveText(const std::string& line) {
    std::vector<std::string> moves;
    std::istringstream iss(line);
    std::string token;

    while (iss >> token) {
        // Skip result markers
        if (token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*") {
            continue;
        }

        // Skip anything that's not a valid UCI move
        // UCI moves are 4-5 characters: e2e4, e7e8q
        if (token.length() >= 4 && token.length() <= 5) {
            // Basic validation: first two chars should be square, next two should be square
            if (token[0] >= 'a' && token[0] <= 'h' &&
                token[1] >= '1' && token[1] <= '8' &&
                token[2] >= 'a' && token[2] <= 'h' &&
                token[3] >= '1' && token[3] <= '8') {
                moves.push_back(token);
            }
        }
    }

    return moves;
}
