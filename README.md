# Findepatzer 🔍♟️

A high-performance chess blunder detection tool written in C++ that analyzes PGN games using Stockfish to identify tactical mistakes and blunders.

## Features

✨ **Fast & Efficient**
- Uses Stockfish MultiPV for single-pass analysis (2x faster than traditional methods)
- Multi-threaded engine support
- Processes entire game collections quickly

🎯 **Smart Detection**
- Configurable blunder threshold (default: 150cp)
- Detects extreme blunders (moves not in top N)
- Mate detection and scoring
- UCI position management for accurate evaluation

📊 **Flexible Analysis**
- Analyze specific games or ranges (`--games "2-5"` or `--games "1,3,7"`)
- Start analysis from any move number
- Adjustable search depth and MultiPV
- Blunders-only mode for clean batch processing

📈 **Clear Output**
```
40W g2g3 | Best: h2h3 (+35cp) | Played: g2g3 (+9cp) | Diff: 26cp
49B c2c1Q | Best: e2b2 (-1500cp) | Played: c2c1Q (not in top 200) | Diff: 9999cp [EXTREME BLUNDER]
```

## Requirements

- **Stockfish** chess engine (any recent version)
- **pgn-extract** for PGN processing
- C++ compiler with C++11 support
- Linux/Unix-like system (WSL works too)

## Installation

1. Clone the repository:
```bash
git clone https://github.com/badyast/findplunder.git
cd findplunder
```

2. Build the project:
```bash
cmake .
make
```

3. Ensure Stockfish and pgn-extract are installed:
```bash
# Ubuntu/Debian
sudo apt-get install stockfish pgn-extract

# Or compile Stockfish from source
# Download from: https://stockfishchess.org/
```

## Usage

### Basic Usage

```bash
./findepatzer game.pgn
```

### Common Options

```bash
# Analyze with custom depth and threshold
./findepatzer game.pgn --depth 20 --threshold 200

# Analyze specific games only
./findepatzer game.pgn --games "2-5"

# Blunders-only output (no move-by-move details)
./findepatzer game.pgn --blunders-only

# Start from move 20 (skip opening)
./findepatzer game.pgn --start-move 20

# Use more CPU threads for faster analysis
./findepatzer game.pgn --threads 16

# Adjust MultiPV (number of top moves analyzed)
./findepatzer game.pgn --multipv 100
```

## Command-Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--threshold <cp>` | Minimum score difference in centipawns to mark as blunder | 150 |
| `--depth <n>` | Stockfish search depth | 15 |
| `--start-move <n>` | Start analysis from move number | 1 |
| `--threads <n>` | Number of CPU threads for Stockfish | 12 |
| `--multipv <n>` | Number of top moves to analyze (1-500) | 200 |
| `--games <sel>` | Analyze specific games: `"2"`, `"2-5"`, or `"2,6,9"` | all |
| `--blunders-only` | Only show blunders, skip per-move output | off |
| `--stockfish <path>` | Path to Stockfish binary | stockfish |
| `--pgn-extract <path>` | Path to pgn-extract binary | pgn-extract |
| `--debug` | Enable debug logging to stockfish_debug.log | off |

## Examples

### Analyze tournament games for big blunders
```bash
./findepatzer tournament.pgn --threshold 300 --blunders-only
```

### Deep analysis of a specific game
```bash
./findepatzer games.pgn --games "5" --depth 25 --multipv 500
```

### Quick check from middlegame onwards
```bash
./findepatzer game.pgn --start-move 15 --depth 10 --multipv 50
```

### Batch processing multiple game collections
```bash
for pgn in *.pgn; do
    ./findepatzer "$pgn" --blunders-only --threshold 200 >> blunders_report.txt
done
```

## How It Works

1. **PGN Parsing**: Converts PGN games to UCI format using pgn-extract
2. **Position Setup**: Uses `position startpos moves ...` to avoid FEN generation issues
3. **MultiPV Analysis**: Analyzes top N moves in single pass
4. **Blunder Detection**:
   - Compares played move to best move
   - Identifies extreme blunders (not in top N)
   - Calculates score difference
5. **Output**: Displays results with clear formatting

### Technical Highlights

- **No FEN Generation**: Uses UCI move sequences for rock-solid position management
- **Single-Pass Analysis**: MultiPV gets best move and played move evaluation together
- **Pipe Communication**: Robust stdin/stdout communication with Stockfish
- **Smart Buffering**: Prevents pipe buffer overflow with unlimited line reading

## Output Format

### Per-Move Analysis
```
40W g2g3 | Best: h2h3 (+35cp) | Played: g2g3 (+9cp) | Diff: 26cp
```

### Blunder Summary
```
=== Blunders Found ===

Game #2 | White: Player1 | Black: Player2 | Move 42w | Played: h3f4 (-805cp) | Best: g4g5 (-653cp) | Loss: 152cp
Game #2 | White: Player1 | Black: Player2 | Move 49b | Played: c2c1Q (-9999cp) | Best: c2c1q (+916cp) | Loss: 9999cp

=== Summary ===
Total games analyzed: 12
Total blunders found: 2
```

## Performance Tips

- **Lower MultiPV** (50-100) for faster analysis of strong games
- **Higher MultiPV** (200-500) for comprehensive analysis of beginner games
- **Start-move** option to skip known openings
- **Blunders-only** mode for large batch processing
- **Adjust depth** based on time constraints (10-15 for quick, 20-25 for thorough)

## Common Issues

### "Stockfish not found"
Ensure Stockfish is in your PATH or specify the path:
```bash
./findepatzer game.pgn --stockfish /path/to/stockfish
```

### "pgn-extract not found"
Install pgn-extract or specify the path:
```bash
./findepatzer game.pgn --pgn-extract /path/to/pgn-extract
```

### Program hangs
- Ensure you're using a recent Stockfish version
- Try with `--debug` flag and check `stockfish_debug.log`
- Reduce `--multipv` value

## Development

### Building from Source
```bash
mkdir build
cd build
cmake ..
make
```

### Project Structure
```
findepatzer/
├── src/
│   ├── main.cpp              # Entry point
│   ├── Config.cpp/h          # Configuration and CLI parsing
│   ├── StockfishEngine.cpp/h # Stockfish communication
│   ├── BlunderAnalyzer.cpp/h # Analysis logic
│   ├── PgnParser.cpp/h       # PGN parsing
│   ├── Game.cpp/h            # Game representation
│   ├── Board.cpp/h           # Board state
│   └── Move.cpp/h            # Move representation
├── CMakeLists.txt
└── README.md
```

## Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests
- Improve documentation

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- **Stockfish** - The powerful chess engine powering the analysis
- **pgn-extract** - For robust PGN parsing
- The chess programming community

## Author

**badyast** - [GitHub](https://github.com/badyast)

---

**Note**: Findepatzer (German for "blunder finder") is designed for analyzing chess games to help players improve by identifying tactical mistakes. It's particularly useful for coaches, students, and anyone looking to learn from their games.

💡 **Tip**: Start with default settings and adjust based on your needs. For most use cases, the defaults work great!
