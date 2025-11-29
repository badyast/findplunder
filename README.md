# Findepatzer - Chess Blunder Finder

Ein C++ Tool zum Finden von Patzern (Blunders) in Schachpartien mit Stockfish-Analyse.

## Anforderungen

- C++11 Compiler (g++, clang++)
- CMake 3.5 oder höher
- Stockfish (Chess Engine)
- pgn-extract

### Installation der Abhängigkeiten

#### Ubuntu/Debian:
```bash
sudo apt-get install cmake g++ stockfish pgn-extract
```

#### macOS:
```bash
brew install cmake stockfish pgn-extract
```

## Kompilierung

```bash
mkdir build
cd build
cmake ..
make
```

Das erzeugt die ausführbare Datei `findepatzer` im `build/` Verzeichnis.

## Verwendung

```bash
./findepatzer <pgn-file> [options]
```

### Optionen:

- `--threshold <cp>` - Mindest-Scoredifferenz in Centipawns (Standard: 150)
- `--depth <n>` - Stockfish Suchtiefe (Standard: 15)
- `--start-move <n>` - Ab welchem Zug analysieren (Standard: 1)
- `--stockfish <path>` - Pfad zu Stockfish Binary (Standard: stockfish)
- `--pgn-extract <path>` - Pfad zu pgn-extract Binary (Standard: pgn-extract)

### Beispiele:

```bash
# Einfach mit Standardwerten
./findepatzer game.pgn

# Mit höherem Threshold und größerer Tiefe
./findepatzer game.pgn --threshold 200 --depth 20

# Ab Zug 10 analysieren
./findepatzer game.pgn --start-move 10

# Eigene Pfade zu Stockfish und pgn-extract
./findepatzer game.pgn --stockfish /usr/local/bin/stockfish --pgn-extract /usr/bin/pgn-extract
```

## Funktionsweise

1. Konvertiert PGN mit `pgn-extract -Wuci` in UCI-Format
2. Parst alle Partien aus der Datei
3. Für jeden Zug:
   - Berechnet mit Stockfish den besten Zug und Score
   - Berechnet den Score des tatsächlich gespielten Zuges
   - Berechnet die absolute Differenz
4. Gibt alle Züge aus, bei denen die Differenz den Schwellwert überschreitet

## Ausgabe

Das Programm gibt Patzer in folgendem Format aus:

```
Game #1 | White: Fischer | Black: Spassky | Move 15w | Played: Nf3 (-45cp) | Best: Qxd5 (+123cp) | Loss: 168cp
```

- **Game #N** - Spielnummer in der PGN-Datei
- **White/Black** - Spielernamen
- **Move Nw/b** - Zugnummer (w=Weiß, b=Schwarz)
- **Played** - Gespielter Zug mit Score
- **Best** - Bester Zug laut Stockfish mit Score
- **Loss** - Score-Differenz in Centipawns

## Hinweise

- Stockfish-Scores sind immer aus Sicht des am Zug befindlichen
- Die Analyse kann je nach Tiefe und Anzahl der Partien längere Zeit in Anspruch nehmen
- Temporäre UCI-PGN Dateien werden automatisch gelöscht

## Implementierung

Basiert auf der Spezifikation in `README.txt`:
- C++11 Standard für maximale Kompatibilität
- UCI-Protokoll Kommunikation mit Stockfish via fork/pipe
- Eigene Schachlogik für FEN-Parsing und Zugausführung
- Modulare Architektur mit klaren Klassentrennung
