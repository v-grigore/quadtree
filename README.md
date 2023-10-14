# Tema 2

Programul implementează un algoritm de comprimare a imaginilor folosind
quadtree-uri. Acesta poate fi utilizat în trei moduri diferite, fiecare fiind
specificat prin primul argument al programului:
- Modul 1 (-c1 factor): Acest mod comprimă o imagine și salvează informații
despre arborele quadtree într-un fișier. Factorul dat ca al doilea argument
specifică pragul de toleranță pentru diferența de culori dintre pixeli. Un
factor mai mic duce la o compresie mai bună, dar poate conduce la pierderea
detaliilor. Fișierul de ieșire va conține următoarele informații pe linii
separate: înălțimea arborelui quadtree, numărul total de blocuri, dimensiunea
blocurilor celei mai înalte niveluri.
- Modul 2 (-c2 factor): Acest mod comprimă o imagine și salvează arborele
quadtree într-un fișier. Factorul dat ca al doilea argument specifică pragul de
toleranță pentru diferența de culori dintre pixeli. Un factor mai mic duce la o
compresie mai bună, dar poate conduce la pierderea detaliilor.
- Modul 3 (-d): Acest mod dezarhivează un fișier cu arborele quadtree și creează
o imagine din acesta. Fișierul de ieșire va fi o imagine în format PPM.

Pentru a realiza aceste operații, programul utilizează mai multe funcții
auxiliare, incluzând funcții pentru citirea și scrierea imaginilor și a
arborelui quadtree. În plus, programul utilizează structuri de date precum Pixel
și QuadTreeNode pentru a stoca informații despre pixeli și noduri din arborele
quadtree.

Programul primește argumente de la linia de comandă și verifică dacă acestea
sunt în numărul corect. Dacă argumentele sunt invalide, programul afișează un
mesaj de eroare și se oprește. În caz contrar, programul determină modul de
funcționare (după primul argument) și continuă cu execuția corespunzătoare. La
sfârșitul fiecărui mod de funcționare, memoria alocată este eliberată.
