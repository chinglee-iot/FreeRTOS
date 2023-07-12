#include <stdint.h>

#include "pmp_apis.h"

#define IMPLEMENTED_PMP_REGIONS     16

#define CONFIG_TO_INT(_config) (*((char *)&(_config)))
#define INT_TO_CONFIG(_int) (*((struct pmp_config *)(char *)&(_int)))

int xPortPmpGetNumRegions( void )
{
    return IMPLEMENTED_PMP_REGIONS;
}

int xPortPmpGetRegion( uint32_t region, struct pmp_config *config, size_t *address )
{
    size_t pmpcfg = 0;
    char *pmpcfg_convert = (char *)&pmpcfg;

    if (!config || !address) {
        /* NULL pointers are invalid arguments */
        return 1;
    }

    if (region > IMPLEMENTED_PMP_REGIONS ) {
        /* Region outside of supported range */
        return 2;
    }

#if __riscv_xlen == 32
    switch (region / 4) {
    case 0:
        __asm__("csrr %[cfg], pmpcfg0" : [cfg] "=r"(pmpcfg)::);
        break;
    case 1:
        __asm__("csrr %[cfg], pmpcfg1" : [cfg] "=r"(pmpcfg)::);
        break;
    case 2:
        __asm__("csrr %[cfg], pmpcfg2" : [cfg] "=r"(pmpcfg)::);
        break;
    case 3:
        __asm__("csrr %[cfg], pmpcfg3" : [cfg] "=r"(pmpcfg)::);
        break;
    }

    pmpcfg = (0xFF & (pmpcfg >> (8 * (region % 4))));

#elif __riscv_xlen == 64
    switch (region / 8) {
    case 0:
        __asm__("csrr %[cfg], pmpcfg0" : [cfg] "=r"(pmpcfg)::);
        break;
    case 1:
        __asm__("csrr %[cfg], pmpcfg2" : [cfg] "=r"(pmpcfg)::);
        break;
    }

    pmpcfg = (0xFF & (pmpcfg >> (8 * (region % 8))));

#else
#error XLEN is not set to supported value for PMP driver
#endif

    *config = INT_TO_CONFIG(*pmpcfg_convert);

    switch (region) {
    case 0:
        __asm__("csrr %[addr], pmpaddr0" : [addr] "=r"(*address)::);
        break;
    case 1:
        __asm__("csrr %[addr], pmpaddr1" : [addr] "=r"(*address)::);
        break;
    case 2:
        __asm__("csrr %[addr], pmpaddr2" : [addr] "=r"(*address)::);
        break;
    case 3:
        __asm__("csrr %[addr], pmpaddr3" : [addr] "=r"(*address)::);
        break;
    case 4:
        __asm__("csrr %[addr], pmpaddr4" : [addr] "=r"(*address)::);
        break;
    case 5:
        __asm__("csrr %[addr], pmpaddr5" : [addr] "=r"(*address)::);
        break;
    case 6:
        __asm__("csrr %[addr], pmpaddr6" : [addr] "=r"(*address)::);
        break;
    case 7:
        __asm__("csrr %[addr], pmpaddr7" : [addr] "=r"(*address)::);
        break;
    case 8:
        __asm__("csrr %[addr], pmpaddr8" : [addr] "=r"(*address)::);
        break;
    case 9:
        __asm__("csrr %[addr], pmpaddr9" : [addr] "=r"(*address)::);
        break;
    case 10:
        __asm__("csrr %[addr], pmpaddr10" : [addr] "=r"(*address)::);
        break;
    case 11:
        __asm__("csrr %[addr], pmpaddr11" : [addr] "=r"(*address)::);
        break;
    case 12:
        __asm__("csrr %[addr], pmpaddr12" : [addr] "=r"(*address)::);
        break;
    case 13:
        __asm__("csrr %[addr], pmpaddr13" : [addr] "=r"(*address)::);
        break;
    case 14:
        __asm__("csrr %[addr], pmpaddr14" : [addr] "=r"(*address)::);
        break;
    case 15:
        __asm__("csrr %[addr], pmpaddr15" : [addr] "=r"(*address)::);
        break;
    }

    return 0;
}

int xPortPmpSetRegion( uint32_t region, struct pmp_config config, size_t address )
{
    struct pmp_config old_config;
    size_t old_address;
    size_t cfgmask;
    size_t pmpcfg;
    int rc = 0;

    if (region > IMPLEMENTED_PMP_REGIONS ) {
        /* Region outside of supported range */
        return 2;
    }

    #if 0
    if (config.A == METAL_PMP_NA4 &&
        pmp->_granularity[metal_cpu_get_current_hartid()] > 4) {
        /* The requested granularity is too small */
        return 3;
    }

    if (config.A == METAL_PMP_NAPOT &&
        pmp->_granularity[metal_cpu_get_current_hartid()] >
            _get_pmpaddr_granularity(address)) {
        /* The requested granularity is too small */
        return 3;
    }
    #endif

    rc = xPortPmpGetRegion( region, &old_config, &old_address);
    if (rc) {
        /* Error reading region */
        return rc;
    }

    if (old_config.L == METAL_PMP_LOCKED) {
        /* Cannot modify locked region */
        return 4;
    }

    /* Update the address first, because if the region is being locked we won't
     * be able to modify it after we set the config */
    if (old_address != address) {
        switch (region) {
        case 0:
            __asm__("csrw pmpaddr0, %[addr]" ::[addr] "r"(address) :);
            break;
        case 1:
            __asm__("csrw pmpaddr1, %[addr]" ::[addr] "r"(address) :);
            break;
        case 2:
            __asm__("csrw pmpaddr2, %[addr]" ::[addr] "r"(address) :);
            break;
        case 3:
            __asm__("csrw pmpaddr3, %[addr]" ::[addr] "r"(address) :);
            break;
        case 4:
            __asm__("csrw pmpaddr4, %[addr]" ::[addr] "r"(address) :);
            break;
        case 5:
            __asm__("csrw pmpaddr5, %[addr]" ::[addr] "r"(address) :);
            break;
        case 6:
            __asm__("csrw pmpaddr6, %[addr]" ::[addr] "r"(address) :);
            break;
        case 7:
            __asm__("csrw pmpaddr7, %[addr]" ::[addr] "r"(address) :);
            break;
        case 8:
            __asm__("csrw pmpaddr8, %[addr]" ::[addr] "r"(address) :);
            break;
        case 9:
            __asm__("csrw pmpaddr9, %[addr]" ::[addr] "r"(address) :);
            break;
        case 10:
            __asm__("csrw pmpaddr10, %[addr]" ::[addr] "r"(address) :);
            break;
        case 11:
            __asm__("csrw pmpaddr11, %[addr]" ::[addr] "r"(address) :);
            break;
        case 12:
            __asm__("csrw pmpaddr12, %[addr]" ::[addr] "r"(address) :);
            break;
        case 13:
            __asm__("csrw pmpaddr13, %[addr]" ::[addr] "r"(address) :);
            break;
        case 14:
            __asm__("csrw pmpaddr14, %[addr]" ::[addr] "r"(address) :);
            break;
        case 15:
            __asm__("csrw pmpaddr15, %[addr]" ::[addr] "r"(address) :);
            break;
        }
    }

#if __riscv_xlen == 32
    if (CONFIG_TO_INT(old_config) != CONFIG_TO_INT(config)) {
        /* Mask to clear old pmpcfg */
        cfgmask = (0xFF << (8 * (region % 4)));
        pmpcfg = (CONFIG_TO_INT(config) << (8 * (region % 4)));

        switch (region / 4) {
        case 0:
            __asm__("csrc pmpcfg0, %[mask]" ::[mask] "r"(cfgmask) :);

            __asm__("csrs pmpcfg0, %[cfg]" ::[cfg] "r"(pmpcfg) :);
            break;
        case 1:
            __asm__("csrc pmpcfg1, %[mask]" ::[mask] "r"(cfgmask) :);

            __asm__("csrs pmpcfg1, %[cfg]" ::[cfg] "r"(pmpcfg) :);
            break;
        case 2:
            __asm__("csrc pmpcfg2, %[mask]" ::[mask] "r"(cfgmask) :);

            __asm__("csrs pmpcfg2, %[cfg]" ::[cfg] "r"(pmpcfg) :);
            break;
        case 3:
            __asm__("csrc pmpcfg3, %[mask]" ::[mask] "r"(cfgmask) :);

            __asm__("csrs pmpcfg3, %[cfg]" ::[cfg] "r"(pmpcfg) :);
            break;
        }
    }
#elif __riscv_xlen == 64
    if (CONFIG_TO_INT(old_config) != CONFIG_TO_INT(config)) {
        /* Mask to clear old pmpcfg */
        cfgmask = (0xFF << (8 * (region % 8)));
        pmpcfg = (CONFIG_TO_INT(config) << (8 * (region % 8)));

        switch (region / 8) {
        case 0:
            __asm__("csrc pmpcfg0, %[mask]" ::[mask] "r"(cfgmask) :);

            __asm__("csrs pmpcfg0, %[cfg]" ::[cfg] "r"(pmpcfg) :);
            break;
        case 1:
            __asm__("csrc pmpcfg2, %[mask]" ::[mask] "r"(cfgmask) :);

            __asm__("csrs pmpcfg2, %[cfg]" ::[cfg] "r"(pmpcfg) :);
            break;
        }
    }
#else
#error XLEN is not set to supported value for PMP driver
#endif
    return 0;
}

size_t xConvertNAPOTSize( size_t xAddress, size_t xSize )
{
	/* PMP addresses are 4-byte aligned, drop the bottom two bits */
	size_t protected_addr = ((size_t) xAddress) >> 2;
	
	/* Clear the bit corresponding with alignment */
	protected_addr &= ~(xSize >> 3);

	/* Set the bits up to the alignment bit */
	protected_addr |= ((xSize >> 3) - 1);

    return protected_addr;
}
