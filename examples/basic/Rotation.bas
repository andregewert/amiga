' Rotation eines Kurvenzuges

' Funktionen deklarieren
DEF FNf(x)=SIN(x)/x
x = 0 : y = 0 : xr% = 0 : yr% = 0

SUB rot.transform(x, y, w)
	SHARED xr%,yr%
    xr% = x*COS(w) +y*SIN(w)
    yr% = -x*SIN(w) +y*COS(W)
END SUB

' Funktionsparameter
a = 320		' x-Koordinate des Nullpunktes
b = 250		' y-Koordinate des Nullpunktes
f1 = 10		' x-Vergrößerungsfaktor
f2 = 50		' y-Vergrößerungsfaktor

' Fenster öffnen
WINDOW 1,"Skalierung",(0,0)-(640,400),31
COLOR 2,1
CLS

' Schleife zur Veränderung des Rotations-Winkels
FOR wi=0 TO 30 STEP 2
	' Winkel in RAD umrechnen
    w = wi/180 * 3.141593

	' y-Koordinatenachse einzeichnen
    COLOR 3
    x = a : y = 399 : CALL rot.transform(x, y, w)
    PSET(xr%,yr%)
    x = a : y = 0 : CALL rot.transform(x, y, w)
    LINE STEP (xr%,yr%)

	' x-Koordinatenachse einzeichnen
    x = 0 : y = b : CALL rot.transform(x, y, w)
    PSET(xr%,yr%)
    x = 640 : y = b : CALL rot.transform(x, y, w)
    LINE STEP (xr%,yr%)

    COLOR 2
    x = 0
    y = b-f2*FNf((x-a)/f1)	' Funktionswert für x = 0 berechnen
    CALL rot.transform(x, y, w)
    PSET (xr%,yr%)

    ' Berechnung der Funktionswerte
    FOR x=0 TO 639
    	IF x-a <> 0 THEN
    		y = b-f2*FNf((x-a)/f1)
        	CALL rot.transform(x, y, w)
            LINE STEP (xr%,yr%)
        END IF
    NEXT x
NEXT wi

WHILE INKEY$=""
	SLEEP
WEND

WINDOW CLOSE 1
END

