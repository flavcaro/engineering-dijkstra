# engineering-dijkstra
Come influisce l’implementazione della priority queue sulle prestazioni di Dijkstra, in termini di tempo e memoria, su famiglie diverse di grafi?

## Datasets

I dataset non sono inclusi nel repository perché troppo grandi.

Random e grid graphs vengono generati automaticamente dal programma.

Per scaricare i dataset reali:

road network:
https://snap.stanford.edu/data/roadNet-CA.html

social graph:
https://snap.stanford.edu/data/com-Youtube.html

Dopo il download i file devono essere messi nelle cartelle:

data/road/
data/social/

e convertiti con:

python3 scripts/convert_snap.py