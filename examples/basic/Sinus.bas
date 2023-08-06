' Testprogramm 1

window 1,"Ellipse 1",(0,0)-(640,400),6

font "topaz",8

menu 1,0,1,"Project"
menu 1,1,1,"Quit","Q"
on menu gosub quit
menu on

color 2,1
cls

FOR x=0 TO 639
    PSET (x,50*SIN(x/40)+100)
NEXT x

while true
wend

quit: 
  if menu(0) <> 1 or menu(1) <> 1 then return 
  window close 1
END
