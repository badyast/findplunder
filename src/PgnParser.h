#ifndef PGN_PARSER_H
#define PGN_PARSER_H

#include "Game.h"
#include <string>
#include <vector>

class PgnParser {
public:
    // Parse UCI-formatted PGN file (output from pgn-extract -Wuci)
    static std::vector<Game> parseFile(const std::string& filename);

private:
    static bool parseHeaderLine(const std::string& line, std::string& key, std::string& value);
    static std::vector<std::string> parseMoveText(const std::string& line);
};

#endif // PGN_PARSER_H
