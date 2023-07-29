# Dateiformat

BEANBAG | V100  | C0 [Compression level]    | Filesize | TOC-Size   <-- Header mit fester Größe
7B      | 4B    | 2B                        | 4B       | 4B

Dateiname1\n
Offset[4B]Size[4B]\n
Dateiname2\n
Offset[4B]Size[4B]\n
Dateiname3\n
Offset[4B]Size[4B]\n
Dateiname4\n
Offset[4B]Size[4B]\n

<-- Dateinamen mit Newline abgeschlossen; max. 1024 Zeichen(?)
<-- Nach Angabe von Offset (bezogen auf Ende des TOC; erste Datei immer mit Offset = 0) und Größe wieder Newline, um Lesbarkeit zu erhöhen

Blob1|Blob2|Blob3|Blob4

<-- Nach dem TOC ohne weiteres Trennzeichen die einzelnen Datei-Blobs hintereinander weg
