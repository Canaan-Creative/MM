setmode -bscan
setCable -p auto
setCableSpeed -speed 12000000
addDevice -p 1 -file mm.bit
program -p 1
closeCable
quit
