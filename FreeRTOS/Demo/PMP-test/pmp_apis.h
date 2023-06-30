#ifndef __PMP_APIS_H__
#define __PMP_APIS_H__

#include <stddef.h>

/*!
 * @brief Set of available PMP addressing modes
 */
enum metal_pmp_address_mode {
    /*! @brief Disable the PMP region */
    METAL_PMP_OFF = 0,
    /*! @brief Use Top-of-Range mode */
    METAL_PMP_TOR = 1,
    /*! @brief Use naturally-aligned 4-byte region mode */
    METAL_PMP_NA4 = 2,
    /*! @brief Use naturally-aligned power-of-two mode */
    METAL_PMP_NAPOT = 3
};

/*!
 * @brief Configuration for a PMP region
 */
struct pmp_config {
    /*! @brief Sets whether reads to the PMP region succeed */
    unsigned int R : 1;
    /*! @brief Sets whether writes to the PMP region succeed */
    unsigned int W : 1;
    /*! @brief Sets whether the PMP region is executable */
    unsigned int X : 1;

    /*! @brief Sets the addressing mode of the PMP region */
    enum metal_pmp_address_mode A : 2;

    int _pad : 2;

    /*! @brief Sets whether the PMP region is locked */
    enum metal_pmp_locked {
        METAL_PMP_UNLOCKED = 0,
        METAL_PMP_LOCKED = 1
    } L : 1;
};

size_t xConvertNAPOTSize( size_t xAddress, size_t xSize );

int xPortPmpGetNumRegions( void );

int xPortPmpGetRegion( uint32_t region, struct pmp_config *config, size_t *address );

int xPortPmpSetRegion( uint32_t region, struct pmp_config config, size_t address );

#endif /* __PMP_APIS_H__ */
