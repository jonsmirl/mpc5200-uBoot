/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <regs.h>
#include <asm/io.h>


/* ------------------------------------------------------------------------- */
#define SMSC9220_Tacs	(0x0)	// 0clk		address set-up
#define SMSC9220_Tcos	(0x4)	// 4clk		chip selection set-up
#define SMSC9220_Tacc	(0xe)	// 14clk	access cycle
#define SMSC9220_Tcoh	(0x1)	// 1clk		chip selection hold
#define SMSC9220_Tah	(0x4)	// 4clk		address holding time
#define SMSC9220_Tacp	(0x6)	// 6clk		page mode access cycle
#define SMSC9220_PMC	(0x0)	// normal(1data)page mode configuration

#define SROM_DATA16_WIDTH(x)	(1<<((x*4)+0))
#define SROM_ADDR_MODE_16BIT(x)	(1<<((x*4)+1))
#define SROM_WAIT_ENABLE(x)	(1<<((x*4)+2))
#define SROM_BYTE_ENABLE(x)	(1<<((x*4)+3))

#define Inp32(_addr)		readl(_addr)
#define Outp32(addr, data)	(*(volatile u32 *)(addr) = (data))

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */
static void smsc9220_pre_init(int bank_num)
{
	unsigned int tmp;
//	unsigned char smc_bank_num=1;

	/* gpio configuration */
	tmp = MP01CON_REG;
	tmp &= ~(0xf << bank_num*4);
	tmp |= (0x2 << bank_num*4);
	MP01CON_REG = tmp;

	tmp = SROM_BW_REG;
	tmp &= ~(0xF<<(bank_num * 4));
	tmp |= SROM_DATA16_WIDTH(bank_num);
	tmp |= SROM_ADDR_MODE_16BIT(bank_num);
	SROM_BW_REG = tmp;

	if(bank_num == 0)
		SROM_BC0_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 1)
		SROM_BC1_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 2)
		SROM_BC2_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 3)
		SROM_BC3_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	smsc9220_pre_init(1);

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

#if defined(PHYS_SDRAM_2)
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif

	return 0;
}

#ifdef BOARD_LATE_INIT
extern int default_boot_mode;

int board_late_init (void)
{
	unsigned int regs;
	char boot_cmd[100];

	regs = Inp32(INF_REG_BASE + INF_REG3_OFFSET);
	Outp32(0xe02002a0, 0x10000000);
	Outp32(0xe02002c0, 0x1);

	switch(regs) {
	case BOOT_NAND:
		Outp32(0xe02002a4, 0x0);	// LED0 On
		Outp32(0xe02002c4, 0x1);	// LED1 Off

         	printf("checking mode for fastboot ...\n");

         	if((~readl(0xE0200C04)) & 0x6) {
         		run_command("fastboot", 0);
                } else
		if(default_boot_mode) {		// using default environment
			sprintf(boot_cmd, "nand read %08x 600000 400000;nand read %08x B00000 300000; bootm %08x %08x"
					, MEMORY_BASE_ADDRESS + 0x8000
					, MEMORY_BASE_ADDRESS + 0x1000000
					, MEMORY_BASE_ADDRESS + 0x8000
					, MEMORY_BASE_ADDRESS + 0x1000000);
			setenv("bootcmd", boot_cmd);

			sprintf(boot_cmd, "root=ramfs devfs=mount console=ttySAC2,115200");
			setenv("bootargs", boot_cmd);
		}
		break;
	case BOOT_MMCSD:
		Outp32(0xe02002a4, 0x80);	// LED1 On
		Outp32(0xe02002c4, 0x0);	// LED0 Off

		if((~Inp32(0xe0200c04)) & 0x6) { // Linux Recovery Booting Mode
			sprintf(boot_cmd, "nand erase clean;nand scrub;movi read u-boot %08x;nand write %08x 0 70000;movi read kernel %08x;bootm %08x"
					, MEMORY_BASE_ADDRESS + 0x1000000, MEMORY_BASE_ADDRESS + 0x1000000
					, MEMORY_BASE_ADDRESS + 0x8000, MEMORY_BASE_ADDRESS + 0x8000);
			setenv("bootcmd", boot_cmd);
			sprintf(boot_cmd, "root=/dev/mmcblk0p3 rootfstype=ext3 console=ttySAC2,115200 rootdelay=1 recovery");
			setenv("bootargs", boot_cmd);
			sprintf(boot_cmd, "0");
			setenv("bootdelay", boot_cmd);
		} else
		if(default_boot_mode) {	// using default environment
			sprintf(boot_cmd, "movi read kernel %08x; bootm %08x"
					, MEMORY_BASE_ADDRESS + 0x8000
					, MEMORY_BASE_ADDRESS + 0x8000);
			setenv("bootcmd", boot_cmd);
			sprintf(boot_cmd, "root=/dev/mmcblk0p4 rootfstype=ext3 console=ttySAC2,115200 rootdelay=1");	// Android Booting Mode
			setenv("bootargs", boot_cmd);
		}
		break;
	}

	//LCD_turnon();
	return 0;
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard:   ARIES\n");
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdkc110(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xd0000000))
		return (addr - 0xc0000000 + 0x20000000);
	else
		printf("The input address don't need "\
			"a virtual-to-physical translation : %08lx\n", addr);

	return addr;
}

#endif

#if defined(CONFIG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
void nand_init(void)
{
	nand_probe(CFG_NAND_BASE);
        if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
                print_size(nand_dev_desc[0].totlen, "\n");
        }
}
#endif
