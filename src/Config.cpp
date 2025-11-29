#include "Config.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <set>
#include <thread>

Config::Config()
    : startMoveNumber(1)
    , stockfishDepth(15)
    , thresholdCP(150)
    , threads(std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1)
    , multiPV(200)
    , stockfishPath("stockfish")
    , pgnExtractPath("pgn-extract")
    , inputPgnFile("")
    , gameSelection("")
    , debugMode(false)
    , blundersOnly(false)
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
        else if (arg == "--multipv" && i + 1 < argc) {
            multiPV = atoi(argv[++i]);
        }
        else if (arg == "--stockfish" && i + 1 < argc) {
            stockfishPath = argv[++i];
        }
        else if (arg == "--pgn-extract" && i + 1 < argc) {
            pgnExtractPath = argv[++i];
        }
        else if (arg == "--games" && i + 1 < argc) {
            gameSelection = argv[++i];
        }
        else if (arg == "--blunders-only") {
            blundersOnly = true;
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

    if (multiPV < 1 || multiPV > 500) {
        std::cerr << "Error: MultiPV must be between 1 and 500" << std::endl;
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
    std::cout << "  --threads <n>         Number of CPU threads for Stockfish (default: auto-detect)" << std::endl;
    std::cout << "  --multipv <n>         Number of top moves to analyze (default: 200)" << std::endl;
    std::cout << "  --games <selection>   Analyze specific games: '2' or '2-5' or '2,6,9' (default: all)" << std::endl;
    std::cout << "  --blunders-only       Only show blunders, skip per-move output" << std::endl;
    std::cout << "  --stockfish <path>    Path to Stockfish binary (default: stockfish)" << std::endl;
    std::cout << "  --pgn-extract <path>  Path to pgn-extract binary (default: pgn-extract)" << std::endl;
    std::cout << "  --debug               Enable debug logging to stockfish_debug.log" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " game.pgn --threshold 200 --depth 20" << std::endl;
    std::cout << "  " << programName << " game.pgn --games \"2-5\" --blunders-only" << std::endl;
    std::cout << "  " << programName << " game.pgn --games \"1,3,7\"" << std::endl;
}

std::set<int> Config::parseGameSelection() const {
    std::set<int> selectedGames;

    if (gameSelection.empty()) {
        return selectedGames;  // Empty set means all games
    }

    // Parse game selection: "2", "2-5", "2,6,9", or combinations
    std::istringstream iss(gameSelection);
    std::string token;

    while (std::getline(iss, token, ',')) {
        // Check if it's a range (e.g., "2-5")
        size_t dashPos = token.find('-');
        if (dashPos != std::string::npos) {
            // Parse range
            int start = atoi(token.substr(0, dashPos).c_str());
            int end = atoi(token.substr(dashPos + 1).c_str());
            for (int i = start; i <= end; i++) {
                if (i > 0) {
                    selectedGames.insert(i);
                }
            }
        } else {
            // Parse single number
            int gameNum = atoi(token.c_str());
            if (gameNum > 0) {
                selectedGames.insert(gameNum);
            }
        }
    }

    return selectedGames;
}
