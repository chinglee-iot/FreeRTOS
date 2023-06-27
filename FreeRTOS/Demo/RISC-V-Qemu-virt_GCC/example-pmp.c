/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdlib.h>
#include <stdio.h>

#include <metal/cpu.h>
#include <metal/pmp.h>

#include "riscv-virt.h"

struct metal_pmp __metal_dt_pmp;
struct __metal_driver_cpu __metal_dt_cpu_0;

#define ECODE_STORE_FAULT	7

#define NAPOT_SIZE 128
#define PROTECTED_ARRAY_LENGTH 32
volatile uint32_t protected_global[PROTECTED_ARRAY_LENGTH] __attribute__((aligned(NAPOT_SIZE)));

#if 0
/* Not used in FreeRTOS. */
void store_access_fault_handler(struct metal_cpu *cpu, int ecode)
{
	/* Get faulting instruction and instruction length */
	unsigned long epc = metal_cpu_get_exception_pc(cpu);
	int inst_len = metal_cpu_get_instruction_length(cpu, epc);

    // vSendString( "Exception\r\n" );

	/* Advance the exception program counter by the length of the
	 * instruction to return execution after the faulting store */
	metal_cpu_set_exception_pc(cpu, epc + inst_len);

	if(ecode != ECODE_STORE_FAULT) {
		exit(7);
	}
}
#endif

void freertos_risc_v_application_exception_handler( void )
{
    uint32_t mCasue, mEpc, mStatus;
    char temp[128];

    vSendString( "Exception\r\n" );

    asm( "csrr %[varCause], mcause\n"
         "csrr %[varEpc], mepc\n"
         "csrr %[varStatus], mstatus\n":
         [varCause] "=r" (mCasue), [varEpc] "=r" (mEpc), [varStatus] "=r" (mStatus)
         :: );

    snprintf( temp, 128, "mcause 0x%08lx\r\n", mCasue );
    vSendString( temp );

    snprintf( temp, 128, "mepc 0x%08lx\r\n", mEpc );
    vSendString( temp );

    snprintf( temp, 128, "mstatus 0x%08lx\r\n", mStatus );
    vSendString( temp );

    while( 1 );
}

int test_main()
{
	int rc;
	struct metal_cpu *cpu;
	struct metal_interrupt *cpu_intr;
	struct metal_pmp *pmp;

	/* PMP addresses are 4-byte aligned, drop the bottom two bits */
	size_t protected_addr = ((size_t) &protected_global) >> 2;
	
	/* Clear the bit corresponding with alignment */
	protected_addr &= ~(NAPOT_SIZE >> 3);

	/* Set the bits up to the alignment bit */
	protected_addr |= ((NAPOT_SIZE >> 3) - 1);

	vSendString("PMP Driver Example\n");

	/* Initialize interrupt handling on the current CPU */
	cpu = metal_cpu_get(metal_cpu_get_current_hartid());
	if(!cpu) {
		vSendString("Unable to get CPU handle\n");
		return 1;
	}
    vSendString("metal_cpu_get done\n");

    /* 
	cpu_intr = metal_cpu_interrupt_controller(cpu);
	if(!cpu_intr) {
		vSendString("Unable to get CPU Interrupt handle\n");
		return 2;
	}
	metal_interrupt_init(cpu_intr);
    vSendString("metal_cpu_interrupt_controller done\n"); */

	/* Register a handler for the store access fault exception */
    /*
	rc = metal_cpu_exception_register(cpu, ECODE_STORE_FAULT, store_access_fault_handler);
	if(rc < 0) {
		vSendString("Failed to register exception handler\n");
		return 3;
	}
    vSendString("metal_cpu_exception_register done\n");
    */

	/* Initialize PMPs */
	pmp = metal_pmp_get_device();
	if(!pmp) {
		vSendString("Unable to get PMP Device\n");
		return 4;
	}
	metal_pmp_init(pmp);

	/* Perform a write to the memory we're about to protect access to */
	protected_global[0] = 0;

	/* Configure PMP 0 to only allow reads to protected_global. The
	 * PMP region is locked so that the configuration applies to M-mode
	 * accesses. */
	struct metal_pmp_config config = {
		.L = METAL_PMP_LOCKED,
		.A = METAL_PMP_NAPOT, /* Naturally-aligned power of two */
		.X = 0,
		.W = 0,
		.R = 1,
	};
	rc = metal_pmp_set_region(pmp, 0, config, protected_addr);
	if(rc != 0) {
		vSendString("Failed to configure PMP 0\n");
		return 5;
	}

	vSendString("Attempting to write to protected address\n");
    volatile int a = 0;

	/* Attempt to write to protected_global. This should generate a store
	 * access fault exception. */
	protected_global[0] = 6;
    
    vSendString("Attempting to write to protected address done\n");

	/* If execution returns to here, return the value of protected
	 * global to demonstrate that we can still read the value */

	/* If the write succeeds, the test fails */
	return protected_global[0];
}
