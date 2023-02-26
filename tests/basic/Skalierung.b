' Punkte und Linien einlesen
READ anzp%, anzl%
DIM x.punkte(32), y.punkte(32)
DIM s.linien%(29), e.linien%(29)

' Skalierungsfaktoren
sx = 1
sy = 1

' Bildposition
mx% = MOUSE(3)
my% = MOUSE(4)

' Punkte einlesen
FOR i = 0 TO anzp%-1
	READ x.punkte(i), y.punkte(i)
NEXT i

' Linien einlesen
FOR i = 0 TO anzl%-1
	READ s.linien%(i), e.linien%(i)
NEXT i

' Alle Koordinaten des Bildes transformieren
SUB transformiere.bild(sx, sy)
	SHARED anzp%, x.punkte, y.punkte
    FOR i=0 TO anzp%-1
        x.punkte(i) = sx*x.punkte(i)
    	y.punkte(i) = sy*y.punkte(i)
    NEXT i
END SUB

' Fenster öffnen
WINDOW 1,"Skalierung",(0,0)-(600,400),31
COLOR 2,1
CLS

' Hauptschleife
WHILE c<>27	' bis Escape

    ' Auf Tastendruck / Maustaste warten
    c$=""
    m%=0
    WHILE c$="" AND m%=0
    	SLEEP
        c$=INKEY$  		' Tastendruck
        m%=MOUSE(0)		' Maustaste
    WEND

    CLS

    sx = 1
    sy = 1

    IF m%<>0 THEN
    	mx% = MOUSE(1)
        my% = MOUSE(2)
    END IF

    IF c$ <> "" THEN
    	c = ASC(c$)
        'LOCATE 1,1
        'PRINT c

        IF c=56 THEN
        	sy = 1.2
        END IF
        IF c=50 THEN
        	sy = .8
        END IF
        IF c=54 THEN
        	sx = 1.2
        END IF
        IF c=52 THEN
        	sx = .8
        END IF
    END IF

    ' Gesamtes Bild transformieren
    CALL transformiere.bild(sx, sy)

    FOR i = 0 TO anzl%-1
    	x1% = x.punkte(s.linien%(i))
        y1% = y.punkte(s.linien%(i))
        x2% = x.punkte(e.linien%(i))
        y2% = y.punkte(e.linien%(i))
        LINE (mx%+x1%, my%-y1%) - (mx%+x2%,my%-y2%)
    NEXT i
WEND

WINDOW CLOSE 1
END

' Bilddaten
DATA 33
DATA 30

' Punktkoordinaten
DATA 0,3,	0,7, 	2,8,	4,8,	4,12
DATA 6,13,	20,13,	22,12,	22,8,	24,8
DATA 26,7,	24,12,	26,3,	24,3,	24,0
DATA 20,0,	20,3,	6,3,	6,0,	2,0
DATA 2,3,	6,9,	6,12,	20,12,	20,9
DATA 22,5,	20,6,	22,7,	24,6,	4,5
DATA 2,6,	4,7,	6,6

' Linien
DATA 0,1,	1,2,	2,9,	9,10,	9,11
DATA 10,12,	12,0,	3,4,	4,5,	5,6
DATA 6,7,	7,8,	21,22,	22,23,	23,24
DATA 24,21,	13,14,	14,15,	15,16,	17,18
DATA 18,19,	19,20,	25,26,	26,27,	27,28
DATA 28,25,	29,30,	30,31,	31,32,	32,29

