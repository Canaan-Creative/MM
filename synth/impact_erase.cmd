setMode -bs
setCable -p auto
setCableSpeed -speed 6000000
Identify -inferir 
identifyMPM 
erase -p 1 -dataWidth 1 -spionly -spi "W25Q80BV"
closeCable
quit
