#include "BlunderAnalyzer.h"
#include "Board.h"
#include "Move.h"
#include <iostream>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <cctype>

BlunderAnalyzer::BlunderAnalyzer(const Config& cfg)
    : config(cfg)
    , engine(NULL)
{
    engine = new StockfishEngine(config.stockfishPath, config.stockfishDepth, config.threads, config.multiPV, config.debugMode);
}

BlunderAnalyzer::~BlunderAnalyzer() {
    if (engine) {
        delete engine;
        engine = NULL;
    }
}

// Helper function to normalize UCI moves to lowercase for comparison
// (e.g., h7h8Q and h7h8q should be treated as the same move)
static std::string toLowerUCI(const std::string& move) {
    std::string result = move;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void BlunderAnalyzer::analyzeGames(std::vector<Game>& games) {
    // Initialize engine
    if (!engine->initialize()) {
        std::cerr << "Error: Failed to initialize Stockfish" << std::endl;
        return;
    }

    // Parse game selection
    std::set<int> selectedGames = config.parseGameSelection();

    std::cout << "=== Findepatzer ===" << std::endl;
    std::cout << "Stockfish depth: " << config.stockfishDepth << std::endl;
    std::cout << "Stockfish threads: " << config.threads << std::endl;
    std::cout << "Threshold: " << config.thresholdCP << " cp" << std::endl;
    std::cout << "Start move: " << config.startMoveNumber << std::endl;
    if (!selectedGames.empty()) {
        std::cout << "Selected games: " << config.gameSelection << std::endl;
    }
    if (config.blundersOnly) {
        std::cout << "Mode: Blunders only" << std::endl;
    }
    std::cout << "Total games: " << games.size() << std::endl;
    std::cout << std::endl;

    // Analyze each game
    for (size_t i = 0; i < games.size(); i++) {
        // Check if this game is selected (1-based index)
        if (!selectedGames.empty() && selectedGames.find(i + 1) == selectedGames.end()) {
            continue;  // Skip this game
        }

        std::cout << "Analyzing game " << (i + 1) << "/" << games.size()
                  << ": " << games[i].getHeader("White")
                  << " vs " << games[i].getHeader("Black") << "..." << std::endl;

        analyzeGame(games[i], i + 1);
    }

    std::cout << std::endl;
}

void BlunderAnalyzer::analyzeGame(Game& game, int gameIndex) {
    Board board;
    board.setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Keep track of all moves from the start (for UCI position command)
    std::vector<std::string> allMoves;

    // Calculate total moves to analyze
    int totalMovesToAnalyze = 0;
    for (size_t i = 0; i < game.moves.size(); i++) {
        int moveNum = (i / 2) + 1;
        if (moveNum >= config.startMoveNumber) {
            totalMovesToAnalyze++;
        }
    }

    if (!config.blundersOnly) {
        std::cout << "  Total moves to analyze: " << totalMovesToAnalyze << std::endl;
    }

    int analyzedCount = 0;
    for (size_t i = 0; i < game.moves.size(); i++) {
        // Calculate current move number
        int moveNum = (i / 2) + 1;

        std::string playedMove = game.moves[i];

        // Skip moves before startMoveNumber (but still track them for position)
        if (moveNum < config.startMoveNumber) {
            Move move = Move::fromUci(playedMove);
            board.makeMove(move);
            allMoves.push_back(playedMove);
            continue;
        }

        analyzedCount++;
        std::string side = (i % 2 == 0) ? "White" : "Black";

        // Show progress indicator in blunders-only mode
        if (config.blundersOnly) {
            std::cout << "\rAnalyzing move " << analyzedCount << "/" << totalMovesToAnalyze << "..." << std::flush;
        }

        // 1. Analyze position with MultiPV to get all top moves
        std::vector<MoveScore> topMoves = engine->analyzePosition("startpos", allMoves, config.stockfishDepth);

        if (topMoves.empty()) {
            std::cout << "  Move " << moveNum << side[0] << ": " << playedMove
                      << " | ERROR: No moves from engine" << std::endl;
            continue;
        }

        // 2. Best move is always first (multipv 1)
        MoveScore bestMove = topMoves[0];

        // 3. Find the played move in the top moves list
        // Normalize to lowercase for comparison (h7h8Q == h7h8q)
        MoveScore* playedMoveScore = nullptr;
        std::string playedMoveLower = toLowerUCI(playedMove);
        for (size_t j = 0; j < topMoves.size(); j++) {
            if (toLowerUCI(topMoves[j].move) == playedMoveLower) {
                playedMoveScore = &topMoves[j];
                break;
            }
        }

        // 4. Calculate score difference
        int scoreDiff;
        int playedScore;
        bool isMate = bestMove.isMate;
        int mateInN = bestMove.mateInN;

        if (playedMoveScore != nullptr) {
            // Played move found in top moves
            playedScore = playedMoveScore->scoreCP;
            scoreDiff = abs(playedScore - bestMove.scoreCP);
            if (playedMoveScore->isMate) {
                isMate = true;
                mateInN = playedMoveScore->mateInN;
            }
        } else {
            // Played move NOT in top 200 - it's extremely bad!
            playedScore = -9999;  // Placeholder for "very bad"
            scoreDiff = 9999;  // Mark as huge blunder
        }

        // 5. Check if this is a blunder
        bool isBlunder = (playedMoveScore == nullptr) || (scoreDiff > config.thresholdCP);

        // 6. Format and display the result
        // In blunders-only mode, only show blunders immediately
        // In normal mode, show all moves
        if (!config.blundersOnly || isBlunder) {
            // In blunders-only mode, show game info for context
            if (config.blundersOnly && isBlunder) {
                std::cout << "Game #" << gameIndex << " | "
                          << "White: " << game.getHeader("White") << " | "
                          << "Black: " << game.getHeader("Black") << " | ";
            } else {
                std::cout << "  ";
            }

            std::cout << moveNum << side[0] << " " << playedMove << " | ";

            // Best move
            std::cout << "Best: " << bestMove.move << " (";
            if (bestMove.isMate) {
                std::cout << (bestMove.mateInN > 0 ? "+" : "") << "M" << abs(bestMove.mateInN);
            } else {
                std::cout << (bestMove.scoreCP > 0 ? "+" : "") << bestMove.scoreCP << "cp";
            }
            std::cout << ") | ";

            // Played move
            std::cout << "Played: " << playedMove << " (";
            if (playedMoveScore != nullptr) {
                if (playedMoveScore->isMate) {
                    std::cout << (playedMoveScore->mateInN > 0 ? "+" : "") << "M" << abs(playedMoveScore->mateInN);
                } else {
                    std::cout << (playedScore > 0 ? "+" : "") << playedScore << "cp";
                }
            } else {
                std::cout << "not in top " << config.multiPV;
            }
            std::cout << ") | ";

            // Difference
            std::cout << "Diff: " << scoreDiff << "cp";

            // Verdict
            if (playedMoveScore == nullptr) {
                std::cout << " [EXTREME BLUNDER]";
            } else if (scoreDiff > config.thresholdCP) {
                std::cout << " [BLUNDER]";
            }

            std::cout << std::endl;
        } else if (config.blundersOnly && !isBlunder) {
            // Clear the progress line if no blunder (move was good)
            std::cout << "\r" << std::string(80, ' ') << "\r" << std::flush;
        }

        // 6. Store analysis
        MoveAnalysis analysis;
        analysis.moveNumber = moveNum;
        analysis.playedMove = playedMove;
        analysis.playedScore = playedScore;
        analysis.bestMove = bestMove.move;
        analysis.bestScore = bestMove.scoreCP;
        analysis.scoreDifference = scoreDiff;
        analysis.isMateScore = isMate;
        analysis.mateInN = mateInN;

        game.addAnalysis(analysis);

        // 7. Make the played move on the board and add to move list
        Move move = Move::fromUci(playedMove);
        board.makeMove(move);
        allMoves.push_back(playedMove);
    }

    // Clear progress line at the end of game analysis
    if (config.blundersOnly) {
        std::cout << "\r" << std::string(80, ' ') << "\r" << std::flush;
    }
}

void BlunderAnalyzer::outputBlunders(const std::vector<Game>& games) {
    int totalBlunders = 0;

    // In blunders-only mode, we already printed blunders during analysis
    // So we just need to count them for the summary
    if (!config.blundersOnly) {
        std::cout << "=== Blunders Found ===" << std::endl;
        std::cout << std::endl;

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
    } else {
        // Just count blunders for summary
        for (size_t gameIdx = 0; gameIdx < games.size(); gameIdx++) {
            const Game& game = games[gameIdx];
            std::vector<MoveAnalysis> blunders = game.getBlunders(config.thresholdCP);
            totalBlunders += blunders.size();
        }
    }

    std::cout << std::endl;
    std::cout << "=== Summary ===" << std::endl;
    std::cout << "Total games analyzed: " << games.size() << std::endl;
    std::cout << "Total blunders found: " << totalBlunders << std::endl;
}
