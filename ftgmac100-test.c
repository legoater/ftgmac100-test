// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * FTGMAC100 VLAN tester
 *
 * Copyright (c) 2020 Cédric Le Goater, IBM Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/io.h>

#include "ftgmac100.h"

#define FTGMAC100_IOMEM 0x1e660000

static unsigned long tx_desc_entries = 1;
module_param(tx_desc_entries, ulong, S_IRUGO);
MODULE_PARM_DESC(tx_desc_entries, "Number of TX descriptors (default = 1)");

static unsigned long bad_address = 0x0;
module_param(bad_address, ulong, S_IRUGO);
MODULE_PARM_DESC(bad_address, "Bad frame address (default = 0)");

static unsigned long bad_size = 1;
module_param(bad_size, ulong, S_IRUGO);
MODULE_PARM_DESC(bad_size, "Bad frame size (default = 1)");

static bool insert_vlan = true;
module_param(insert_vlan, bool, S_IRUGO);
MODULE_PARM_DESC(insert_vlan, "Insert VLAN (default true)");

static struct ftgmac100 {
	void __iomem *base;

	struct ftgmac100_txdes *txdes;
	dma_addr_t txdes_dma;
} ftgmac100;

static int __init ftgmac100_test_init(void)
{
	struct ftgmac100 *priv = &ftgmac100;
	u32 maccr, nptxr_badr;
	int i;
	
	pr_info("%s: #tx=%ld sz=%ld @%08lx %s\n", __func__,
	       tx_desc_entries, bad_size, bad_address,
	       insert_vlan ? "VLAN" : "");

	priv->base = ioremap(FTGMAC100_IOMEM, 0x1000);
	
	priv->txdes = kcalloc(tx_desc_entries, sizeof(struct ftgmac100_txdes),
			      GFP_KERNEL);

	for (i = 0; i < tx_desc_entries; i++) {
		priv->txdes[0].txdes0 =
			FTGMAC100_TXDES0_TXBUF_SIZE(bad_size) |
			FTGMAC100_TXDES0_TXDMA_OWN;
		
		if (i == 0)
			priv->txdes[0].txdes0 |= FTGMAC100_TXDES0_FTS;
		if (i == tx_desc_entries - 1)
			priv->txdes[0].txdes0 |= FTGMAC100_TXDES0_LTS;

		if (insert_vlan) 
			priv->txdes[0].txdes1 = FTGMAC100_TXDES1_INS_VLANTAG;
		
		priv->txdes[0].txdes2 = 0; /* Unused */
		priv->txdes[0].txdes3 = bad_address;
	}
	
	priv->txdes_dma = virt_to_phys(priv->txdes);

	/* Enable transmit */ 
	maccr = ioread32(priv->base + FTGMAC100_OFFSET_MACCR);
	maccr |= FTGMAC100_MACCR_TXDMA_EN | FTGMAC100_MACCR_TXMAC_EN;
	iowrite32(maccr, priv->base + FTGMAC100_OFFSET_MACCR);

	/* Register Bogus TX descriptor */ 
	nptxr_badr = ioread32(priv->base + FTGMAC100_OFFSET_NPTXR_BADR);
	iowrite32(priv->txdes_dma, priv->base + FTGMAC100_OFFSET_NPTXR_BADR);

	/* Trigger write */ 
	iowrite32(0x0, priv->base + FTGMAC100_OFFSET_NPTXPD);
	
	pr_info("%s: ISR=%08x\n", __func__,
	       ioread32(priv->base + FTGMAC100_OFFSET_ISR));

	/* Restore previous state. */
	iowrite32(nptxr_badr, priv->base + FTGMAC100_OFFSET_NPTXR_BADR);

	pr_info("%s: done\n", __func__);
	return 0;
}

static void __init ftgmac100_test_exit(void)
{
	struct ftgmac100 *priv = &ftgmac100;

	pr_info("%s\n", __func__);

	kfree(priv->txdes);
	iounmap(priv->base);
}

module_init(ftgmac100_test_init);
module_exit(ftgmac100_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cédric Le Goater <clg@kaod.org>");
MODULE_DESCRIPTION("FTGMAC100 tester");
