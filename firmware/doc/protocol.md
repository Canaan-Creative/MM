# Protocol
Between cgminer and FPGA controller

# Physical link
Multiple FPGA controllers are daisy-chained together using TTL UART.

# Packet structure
Length: 39Bytes

Format: |2B:HEAD|1B:TYPE|1B:IDX|1B:CNT|32B:DATA|2B:CRC|

HEAD: 'A' 'V'
TYPE: Please read this head file: [protocol.h](https://github.com/BitSyncom/mm/blob/master/firmware/protocol.h)

# Conceptual model
## Detect MM controller
1. P_DETECT: send out this package, attach the modular id at the end of pakcage
2. P_ACTDETECT: mm will send back this package, include teh MM version in this package

## Cgminer broadcasts the stratum information to all MM controllers
1. P_STATIC: Send the length info first, the package like: coinbase length, nonce2 offset, nonce2 size, merkle offset, merkles count, pool diff, pool no. each variable using 32bits. the P_STATIC will make MM stop generate works. until a P_SET pkg send out.
2. P_TARGET: Send out the package, 
3. P_JOB_ID: Send out the stratum package
4. P_COINBASE: Send out the whole coinbase, split by 32Bytes, we using IDX/CNT here. max length: **6KB**
5. P_MERKLES: Send out the merkels one by one , we using IDX/CNT here. max count: **20**
6. P_HEADER: Send out the block header of the stratum message.
7. P_SET: Send the MM configurations: fan pwm, chip voltage, chip frequency, nonce2 start, nonce2 range, each variable using 32bits. the P_SET will trigger MM to start generate works.

## Cgminer will polling the MM controllers
The mm controller selects its own partition of extranonce in coinbase, base on own modular id and nonce2 start and nonce range. there are 2 type of packages send back 
1. P_POLLING: 
2. P_NONCE: miner id, pool no, nonce2, nonce, job id, attach the modular id at the end of package.
3. P_STATUS: temp, fan, frequency, voltage, local works, hardware error works, attach the modular id at the end of pakcage.