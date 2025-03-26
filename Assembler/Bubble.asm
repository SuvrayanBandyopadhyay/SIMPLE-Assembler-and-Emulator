;Store address of the first element in begin
ldc arr0
stl begin

outer: ldl size
ldl i
sub 
brz end ; We check if i==size, (program is over)

;Initializing inner loop
ldc 0
stl j

inner: ldl size
ldc 1
sub ; Store size -1
ldl i ;store i
sub ;Store size -1 -i
ldl j ;Get j
sub ; Subtract
brz break ;Leave inner loop if they are equal
brlz break ; If somehow it becomes less than zero (Check values if this happens)

ldl j ; Get j 
ldl begin ;Get adress of array
add ; Get jth position address
ldnl 0 ;Load value 
stl temp1 ; Store value

ldl j ;Get j
ldc 1  ;Get 1
add ;Add
ldl begin 
add ;Get j+1 th element
ldnl 0 ;Load non local 

stl temp2

;Compare the two variables
ldl temp1
ldl temp2
sub
brlz nochange

;Swap
ldl j ; Get j 
ldl begin ;Get adress of array
add ; Get jth position address
stl addrtemp
ldl temp2
ldl addrtemp
stnl 0

ldl j ;Get j
ldc 1  ;Get 1
add ;Add
ldl begin 
add ;Get j+1 th element
stl addrtemp
ldl temp1
ldl addrtemp
stnl 0


nochange:
ldl j
ldc 1
add
stl j
br inner

break:
ldl i
ldc 1
add
stl i
br outer

end:
HALT
arr0: data 0129
arr1: data 5
arr2: data 2
arr3: data 8
arr4: data 100
arr5: data 21
size: data 6
i: data 0
j: data 0
temp1: data 0
temp2: data 0
begin: data 0
addrtemp: data 0