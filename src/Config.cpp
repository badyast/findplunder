#include "Config.h"
#include <iostream>
#include <cstdlib>
#include <fstream>

Config::Config()
    : startMoveNumber(1)
    , stockfishDepth(15)
    , thresholdCP(150)
    , threads(12)
    , stockfishPath("stockfish")
    , pgnExtractPath("pgn-extract")
    , inputPgnFile("")
    , debugMode(false)
{
}

void Config::loadFromCommandLine(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        exit(1);
    }

    inputPgnFile = argv[1];

    // Parse optional arguments
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--threshold" && i + 1 < argc) {
            thresholdCP = atoi(argv[++i]);
        }
        else if (arg == "--depth" && i + 1 < argc) {
            stockfishDepth = atoi(argv[++i]);
        }
        else if (arg == "--start-move" && i + 1 < argc) {
            startMoveNumber = atoi(argv[++i]);
        }
        else if (arg == "--threads" && i + 1 < argc) {
            threads = atoi(argv[++i]);
        }
        else if (arg == "--stockfish" && i + 1 < argc) {
            stockfishPath = argv[++i];
        }
        else if (arg == "--pgn-extract" && i + 1 < argc) {
            pgnExtractPath = argv[++i];
        }
        else if (arg == "--debug") {
            debugMode = true;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            exit(1);
        }
    }
}

bool Config::validate() const {
    // Check if input PGN file exists
    std::ifstream f(inputPgnFile.c_str());
    if (!f.good()) {
        std::cerr << "Error: Input PGN file not found: " << inputPgnFile << std::endl;
        return false;
    }

    // Validate parameters
    if (thresholdCP <= 0) {
        std::cerr << "Error: Threshold must be positive" << std::endl;
        return false;
    }

    if (stockfishDepth <= 0 || stockfishDepth > 50) {
        std::cerr << "Error: Depth must be between 1 and 50" << std::endl;
        return false;
    }

    if (startMoveNumber < 1) {
        std::cerr << "Error: Start move must be at least 1" << std::endl;
        return false;
    }

    if (threads < 1 || threads > 512) {
        std::cerr << "Error: Threads must be between 1 and 512" << std::endl;
        return false;
    }

    return true;
}

void Config::printUsage(const char* programName) const {
    std::cout << "Usage: " << programName << " <pgn-file> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --threshold <cp>      Minimum score difference in centipawns (default: 150)" << std::endl;
    std::cout << "  --depth <n>           Stockfish search depth (default: 15)" << std::endl;
    std::cout << "  --start-move <n>      Start analysis from move number (default: 1)" << std::endl;
    std::cout << "  --threads <n>         Number of CPU threads for Stockfish (default: 12)" << std::endl;
    std::cout << "  --stockfish <path>    Path to Stockfish binary (default: stockfish)" << std::endl;
    std::cout << "  --pgn-extract <path>  Path to pgn-extract binary (default: pgn-extract)" << std::endl;
    std::cout << "  --debug               Enable debug logging to stockfish_debug.log" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " game.pgn --threshold 200 --depth 20" << std::endl;
}
