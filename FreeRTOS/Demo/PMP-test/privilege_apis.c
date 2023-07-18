#include "privilege_apis.h"

enum metal_privilege_mode {
    METAL_PRIVILEGE_USER = 0,
    METAL_PRIVILEGE_SUPERVISOR = 1,
    METAL_PRIVILEGE_MACHINE = 3,
};

#define METAL_MSTATUS_MIE_OFFSET 3
#define METAL_MSTATUS_MPIE_OFFSET 7
#define METAL_MSTATUS_SIE_OFFSET 1
#define METAL_MSTATUS_SPIE_OFFSET 5
#define METAL_MSTATUS_UIE_OFFSET 0
#define METAL_MSTATUS_UPIE_OFFSET 4

#define METAL_MSTATUS_MPP_OFFSET 11
#define METAL_MSTATUS_MPP_MASK 3

static void prvPrivilegeDropToMode( enum metal_privilege_mode mode )
{
    uintptr_t mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));

    /* Set xPIE bits based on current xIE bits */
    if (mstatus & (1 << METAL_MSTATUS_MIE_OFFSET)) {
        mstatus |= (1 << METAL_MSTATUS_MPIE_OFFSET);
    } else {
        mstatus &= ~(1 << METAL_MSTATUS_MPIE_OFFSET);
    }
    if (mstatus & (1 << METAL_MSTATUS_SIE_OFFSET)) {
        mstatus |= (1 << METAL_MSTATUS_SPIE_OFFSET);
    } else {
        mstatus &= ~(1 << METAL_MSTATUS_SPIE_OFFSET);
    }
    if (mstatus & (1 << METAL_MSTATUS_UIE_OFFSET)) {
        mstatus |= (1 << METAL_MSTATUS_UPIE_OFFSET);
    } else {
        mstatus &= ~(1 << METAL_MSTATUS_UPIE_OFFSET);
    }

    /* Set MPP to the requested privilege mode */
    mstatus &= ~(METAL_MSTATUS_MPP_MASK << METAL_MSTATUS_MPP_OFFSET);
    mstatus |= (mode << METAL_MSTATUS_MPP_OFFSET);

    __asm__ volatile("csrw mstatus, %0" ::"r"(mstatus));

    /* Set the entry point in MEPC */
    __asm__ volatile("csrw mepc, ra");

    __asm__ volatile("mret");
}

BaseType_t portIS_PRIVILEGED( void )
{
    int ret;

    ret = __internal_syscall_0( portECALL_IS_PRIVILEGED );

    /* Get the machine previous privilege. */
    return ret;
}

void portRAISE_PRIVILEGE( void )
{
    /* Rasie privilege through ecall. */
    __internal_syscall_0( portECALL_RAISE_PRIORITY );
}

void portRESET_PRIVILEGE( void )
{
    /* User mode entry point is the return address. */
    prvPrivilegeDropToMode( METAL_PRIVILEGE_USER );
}

void portSWITCH_TO_USER_MODE( void )
{
    /* User mode still using the same stack. */

    /* User mode entry point is the return address. */
    prvPrivilegeDropToMode( METAL_PRIVILEGE_USER );
}
