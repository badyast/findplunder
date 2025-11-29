#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <set>

class Config {
public:
    int startMoveNumber;
    int stockfishDepth;
    int thresholdCP;
    int threads;
    std::string stockfishPath;
    std::string pgnExtractPath;
    std::string inputPgnFile;
    std::string gameSelection;  // e.g., "2", "2-5", "2,6,9"
    bool debugMode;
    bool blundersOnly;  // Only show blunders, skip per-move output

    Config();

    void loadFromCommandLine(int argc, char** argv);
    bool validate() const;
    void printUsage(const char* programName) const;

    // Parse game selection string (e.g., "2", "2-5", "2,6,9") into a set of game indices (1-based)
    // Returns empty set if no selection (meaning all games)
    std::set<int> parseGameSelection() const;
};

#endif // CONFIG_H
