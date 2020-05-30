/*
 * QEMU Hypervisor.framework (HVF) support
 *
 * Copyright Google Inc., 2017
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

/* header to be included in non-HVF-specific code */

#ifndef HVF_H
#define HVF_H

extern bool hvf_allowed;
#ifdef CONFIG_HVF
#include "exec/cpu-defs.h"

uint32_t hvf_get_supported_cpuid(uint32_t func, uint32_t idx,
                                 int reg);
#define hvf_enabled() (hvf_allowed)

typedef struct hvf_lazy_flags {
    target_ulong result;
    target_ulong auxbits;
} hvf_lazy_flags;
#else
#define hvf_enabled() 0
#define hvf_get_supported_cpuid(func, idx, reg) 0
typedef unsigned hv_vcpuid_t;
#endif

int hvf_init_vcpu(CPUState *);
int hvf_vcpu_exec(CPUState *);
int hvf_vcpu_kick(CPUState *);
void hvf_cpu_synchronize_state(CPUState *);
void hvf_cpu_synchronize_post_reset(CPUState *);
void hvf_cpu_synchronize_post_init(CPUState *);
void hvf_cpu_synchronize_pre_loadvm(CPUState *);
void hvf_vcpu_destroy(CPUState *);

int hvf_update_guest_debug(CPUState *);

#define TYPE_HVF_ACCEL ACCEL_CLASS_NAME("hvf")

#define HVF_STATE(obj) \
    OBJECT_CHECK(HVFState, (obj), TYPE_HVF_ACCEL)

#endif
