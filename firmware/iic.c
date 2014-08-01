/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"

#define IIC_PACKSIZE	40

static unsigned char dnadat[IIC_PACKSIZE];
static struct lm32_iic *iic = (struct lm32_iic *)IIC_BASE;
static struct lm32_dna *dna = (struct lm32_dna *)DNA_BASE;

static void iic_dna_read(unsigned char *dnadat)
{
	unsigned int i, tmp, val;
	unsigned int *pdnadat = (unsigned int *)dnadat;

	pdnadat[0] = 0;
	pdnadat[1] = 0;

	val = ~(LM32_DNA_CLK | LM32_DNA_DIN | LM32_DNA_READ | LM32_DNA_SHIFT | LM32_DNA_DOUT);
	val &= LM32_DNA_MASK;
	writel(val, &dna->dna);

	val = LM32_DNA_READ;
	writel(val, &dna->dna);

	val |= LM32_DNA_CLK;
	writel(val, &dna->dna);

	tmp = readl(&dna->dna);
	pdnadat[1] = (pdnadat[1]<<1) | ((tmp >> 4)&1);

	val = LM32_DNA_READ;
	writel(val, &dna->dna);

	val = ~(LM32_DNA_CLK | LM32_DNA_DIN | LM32_DNA_READ | LM32_DNA_SHIFT | LM32_DNA_DOUT);
	val &= LM32_DNA_MASK;
	writel(val, &dna->dna);
	for (i = 1; i < 32; i++) {
		val = LM32_DNA_SHIFT;
		writel(val, &dna->dna);

		val |= LM32_DNA_CLK;
		writel(val, &dna->dna);

		tmp = readl(&dna->dna);
		pdnadat[1] = (pdnadat[1]<<1) | ((tmp >> 4)&1);

		val = LM32_DNA_SHIFT;
		writel(val, &dna->dna);
	}
	val = ~(LM32_DNA_CLK | LM32_DNA_DIN | LM32_DNA_READ | LM32_DNA_SHIFT | LM32_DNA_DOUT);
	val &= LM32_DNA_MASK;

	writel(val, &dna->dna);

	for (; i < 57; i++) {
		val = LM32_DNA_SHIFT;
		writel(val, &dna->dna);

		val |= LM32_DNA_CLK;
		writel(val, &dna->dna);

		pdnadat[0] = (pdnadat[1]>>31) | (pdnadat[0] << 1);
		pdnadat[1] = (pdnadat[1]<< 1) | ((readl(&dna->dna) >> 4)&1);

		val = LM32_DNA_SHIFT;
		writel(val, &dna->dna);
	}
}

uint16_t iic_write(uint8_t *data, uint16_t len)
{
	unsigned int tmp;
	uint16_t i, txlen;
	unsigned int *pdat = (unsigned int *)data;

	len = len >> 2;
	for (i = 0; i < len; i++)
		writel(pdat[i], &iic->tx);

	while (1) {
		tmp = readl(&iic->ctrl);
		tmp = tmp & (LM32_IIC_CR_RSTOP | LM32_IIC_CR_RERR);
		if(tmp)
			break;
	}

	writel(tmp, &iic->ctrl);
	tmp = tmp >> 19;

	if ((tmp & LM32_IIC_CR_RERR) == LM32_IIC_CR_RERR) {
		writel(LM32_IIC_CR_TXFIFORESET, &iic->ctrl);
		writel(LM32_IIC_CR_LOGICRESET, &iic->ctrl);
		return 0;
	}

	txlen = readl(&iic->ctrl);
	txlen = (txlen >> LM32_IIC_CR_TXFIFOOFFSET) & 0x1ff;
	return txlen;
}

uint16_t iic_read_cnt(void)
{
	uint16_t 	rxlen;
	int		tmp;

	tmp = readl(&iic->ctrl);
	tmp = tmp & LM32_IIC_CR_WSTOP;
	if (!tmp)
		return 0;

	writel(tmp, &iic->ctrl);

	rxlen = readl(&iic->ctrl);
	rxlen = rxlen & 0x1ff;
	return rxlen << 2;
}

int iic_read(uint8_t *data, uint16_t len)
{
	unsigned int i;
	unsigned int *pdat = (unsigned int *)data;

	len = len >> 2;
	for (i = 0; i < len; i++)
		pdat[i] = readl(&iic->rx);

	return 0;
}

void iic_addr_set(unsigned char addr)
{
	unsigned int tmp;
	tmp = addr & 0x7f;
	writel(addr, &iic->addr);
}

unsigned char iic_addr_get(void)
{
	return readl(&iic->addr) & 0x7f;
}

void iic_init(void)
{
	memset(dnadat, 0xff, sizeof(dnadat));
	iic_dna_read(dnadat);
	debug32("D: DNA:\n");
	hexdump(dnadat, 8);
}

void iic_logic_reset(void)
{
	writel(LM32_IIC_CR_TXFIFORESET, &iic->ctrl);
}

void iic_rx_reset(void)
{
	writel(LM32_IIC_CR_RXFIFORESET, &iic->ctrl);
}

void iic_tx_reset(void)
{
	writel(LM32_IIC_CR_TXFIFORESET, &iic->ctrl);
}

#ifdef DEBUG_IIC_TEST
#define IIC_ADDR 	0
#define IIC_LOOP 	1
void iic_test(void)
{
	unsigned char data[IIC_PACKSIZE];
	uint16_t rxlen, i;
	unsigned int slv_addr = 0;

	debug32("D: IIC Test\n");
	iic_init();

	while (1) {
		rxlen = iic_read_cnt();
		if (rxlen && !iic_read(data, rxlen)) {
			debug32("RX Data(%d):", rxlen);
			for (i = 0; i < rxlen; i++)
				debug32("%02x", i, data[i]);
			debug32("\n");

			if (data[3] == IIC_ADDR) {
				if (slv_addr) {
					debug32("This is a Named slave, %x\n", iic_addr_get());
					continue;
				} else {
					slv_addr = 1;
					iic_addr_set(data[7]);
					debug32("Set slave addr, %x\n", data[7]);
				}
				iic_write((unsigned char*)dnadat, IIC_PACKSIZE);
				debug32("TX Data (DNA):");
				for (i = 0; i < IIC_PACKSIZE; i++)
					debug32("%02x", dnadat[i]);
				debug32("\n");
			} else if (data[3] == IIC_LOOP) {
				iic_write(data, IIC_PACKSIZE);
				debug32("TX Data:");
				for (i = 0; i < IIC_PACKSIZE; i++)
					debug32("%02x", i, data[i]);
				debug32("\n");
			}
		}
	}
}
#endif
