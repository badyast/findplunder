1. pgn umformatieren mit pgn-extract -Wuci
2. Objekt "Partie" erstellen. Mit allen vorhanden Kopfeldern und der Zugliste.
3. Alle Partien aus der mit pgn-extract erstellten Datei parsen.
4. Grundstellung setzen.
5. Loop: stockfisch einen Zug berechnen lassen und zusammen mit dem score und der Zugnummer merken.
6.  Siehe "C:\Users\badya\Documents\GitHub\Claude_Autonomieexperiment\Tests\Patzer\checkscore.py" Auf diese Weise den score des in der Partie gemachten Zuges errechnen und merken.
7. Den absoluten Wert der scoredifferenz zwischen den beiden Zügen berechnen und merken. Alle Werte sollen im Objekt "Partie" sein. 
8. Loop ende
9. Züge ausgeben bei denen die Scorediffernz den Schwellwert überschreitet. Zusammen mit relevanten Kopfeldern damit man die Partie schnell finden kann. Vorallem die Nummer in der Partieliste. So gut es geht in einer Zeile.




Optionen:
Ab welchem Zug
Rechentiefe stockfisch
Schwellwert
Pfade zu den benötigten externen Programmen. Standard ist, dass sie sich im PFAD der Umgebung befinden. 

Bemerkungen:
stockfish scores sind immer aus der Sicht des am Zug befindlichen
Sinn der App ist es Patzer in Schachpartien zu finden. 