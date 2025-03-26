;Multiplies two numbers and stores it in var3
loop:
ldl var2
ldc 1
sub

brlz end
stl var2

ldl var1
ldl var3
add
stl var3

br loop

end:
HALT
var1:data 6
var2:data 3
var3:data 0