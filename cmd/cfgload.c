/*
 * Copyright (C) 2018 Wesion Co,. Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <linux/errno.h>
#include <malloc.h>
#include <linux/ctype.h>    /* isalpha, isdigit */
#include <linux/sizes.h>

#ifdef CONFIG_HUSH_PARSER
#include <cli_hush.h>
#endif

#define BOOTINI_MAGIC	"KHADAS-UBOOT-CONFIG"
#define SZ_BOOTINI		SZ_64K

/* Nothing to proceed with zero size string or comment.
 *
 * FIXME: Do we really need to strip the line start with '#' or ';',
 *        since any U-boot command does not start with punctuation character.
 */
static int valid_command(const char* p)
{
	char *q;

	for (q = (char*)p; *q; q++) {
		if (isblank(*q)) continue;
		if (isalnum(*q)) return 1;
		if (ispunct(*q))
			return (*q != '#') && (*q != ';');
	}

	return !(p == q);
}

/* Read boot.ini from FAT/EXT4 partition
 */
static char* read_cfgload(void)
{
	char cmd[128];
	unsigned long filesize;
	char *p;

	p = (char *)simple_strtoul(env_get("loadaddr"), NULL, 16);
	if (NULL == p)
		p = (char *)CONFIG_SYS_LOAD_ADDR;

	env_set("filesize", "0");

	// Try to load fatfs partition
	sprintf(cmd, "fatload mmc 0:1 0x%p boot.ini", (void *)p);
	run_command(cmd, 0);

	filesize = env_get_ulong("filesize", 16, 0);
	if (0 == filesize) {
		printf("cfgload: fatload: no boot.ini or empty file\n");

		// Try to load ext4 partition
		sprintf(cmd, "ext4load mmc 0:5 0x%p /boot/boot.ini", (void *)p);
		run_command(cmd, 0);

		filesize = env_get_ulong("filesize", 16, 0);
		if (0 == filesize) {
			printf("cfgload: ext4load: no boot.ini or empty file\n");

			return NULL;
		}
	}

	if (filesize > SZ_BOOTINI) {
		printf("boot.ini: 'boot.ini' exceeds %d, size=%ld\n",
				SZ_BOOTINI, filesize);
		return NULL;
    }

	/* Terminate the read buffer with '\0' to be treated as string */
	*(char *)(p + filesize) = '\0';

	/* Scan MAGIC string, readed boot.ini must start with exact magic string.
	 * Otherwise, we will not proceed at all.
	 */
	while (*p) {
		char *s = strsep(&p, "\n");
		if (!valid_command(s))
			continue;

		/* MAGIC string is discovered, return the buffer address of next to
		 * proceed the commands.
		 */
		if (!strncmp(s, BOOTINI_MAGIC, sizeof(BOOTINI_MAGIC)))
			return memcpy(malloc(filesize), p, filesize);
	}

	printf("cfgload: MAGIC NAME, %s, is not found!!\n", BOOTINI_MAGIC);

	return NULL;
}

static int do_load_cfgload(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	char *p;
	char *cmd;

	p = read_cfgload();
	if (NULL == p)
		return 0;

	printf("cfgload: applying boot.ini...\n");

	while (p) {
		cmd = strsep(&p, "\n");
		if (!valid_command(cmd))
			continue;

		printf("cfgload: %s\n", cmd);

#ifndef CONFIG_SYS_HUSH_PARSER
		run_command(cmd, 0);
#else
		parse_string_outer(cmd, FLAG_PARSE_SEMICOLON
				| FLAG_EXIT_FROM_LOOP);
#endif
}

	return 0;
}

U_BOOT_CMD(
		cfgload,		1,		0,		do_load_cfgload,
		"read 'boot.ini' from FAT/EXT4 partiton",
		"\n"
		"    - read boot.ini from FAT/EXT4 partiton"
);

/* vim: set ts=4 sw=4 tw=80: */
