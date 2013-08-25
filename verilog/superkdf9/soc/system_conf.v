`define LATTICE_FAMILY "EC"
`define LATTICE_FAMILY_EC
`define LATTICE_DEVICE "All"
`ifndef SYSTEM_CONF
`define SYSTEM_CONF
`timescale 1ns / 100 ps
`define CFG_EBA_RESET 32'h0
`define MULT_ENABLE
`define CFG_PL_MULTIPLY_ENABLED
`define SHIFT_ENABLE
`define CFG_PL_BARREL_SHIFT_ENABLED
`define CFG_IROM_ENABLED
`define CFG_IROM_EXPOSE
`define CFG_IROM_BASE_ADDRESS 32'h0000
`define CFG_IROM_LIMIT 32'h3fff
`define CFG_IROM_INIT_FILE_FORMAT "hex"
`define CFG_IROM_INIT_FILE "none"
`define CFG_DRAM_ENABLED
`define CFG_DRAM_EXPOSE
`define CFG_DRAM_BASE_ADDRESS 32'h4000
`define CFG_DRAM_LIMIT 32'h5fff
`define CFG_DRAM_INIT_FILE_FORMAT "hex"
`define CFG_DRAM_INIT_FILE "none"
`define LM32_I_PC_WIDTH 16
`define CFG_USER_ENABLED
`define uartUART_WB_DAT_WIDTH 8
`define uartUART_WB_ADR_WIDTH 4
`define uartCLK_IN_MHZ 0
`define uartBAUD_RATE 115200
`define IB_SIZE 32'h20
`define OB_SIZE 32'h20
`define BLOCK_WRITE
`define BLOCK_READ
`define RXRDY_ENABLE
`define TXRDY_ENABLE
`define INTERRUPT_DRIVEN
`define CharIODevice
`define uartLCR_DATA_BITS 8
`define uartLCR_STOP_BITS 1
`define uartFIFO
`define spiMASTER
`define spiSLAVE_NUMBER 32'h1
`define spiCLOCK_SEL 0
`define spiCLKCNT_WIDTH 6
`define DELAY_TIME 3
`define spiINTERVAL_LENGTH 2
`define spiDATA_LENGTH 8
`define spiSHIFT_DIRECTION 0
`define spiCLOCK_PHASE 0
`define spiCLOCK_POLARITY 0
`define gpioGPIO_WB_DAT_WIDTH 32
`define gpioGPIO_WB_ADR_WIDTH 4
`define gpioBOTH_INPUT_AND_OUTPUT
`define gpioDATA_WIDTH 32'h1
`define gpioINPUT_WIDTH 32'h20
`define gpioOUTPUT_WIDTH 32'h20
`define gpioIRQ_MODE
`define gpioEDGE
`define gpioEITHER_EDGE_IRQ
`define uart_debugUART_WB_DAT_WIDTH 8
`define uart_debugUART_WB_ADR_WIDTH 4
`define uart_debugCLK_IN_MHZ 0
`define uart_debugBAUD_RATE 115200
`define IB_SIZE 32'h20
`define OB_SIZE 32'h20
`define INTERRUPT_DRIVEN
`define CharIODevice
`define uart_debugLCR_DATA_BITS 8
`define uart_debugLCR_STOP_BITS 1
`define uart_debugFIFO
`endif // SYSTEM_CONF
