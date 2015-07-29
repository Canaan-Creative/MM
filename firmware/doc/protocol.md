# Protocol
Between CGMiner and FPGA controller/Avalon miner

# Physical link
Multiple FPGA controllers are daisy-chained together using IIC.

Avalon miner is a standalone device.

# Packet structure
Length: 40Bytes

Format: |2B:HEAD|1B:TYPE|1B:OPT|1B:IDX|1B:CNT|32B:DATA|2B:CRC|

1. HEAD: 'A' 'V'
2. TYPE: Please read this head file: [protocol.h](https://github.com/Canaan-Creative/mm/blob/avalon4/firmware/protocol.h)
3. IDX: This package index of the whole packages count.
4. CNT: Packages count
5. DATA: The real data
6. CRC: CRC-16

# Conceptual model
### Detect MM controller/Avalon miner
1. P_DETECT: send out this package to MM/Avalon miner, attach the modular id at the end of pakcage
2. P_ACTDETECT: MM/Avalon miner will send back this package with the following. cpuid/dna(8B), version(15B), asic count(4)

### Send jobs to ASIC
#### CGMiner broadcasts the stratum information to all MM controllers
1. P_STATIC: Send the length info first, the package like: coinbase length, nonce2 offset, nonce2 size, merkle offset, merkles count, pool diff, pool no. each variable using 32bits. the P_STATIC will make MM stop generate works. until a P_SET pkg send out.
2. P_TARGET: Send out the package,
3. P_JOB_ID: Send out the stratum package
4. P_COINBASE: Send out the whole coinbase, split by 32Bytes, we using IDX/CNT here. max length: **6KB**
5. P_MERKLES: Send out the merkels one by one , we using IDX/CNT here. max count: **20**
6. P_HEADER: Send out the block header of the stratum message.

#### CGMiner send an icarus-like package to Avalon miner
1. P_WORK: It is an icarus-like package. It contains two packets.

	IDX1: all data is midstate(icarus midstate byte order);

	IDX2: id(6B), reserved(2B), ntime(1B), fan(3B), led(4B), reserved(4), data(12)

### CGMiner will polling the MM controllers/Avalon miner
The MM controller selects its own partition of extranonce in coinbase, base on own modular id and nonce2 start and nonce range. there are 2 type of packages send back

1. P_POLLING:
2. P_NONCE: get nonce from MM: miner id(4B), pool no(4B), nonce2(4B), ntime(4B), nonce(4B), job id(4B), attach the modular id at the end of package.
3. P_STATUS: get status from MM: temp(4B), fan(4B), frequency(4B), voltage(4B), local works(4B), hardware error works(4B), power good(4B) attach the modular id at the end of pakcage.
4. P_NONCE_M: get nonce from Avalon miner: It contains two nonces maximum, if data is invalid then it will fill with 0xff.

	id/(6B), chipid(1B), ntime(1B), nonce(4B), reserved(1B), usb ringbuf count(1B), work ringbuf count(1B), nonce ringbuf count(1B)
5. P_STATUS_M: get status from Avalon miner.
	* Avalon4 mini format:

      spi speed(4B), led(4B), fan(4B), voltage(4B), adc_12v(4B), adc_copper(4B), adc_fan(4B), power good(4B)
	* Avalon nano 2.0 format:

      frequency(4B), led(4B), reserved(4B), v2_5(4B), vcore(4B), temp(4B), v1_8 & V0_9(4B), power good(4B)

### Device configurations and status
1. P_SET: Send the MM configurations: fan pwm, chip voltage, chip frequency, nonce2 start, nonce2 range, each variable using 32bits. the P_SET will trigger MM to start generate works.
2. P_SETM: Send the Avalon miner configurations: spi speed(4B), led(4B), fan(4B), asic index(4)
3. P_SET_VOLT: voltage(2B) x miner count; opt is used for mining mode and save flag, high 4 bit is save flag(1:save, 0:don't save), low 4 bit is mining mode(0:CUSTOM, 1:ECO, 2:NORMAL, 3:TURBO)
4. P_SET_FREQ: pll1(4B), pll2(4B), pll3(4B); opt is used for asic tweak, if 0 then all asics use the same settings, else the asic with opt index used the value
5. P_GET_VOLT:
6. P_STATUS_VOLT: voltage(2B) x miner count; opt is used for miningmode, low 4 bit is mining mode(0:CUSTOM, 1:ECO, 2:NORMAL, 3:TURBO)
7. P_GET_FREQ:
7. P_STATUS_FREQ: pll1(4B), pll2(4B), pll3(4B); opt is used for asic index(1-?), 0 is used for all asics.

# Module ID
MM controller: Each module have it's own ID. All ID was attach to the end of the data.

Avalon miner: It's a standalone device id with id 0.
