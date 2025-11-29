#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    int startMoveNumber;
    int stockfishDepth;
    int thresholdCP;
    int threads;
    std::string stockfishPath;
    std::string pgnExtractPath;
    std::string inputPgnFile;
    bool debugMode;

    Config();

    void loadFromCommandLine(int argc, char** argv);
    bool validate() const;
    void printUsage(const char* programName) const;
};

#endif // CONFIG_H
