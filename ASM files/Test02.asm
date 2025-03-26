;Stores the largest number in var3
ldl var1
ldl var2
sub
brlz secondlarge
ldl var1
stl var3
br end

secondlarge:
ldl var2
stl var3


end:
HALT
var1: data 5
var2: data 8
var3: data 0