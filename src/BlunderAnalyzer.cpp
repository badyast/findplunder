#include "BlunderAnalyzer.h"
#include "Board.h"
#include "Move.h"
#include <iostream>
#include <cstdlib>

BlunderAnalyzer::BlunderAnalyzer(const Config& cfg)
    : config(cfg)
    , engine(NULL)
{
    engine = new StockfishEngine(config.stockfishPath, config.stockfishDepth, config.threads, config.debugMode);
}

BlunderAnalyzer::~BlunderAnalyzer() {
    if (engine) {
        delete engine;
        engine = NULL;
    }
}

void BlunderAnalyzer::analyzeGames(std::vector<Game>& games) {
    // Initialize engine
    if (!engine->initialize()) {
        std::cerr << "Error: Failed to initialize Stockfish" << std::endl;
        return;
    }

    std::cout << "=== Findepatzer ===" << std::endl;
    std::cout << "Stockfish depth: " << config.stockfishDepth << std::endl;
    std::cout << "Stockfish threads: " << config.threads << std::endl;
    std::cout << "Threshold: " << config.thresholdCP << " cp" << std::endl;
    std::cout << "Start move: " << config.startMoveNumber << std::endl;
    std::cout << "Total games: " << games.size() << std::endl;
    std::cout << std::endl;

    // Analyze each game
    for (size_t i = 0; i < games.size(); i++) {
        std::cout << "Analyzing game " << (i + 1) << "/" << games.size()
                  << ": " << games[i].getHeader("White")
                  << " vs " << games[i].getHeader("Black") << "..." << std::endl;

        analyzeGame(games[i]);
    }

    std::cout << std::endl;
}

void BlunderAnalyzer::analyzeGame(Game& game) {
    Board board;
    board.setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Calculate total moves to analyze
    int totalMovesToAnalyze = 0;
    for (size_t i = 0; i < game.moves.size(); i++) {
        int moveNum = (i / 2) + 1;
        if (moveNum >= config.startMoveNumber) {
            totalMovesToAnalyze++;
        }
    }

    std::cout << "  Total moves to analyze: " << totalMovesToAnalyze << std::endl;

    int analyzedCount = 0;
    for (size_t i = 0; i < game.moves.size(); i++) {
        // Calculate current move number
        int moveNum = (i / 2) + 1;

        // Skip moves before startMoveNumber
        if (moveNum < config.startMoveNumber) {
            Move move = Move::fromUci(game.moves[i]);
            board.makeMove(move);
            continue;
        }

        analyzedCount++;
        std::string side = (i % 2 == 0) ? "White" : "Black";

        std::cout << "  Analyzing move " << analyzedCount << "/" << totalMovesToAnalyze
                  << " (Move " << moveNum << side[0] << ": " << game.moves[i] << ")...";
        std::cout.flush();

        std::string currentFen = board.toFen();
        std::string playedMove = game.moves[i];

        // 1. Get best move and its score from current position
        engine->setPosition(currentFen);
        ScoreResult bestResult = engine->getBestMove(config.stockfishDepth);

        // 2. Get score of played move (checkscore.py approach)
        ScoreResult playedResult = engine->evaluateMove(currentFen, playedMove, config.stockfishDepth);

        // 3. Calculate score difference (absolute value)
        int scoreDiff = abs(playedResult.scoreCP - bestResult.scoreCP);

        // Show result on same line
        if (scoreDiff > config.thresholdCP) {
            std::cout << " BLUNDER! (loss: " << scoreDiff << "cp)" << std::endl;
        } else {
            std::cout << " OK (diff: " << scoreDiff << "cp)" << std::endl;
        }

        // 4. Store analysis
        MoveAnalysis analysis;
        analysis.moveNumber = moveNum;
        analysis.playedMove = playedMove;
        analysis.playedScore = playedResult.scoreCP;
        analysis.bestMove = bestResult.bestMove;
        analysis.bestScore = bestResult.scoreCP;
        analysis.scoreDifference = scoreDiff;
        analysis.isMateScore = bestResult.isMate || playedResult.isMate;

        if (bestResult.isMate) {
            analysis.mateInN = bestResult.mateInN;
        } else if (playedResult.isMate) {
            analysis.mateInN = playedResult.mateInN;
        }

        game.addAnalysis(analysis);

        // 5. Make the played move on the board
        Move move = Move::fromUci(playedMove);
        board.makeMove(move);
    }
}

void BlunderAnalyzer::outputBlunders(const std::vector<Game>& games) {
    std::cout << "=== Blunders Found ===" << std::endl;
    std::cout << std::endl;

    int totalBlunders = 0;

    for (size_t gameIdx = 0; gameIdx < games.size(); gameIdx++) {
        const Game& game = games[gameIdx];
        std::vector<MoveAnalysis> blunders = game.getBlunders(config.thresholdCP);

        for (size_t i = 0; i < blunders.size(); i++) {
            const MoveAnalysis& blunder = blunders[i];

            // Determine side that moved (even index = white, odd = black in 0-based)
            // But we need to look at the actual move number
            bool isWhiteMove = (blunder.moveNumber * 2 - 1) <= static_cast<int>(game.moves.size()) &&
                               (game.moves.size() > 0 &&
                                (blunder.playedMove == game.moves[(blunder.moveNumber - 1) * 2] ||
                                 (blunder.moveNumber == 1 && blunder.playedMove == game.moves[0])));

            // Find position in move list
            std::string sideLetter = "?";
            for (size_t j = 0; j < game.moves.size(); j++) {
                if (game.moves[j] == blunder.playedMove) {
                    sideLetter = (j % 2 == 0) ? "w" : "b";
                    break;
                }
            }

            // Format: Game #N | White | Black | Move Nw/b | Played (score) | Best (score) | Loss
            std::cout << "Game #" << (gameIdx + 1)
                      << " | White: " << game.getHeader("White")
                      << " | Black: " << game.getHeader("Black")
                      << " | Move " << blunder.moveNumber << sideLetter
                      << " | Played: " << blunder.playedMove;

            if (blunder.isMateScore && blunder.playedScore > 5000) {
                std::cout << " (mate)";
            } else if (blunder.isMateScore && blunder.playedScore < -5000) {
                std::cout << " (-mate)";
            } else {
                std::cout << " (";
                if (blunder.playedScore > 0) std::cout << "+";
                std::cout << blunder.playedScore << "cp)";
            }

            std::cout << " | Best: " << blunder.bestMove;

            if (blunder.isMateScore && blunder.bestScore > 5000) {
                std::cout << " (mate)";
            } else if (blunder.isMateScore && blunder.bestScore < -5000) {
                std::cout << " (-mate)";
            } else {
                std::cout << " (";
                if (blunder.bestScore > 0) std::cout << "+";
                std::cout << blunder.bestScore << "cp)";
            }

            std::cout << " | Loss: " << blunder.scoreDifference << "cp"
                      << std::endl;

            totalBlunders++;
        }
    }

    std::cout << std::endl;
    std::cout << "=== Summary ===" << std::endl;
    std::cout << "Total games analyzed: " << games.size() << std::endl;
    std::cout << "Total blunders found: " << totalBlunders << std::endl;
}
