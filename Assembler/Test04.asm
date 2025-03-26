;Finding the quotient of 100 divided by 11

loop:
ldl var1
ldl var2
sub

;Go to end
brlz end
brz end
stl var1


ldc 1
ldl var3
add
stl var3

br loop


end:
HALT
var1: data 100
var2: data 11
var3: data 0

