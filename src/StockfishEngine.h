#ifndef STOCKFISH_ENGINE_H
#define STOCKFISH_ENGINE_H

#include <string>
#include <vector>

struct ScoreResult {
    std::string bestMove;
    int scoreCP;      // Centipawns (from side to move perspective)
    bool isMate;
    int mateInN;

    ScoreResult() : scoreCP(0), isMate(false), mateInN(0) {}
};

class StockfishEngine {
public:
    StockfishEngine(const std::string& path, int depth, int numThreads = 1, bool enableDebug = false);
    ~StockfishEngine();

    bool initialize();
    void terminate();

    // Set current position
    void setPosition(const std::string& fen, const std::vector<std::string>& moves = std::vector<std::string>());

    // Get best move from current position
    ScoreResult getBestMove(int depth);

    // Evaluate a specific move from a FEN position (checkscore.py approach)
    ScoreResult evaluateMove(const std::string& fen, const std::string& move, int depth);

private:
    std::string stockfishPath;
    int defaultDepth;
    int threads;
    bool debugMode;

    // Process communication
    int pid;
    int fdToEngine;    // File descriptor to write to Stockfish
    int fdFromEngine;  // File descriptor to read from Stockfish
    std::string readBuffer;  // Buffer for partial lines
    FILE* logFile;     // Debug log file

    bool sendCommand(const std::string& cmd);
    std::string readLine();
    bool waitUntilReady();  // Send isready and wait for readyok
    ScoreResult parseSearchResult();
};

#endif // STOCKFISH_ENGINE_H
