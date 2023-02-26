COLOR 2
CLS

FOR x = 0 TO 550 STEP 2
    x1 = x
    y1 = 40*sin(x/40)+80
    x2 = 200*sin(x/40)+250
    y2 = 80*sin(x/50)+110
    LINE(x1,y1)-(x2,y2)
NEXT x

WHILE INKEY$=""
    SLEEP
WEND

CLS
FOR x = 1 TO 250
    FOR y = -100 TO 100
        w = ATN(y/x)
        r = SQR(x*x+y*y)
        h = .5 + .5 *SIN(2*w+LOG(r)*10)
        IF h >= RND(1) THEN
            PSET (250-x, 120+y)
            PSET (249+x, 120-y)
        END IF
    NEXT y
NEXT x
WHILE INKEY$=""
    SLEEP
WEND

CLS

