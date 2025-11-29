#include "StockfishEngine.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <errno.h>

StockfishEngine::StockfishEngine(const std::string& path, int depth, int numThreads, bool enableDebug)
    : stockfishPath(path)
    , defaultDepth(depth)
    , threads(numThreads)
    , debugMode(enableDebug)
    , pid(-1)
    , fdToEngine(-1)
    , fdFromEngine(-1)
    , readBuffer("")
    , logFile(NULL)
{
    // Open debug log file only in debug mode
    if (debugMode) {
        logFile = fopen("stockfish_debug.log", "w");
        if (logFile) {
            fprintf(logFile, "=== Stockfish Communication Log ===\n");
            fflush(logFile);
        }
    }
}

StockfishEngine::~StockfishEngine() {
    terminate();
    if (logFile) {
        fclose(logFile);
        logFile = NULL;
    }
}

bool StockfishEngine::initialize() {
    int pipeToEngine[2];
    int pipeFromEngine[2];

    if (pipe(pipeToEngine) == -1 || pipe(pipeFromEngine) == -1) {
        std::cerr << "Error: Failed to create pipes" << std::endl;
        return false;
    }

    pid = fork();

    if (pid == -1) {
        std::cerr << "Error: Failed to fork process" << std::endl;
        return false;
    }

    if (pid == 0) {
        // Child process - run Stockfish
        close(pipeToEngine[1]);
        close(pipeFromEngine[0]);

        dup2(pipeToEngine[0], STDIN_FILENO);
        dup2(pipeFromEngine[1], STDOUT_FILENO);

        // Redirect stderr to /dev/null to prevent debug output interference
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        close(pipeToEngine[0]);
        close(pipeFromEngine[1]);

        execlp(stockfishPath.c_str(), stockfishPath.c_str(), (char*)NULL);

        // If exec fails
        exit(1);
    }

    // Parent process
    close(pipeToEngine[0]);
    close(pipeFromEngine[1]);

    fdToEngine = pipeToEngine[1];
    fdFromEngine = pipeFromEngine[0];

    // Initialize UCI
    usleep(100000);  // 100ms delay
    sendCommand("uci");

    // Wait for uciok (silently consume all output)
    std::string line;
    int lineCount = 0;
    int emptyCount = 0;
    while (true) {
        line = readLine();
        lineCount++;
        if (line.find("uciok") == 0) {
            break;
        }
        if (line.empty()) {
            emptyCount++;
            if (emptyCount > 3) {
                std::cerr << "Error: Stockfish didn't respond to 'uci'" << std::endl;
                return false;
            }
            continue;  // Skip empty lines
        }
        if (lineCount > 100) {
            std::cerr << "Error: Too many lines without uciok" << std::endl;
            return false;
        }
    }

    // Set number of threads
    usleep(100000);  // 100ms delay
    std::ostringstream threadsCmd;
    threadsCmd << "setoption name Threads value " << threads;
    sendCommand(threadsCmd.str());

    // Enable Stockfish internal debug logging (only in debug mode)
    if (debugMode) {
        usleep(100000);  // 100ms delay
        sendCommand("setoption name Debug Log File value stockfish_internal.log");
    }

    // Set to UCI mode
    usleep(100000);  // 100ms delay
    sendCommand("isready");

    // Wait for readyok
    while (true) {
        line = readLine();
        if (line.find("readyok") == 0) {
            break;
        }
        if (line.empty()) {
            std::cerr << "Error: Stockfish didn't respond to 'isready'" << std::endl;
            return false;
        }
    }

    if (logFile) {
        fprintf(logFile, "\n=== Stockfish initialized successfully ===\n");
        if (debugMode) {
            fprintf(logFile, "Internal Stockfish debug log: stockfish_internal.log\n");
        }
        fprintf(logFile, "\n");
        fflush(logFile);
    }

    if (debugMode) {
        std::cout << "Debug mode enabled: stockfish_debug.log, stockfish_internal.log" << std::endl;
    }

    return true;
}

void StockfishEngine::terminate() {
    if (pid != -1) {
        sendCommand("quit");

        if (fdToEngine >= 0) {
            close(fdToEngine);
            fdToEngine = -1;
        }
        if (fdFromEngine >= 0) {
            close(fdFromEngine);
            fdFromEngine = -1;
        }

        waitpid(pid, NULL, 0);
        pid = -1;
    }
}

bool StockfishEngine::sendCommand(const std::string& cmd) {
    if (fdToEngine < 0) {
        return false;
    }

    // Log command with call stack info
    if (logFile) {
        fprintf(logFile, ">>> SEND: %s (fdToEngine=%d)\n", cmd.c_str(), fdToEngine);
        fflush(logFile);
    }

    std::string cmdWithNewline = cmd + "\n";
    ssize_t written = write(fdToEngine, cmdWithNewline.c_str(), cmdWithNewline.length());

    bool success = written == (ssize_t)cmdWithNewline.length();
    if (logFile) {
        fprintf(logFile, "    (written %zd bytes, success=%d, errno=%d)\n", written, success, errno);
        fflush(logFile);
    }

    // Verify the write completed fully
    if (written != (ssize_t)cmdWithNewline.length()) {
        fprintf(logFile, "    WARNING: Partial write! Expected %zu, got %zd\n", cmdWithNewline.length(), written);
        fflush(logFile);
    }

    return success;
}

std::string StockfishEngine::readLine() {
    if (fdFromEngine < 0) {
        if (logFile) {
            fprintf(logFile, "<<< ERROR: fdFromEngine < 0\n");
            fflush(logFile);
        }
        return "";
    }

    while (true) {
        // Check if we have a complete line in the buffer
        size_t newlinePos = readBuffer.find('\n');
        if (newlinePos != std::string::npos) {
            // Extract line and remove from buffer
            std::string line = readBuffer.substr(0, newlinePos);
            readBuffer = readBuffer.substr(newlinePos + 1);

            // Log received line
            if (logFile) {
                fprintf(logFile, "<<< RECV: %s\n", line.c_str());
                fflush(logFile);
            }

            return line;
        }

        // Need to read more data
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(fdFromEngine, &readfds);

        int ret = select(fdFromEngine + 1, &readfds, NULL, NULL, &timeout);

        if (ret == 0) {
            if (logFile) {
                fprintf(logFile, "<<< TIMEOUT after 60 seconds\n");
                fflush(logFile);
            }
            std::cerr << "\nWarning: Stockfish timeout" << std::endl;
            return "";
        } else if (ret < 0) {
            if (logFile) {
                fprintf(logFile, "<<< ERROR: select() failed (errno=%d)\n", errno);
                fflush(logFile);
            }
            std::cerr << "\nError: select() failed" << std::endl;
            return "";
        }

        // Read available data
        char buffer[4096];
        ssize_t n = read(fdFromEngine, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            if (logFile) {
                fprintf(logFile, "<<< ERROR: read() returned %zd (errno=%d)\n", n, errno);
                fflush(logFile);
            }
            // End of stream or error
            if (!readBuffer.empty()) {
                std::string line = readBuffer;
                readBuffer.clear();
                if (logFile) {
                    fprintf(logFile, "<<< RECV (final): %s\n", line.c_str());
                    fflush(logFile);
                }
                return line;
            }
            return "";
        }

        buffer[n] = '\0';
        readBuffer += std::string(buffer);

        if (logFile) {
            fprintf(logFile, "    (read %zd bytes into buffer, buffer now has %zu chars)\n", n, readBuffer.size());
            fflush(logFile);
        }
    }
}

void StockfishEngine::setPosition(const std::string& fen, const std::vector<std::string>& moves) {
    std::ostringstream cmd;

    // If fen is "startpos", use startpos instead of FEN notation
    if (fen == "startpos") {
        cmd << "position startpos";
    } else {
        cmd << "position fen " << fen;
    }

    if (!moves.empty()) {
        cmd << " moves";
        for (size_t i = 0; i < moves.size(); i++) {
            cmd << " " << moves[i];
        }
    }

    usleep(50000);  // 50ms delay
    sendCommand(cmd.str());

    // Wait for position to be set before continuing
    usleep(50000);  // 50ms delay
    if (!waitUntilReady()) {
        std::cerr << "Warning: Stockfish not ready after setPosition" << std::endl;
    }
}

bool StockfishEngine::waitUntilReady() {
    if (logFile) {
        fprintf(logFile, "\n=== Checking if Stockfish is ready ===\n");
        fflush(logFile);
    }

    sendCommand("isready");

    std::string line;
    // Read ALL lines until we get "readyok" - no limit!
    // This prevents pipe buffer overflow
    while (true) {
        line = readLine();
        if (line.find("readyok") == 0) {
            if (logFile) {
                fprintf(logFile, "=== Stockfish is ready ===\n\n");
                fflush(logFile);
            }
            return true;
        }
        if (line.empty()) {
            if (logFile) {
                fprintf(logFile, "=== ERROR: No readyok received (timeout or connection lost) ===\n\n");
                fflush(logFile);
            }
            return false;
        }
        // Continue reading - no limit on number of lines!
    }
}

ScoreResult StockfishEngine::getBestMove(int depth) {
    std::ostringstream cmd;
    cmd << "go depth " << depth;
    usleep(50000);  // 50ms delay
    sendCommand(cmd.str());

    return parseSearchResult();
}

ScoreResult StockfishEngine::evaluateMove(const std::string& fenOrStartpos, const std::vector<std::string>& movesToPosition, const std::string& moveToEvaluate, int depth) {
    // Set position after the move (checkscore.py approach)
    std::ostringstream cmd;

    // Use startpos or FEN
    if (fenOrStartpos == "startpos") {
        cmd << "position startpos";
    } else {
        cmd << "position fen " << fenOrStartpos;
    }

    // Add all moves to get to the position, plus the move to evaluate
    cmd << " moves";
    for (size_t i = 0; i < movesToPosition.size(); i++) {
        cmd << " " << movesToPosition[i];
    }
    cmd << " " << moveToEvaluate;

    usleep(50000);  // 50ms delay
    sendCommand(cmd.str());

    // Wait for position to be processed
    usleep(50000);  // 50ms delay
    if (!waitUntilReady()) {
        std::cerr << "Warning: Stockfish not ready after setPosition in evaluateMove" << std::endl;
    }

    // Search
    cmd.str("");
    cmd << "go depth " << depth;
    usleep(50000);  // 50ms delay
    sendCommand(cmd.str());

    return parseSearchResult();
}

ScoreResult StockfishEngine::parseSearchResult() {
    ScoreResult result;
    std::string line;

    while (true) {
        line = readLine();

        // Check for empty line (timeout or error)
        if (line.empty()) {
            std::cerr << "\nError: No response from Stockfish, returning empty result" << std::endl;
            break;
        }

        if (line.find("info ") == 0) {
            // Parse score from info line
            size_t cpPos = line.find(" cp ");
            if (cpPos != std::string::npos) {
                std::istringstream iss(line.substr(cpPos + 4));
                iss >> result.scoreCP;
                result.isMate = false;
            }

            size_t matePos = line.find(" mate ");
            if (matePos != std::string::npos) {
                std::istringstream iss(line.substr(matePos + 6));
                iss >> result.mateInN;
                result.isMate = true;
                // Convert mate to large score
                result.scoreCP = (result.mateInN > 0) ? 10000 : -10000;
            }

            // Parse best move from pv
            size_t pvPos = line.find(" pv ");
            if (pvPos != std::string::npos) {
                std::istringstream iss(line.substr(pvPos + 4));
                iss >> result.bestMove;
            }
        }

        if (line.find("bestmove ") == 0) {
            // Parse best move
            std::istringstream iss(line.substr(9));
            std::string move;
            iss >> move;

            // Only update if we didn't get it from pv
            if (result.bestMove.empty()) {
                result.bestMove = move;
            }

            break;
        }
    }

    return result;
}
