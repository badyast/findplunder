#include "Config.h"
#include "PgnParser.h"
#include "BlunderAnalyzer.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>

// Convert PGN to UCI format using pgn-extract
bool convertPgnToUci(const Config& config, std::string& uciPgnFile) {
    // Create temporary filename for UCI output
    uciPgnFile = config.inputPgnFile + ".uci.pgn";

    // Build pgn-extract command
    std::string command = config.pgnExtractPath + " -Wuci " + config.inputPgnFile + " > " + uciPgnFile;

    std::cout << "Converting PGN to UCI format..." << std::endl;
    std::cout << "Command: " << command << std::endl;

    int result = system(command.c_str());

    if (result != 0) {
        std::cerr << "Error: pgn-extract failed with code " << result << std::endl;
        std::cerr << "Make sure pgn-extract is installed and in your PATH" << std::endl;
        return false;
    }

    std::cout << "Conversion successful: " << uciPgnFile << std::endl;
    std::cout << std::endl;

    return true;
}

// Clean up temporary UCI file
void cleanupTempFile(const std::string& filename) {
    remove(filename.c_str());
}

int main(int argc, char** argv) {
    // Parse configuration
    Config config;
    config.loadFromCommandLine(argc, argv);

    // Validate configuration
    if (!config.validate()) {
        return 1;
    }

    // Convert PGN to UCI format
    std::string uciPgnFile;
    if (!convertPgnToUci(config, uciPgnFile)) {
        return 1;
    }

    // Parse UCI-formatted PGN
    std::cout << "Parsing games..." << std::endl;
    std::vector<Game> games = PgnParser::parseFile(uciPgnFile);

    if (games.empty()) {
        std::cerr << "Error: No games found in file" << std::endl;
        cleanupTempFile(uciPgnFile);
        return 1;
    }

    std::cout << "Found " << games.size() << " game(s)" << std::endl;
    std::cout << std::endl;

    // Analyze games
    BlunderAnalyzer analyzer(config);
    analyzer.analyzeGames(games);

    // Output blunders
    analyzer.outputBlunders(games);

    // Cleanup
    cleanupTempFile(uciPgnFile);

    return 0;
}
