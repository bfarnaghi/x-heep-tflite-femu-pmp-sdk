// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
#include <stdint.h>

#ifndef OPENTITAN_SW_DEVICE_LIB_HANDLER_H_
#define OPENTITAN_SW_DEVICE_LIB_HANDLER_H_

typedef enum exc_id {
  kInstMisa = 0,
  kInstAccFault = 1,
  kInstIllegalFault = 2,
  kBkpt = 3,
  kLoadAccFault = 5,
  kStrAccFault = 7,
  uECall = 8,
  kECall = 11,
  kIdMax = 31
} exc_id_t;

// The RISC-V interrupt vector will not include the addresses of the handlers,
// instead, it includes (uncompressed) instructions. Thus the interrupt vector
// will include `j <interrupt handler name>` for each handler.
//
// The only requirement on the symbol in the jump is that it must be correctly
// aligned. If the processor supports the C extension, this can be 2-byte
// aligned, but 4-byte aligned is compatible with all RISC-V processors.
//
// If the processor is not using interrupt vectoring, then there will be a
// single address where interrupts jump to, which will either contain a function
// (which will need to be aligned), or will contain a jump to a function, again
// which will need to be aligned.
//
// You only need to use this ABI for handlers that are the first function called
// in an interrupt handler. Subsequent functions can just use the regular RISC-V
// calling convention.
#define INTERRUPT_HANDLER_ABI __attribute__((aligned(4), interrupt))

// The following `handler_*` functions have weak definitions, provided by
// `handler.c`. This weak definition can be overriden at link-time by providing
// an additional non-weak definition of each function. Executables and libraries
// must not contain more than one weak definition of the same symbol.

/**
 * Default exception handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_exception(void);

/**
 * Software IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_software(void);

/**
 * Timer IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_timer(void);

/**
 * External IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_external(void);

/**
 * Fast timer 1 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_timer_1(void);

/**
 * Fast timer 2 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_timer_2(void);

/**
 * Fast timer 3 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_timer_3(void);

/**
 * Fast dma IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_dma(void);

/**
 * Fast spi IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_spi(void);

/**
 * Fast spi flash IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_spi_flash(void);

/**
 * Fast gpio 0 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_0(void);

/**
 * Fast gpio 1 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_1(void);

/**
 * Fast gpio 2 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_2(void);

/**
 * Fast gpio 3 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_3(void);

/**
 * Fast gpio 4 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_4(void);

/**
 * Fast gpio 5 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_5(void);

/**
 * Fast gpio 6 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_6(void);

/**
 * Fast gpio 7 IRQ handler.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_7(void);

/**
 * Instruction access fault.
 *
 * Called by default implementation of `handler_exception`. If that function is
 * overriden, this function may not be called.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
void handler_instr_acc_fault(void);

/**
 * Illegal Instruction fault.
 *
 * Called by default implementation of `handler_exception`. If that function is
 * overriden, this function may not be called.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
void handler_instr_ill_fault(void);

/**
 * Breakpoint handler.
 *
 * Called by default implementation of `handler_exception`. If that function is
 * overriden, this function may not be called.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
void handler_bkpt(void);

/**
 * Load store unit fault.
 *
 * Called by default implementation of `handler_exception`. If that function is
 * overriden, this function may not be called.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
void handler_lsu_fault(void);

/**
 * Exception call handler.
 *
 * Called by default implementation of `handler_exception`. If that function is
 * overriden, this function may not be called.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
void handler_ecall(void);

/**
 * User-mode exception call handler.
 *
 * Called by default implementation of `handler_exception`. If that function is
 * overriden, this function may not be called.
 *
 * `handler.c` provides a weak definition of this symbol, which can be overriden
 * at link-time by providing an additional non-weak definition.
 */
void handler_user_ecall(uint32_t syscall_id,uintptr_t  buf_ptr,uint32_t len);

#endif  // OPENTITAN_SW_DEVICE_LIB_HANDLER_H_
