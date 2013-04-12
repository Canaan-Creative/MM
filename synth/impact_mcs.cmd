setMode -bs
setCable -p auto
setCableSpeed -speed 6000000
Identify -inferir 
identifyMPM 
attachflash -position 1 -spi "W25Q80BV"
assignFileToAttachedFlash -p 1 -file mm.mcs
erase -p 1 -dataWidth 1 -spionly
Program -p 1 -dataWidth 1 -spionly -e -v -loadfpga 
closeCable
quit
