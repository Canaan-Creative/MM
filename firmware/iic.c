/*
 * Author: Xiangfu <xiangfu@openmobilefree.net>
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "intr.h"
#include "io.h"

#define IIC_RINGBUFFER_SIZE_RX 128
#define IIC_RINGBUFFER_MASK_RX (IIC_RINGBUFFER_SIZE_RX-1)
static unsigned int rx_buf[IIC_RINGBUFFER_SIZE_RX];
static volatile unsigned int rx_produce;
static volatile unsigned int rx_consume;

#define IIC_PACKSIZE	40
static struct lm32_iic *iic = (struct lm32_iic *)IIC_BASE;
static struct lm32_dna *dna = (struct lm32_dna *)DNA_BASE;

void iic_isr(void)
{
	writel(LM32_IIC_CR_RX_INTR_MASK_SET, &iic->ctrl);

	while (readl(&iic->ctrl) & (LM32_IIC_CR_RX_CNT)) {
		rx_buf[rx_produce] = readl(&iic->rx);
		rx_produce = (rx_produce + 1) & IIC_RINGBUFFER_MASK_RX;
	}

	writel(LM32_IIC_CR_RX_INTR_MASK_CLEAR, &iic->ctrl);

	irq_ack(IRQ_IIC);
}

void iic_dna_read(uint8_t *dnadat)
{
	uint32_t i, tmp, val;
	uint32_t *pdnadat = (uint32_t *)dnadat;

	memset(dnadat, 0xff, 8);

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

uint32_t iic_write(uint8_t *data, uint16_t len)
{
	uint32_t tmp, i, j = 0, ret = 0;
	uint32_t *pdat = (uint32_t *)data;

	len = len >> 2;
	for (i = 0; i < len; i++)
		writel(pdat[i], &iic->tx);

	tmp = readl(&iic->ctrl);
	while (!(tmp & (LM32_IIC_CR_RSTOP | LM32_IIC_CR_RERR))) {
		if (tmp & LM32_IIC_CR_RERR) {
			debug32("D: IIC_CR_RERR\n");
			ret = 0;
			break;
		}

		if (tmp & LM32_IIC_CR_RSTOP) {
			ret = len << 2;
			break;
		}

		if (++j > 0xfffff) {
			debug32("D: iic write timeout\n");
			ret = 0;
			break;
		}

		tmp = readl(&iic->ctrl);
	}

	writel((tmp & (LM32_IIC_CR_RSTOP | LM32_IIC_CR_RERR)), &iic->ctrl);

	return ret;
}

int iic_read_nonblock(void)
{
	return (rx_consume != rx_produce);
}

uint32_t iic_read()
{
	uint32_t d;

	while (rx_consume == rx_produce);
	d = rx_buf[rx_consume];
	rx_consume = (rx_consume + 1) & IIC_RINGBUFFER_MASK_RX;

	return d;
}

void iic_addr_set(uint8_t addr)
{
	uint32_t tmp = addr & 0x7f;
	writel(tmp, &iic->addr);
}

uint8_t iic_addr_get(void)
{
	return readl(&iic->addr) & 0x7f;
}

void iic_init(void)
{
	uint32_t mask;

	rx_produce = 0;
	rx_consume = 0;

	irq_ack(IRQ_IIC);

	/* Enable IIC interrupts */
	writel(LM32_IIC_CR_RX_INTR_MASK_CLEAR, &iic->ctrl);
	mask = irq_getmask();
	mask |= IRQ_IIC;
	irq_setmask(mask);
}

void iic_logic_reset(void)
{
	writel(LM32_IIC_CR_LOGICRESET, &iic->ctrl);
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
	uint8_t data[IIC_PACKSIZE];
	uint32_t rxlen;
	uint32_t slv_addr = 0;

	debug32("D: IIC test\n");
	iic_init();

	while (1) {
		rxlen = iic_read_cnt();
		if (rxlen && !iic_read(data, rxlen)) {
			debug32("D: RX data(%d):\n", rxlen);
			hexdump(data, rxlen);

			if (data[3] == IIC_ADDR) {
				if (slv_addr) {
					debug32("Slave, %x\n", iic_addr_get());
					continue;
				} else {
					slv_addr = 1;
					iic_addr_set(data[7]);
					debug32("Set slave addr: %x\n", data[7]);
				}
				iic_dna_read(data);
				iic_write(data, IIC_PACKSIZE);
				debug32("D: TX data (DNA):\n");
				hexdump(data, IIC_PACKSIZE);
			} else if (data[3] == IIC_LOOP) {
				iic_write(data, IIC_PACKSIZE);
				debug32("D: TX data:\n");
				hexdump(data, IIC_PACKSIZE);
			}
		}
	}
}
#endif
