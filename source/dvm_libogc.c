// SPDX-License-Identifier: ZPL-2.1
// SPDX-FileCopyrightText: Copyright fincs, devkitPro
#include <sdcard/wiisd_io.h>
#include <ogc/usbstorage.h>
#include <sdcard/gcsd.h>
#include <dvm.h>

__attribute__((weak)) unsigned g_dvmDefaultCachePages = 4;
__attribute__((weak)) unsigned g_dvmDefaultSectorsPerPage = 64;
__attribute__((weak)) unsigned g_dvmOgcUsbMount = 1;

void _dvmSetAppWorkingDir(const char* argv0);

#if defined(__wii__)
unsigned _dvmDetectSectorShift(const DISC_INTERFACE* iface)
{
	if (iface == &__io_usbstorage) {
		return __builtin_ffs(__io_usbstorage_sector_size)-1;
	} else {
		return 9; // 512 byte sectors
	}
}
#endif

bool dvmInit(bool set_app_cwdir, unsigned cache_pages, unsigned sectors_per_page)
{
	unsigned num_mounted = 0;

#if defined(__wii__)
	const DISC_INTERFACE* sd    = &__io_wiisd;
	const DISC_INTERFACE* usb   = &__io_usbstorage;
#elif defined(__gamecube__)
	const DISC_INTERFACE* sd    = &__io_gcsd2;
#else
#error "Neither Wii nor GameCube"
#endif
	const DISC_INTERFACE* carda = &__io_gcsda;
	const DISC_INTERFACE* cardb = &__io_gcsdb;

	// Try mounting SD card
	num_mounted += dvmProbeMountDiscIface("sd", sd, cache_pages, sectors_per_page);

#if defined(__wii__)
	// Try mounting the first found USB drive if requested
	if (g_dvmOgcUsbMount) {
		num_mounted += dvmProbeMountDiscIface("usb", usb, cache_pages, sectors_per_page);
	}
#endif

	// Try mounting Card A
	num_mounted += dvmProbeMountDiscIface("carda", carda, cache_pages, sectors_per_page);

	// Try mounting Card B
	num_mounted += dvmProbeMountDiscIface("cardb", cardb, cache_pages, sectors_per_page);

	// Set current working directory if needed
	if (set_app_cwdir && num_mounted != 0 && __system_argv->argvMagic == ARGV_MAGIC && __system_argv->argc >= 1) {
		_dvmSetAppWorkingDir(__system_argv->argv[0]);
	}

	return num_mounted != 0;
}
