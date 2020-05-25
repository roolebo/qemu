/*
 * QTest accelerator code
 *
 * Copyright IBM, Corp. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#include "qemu/osdep.h"
#include "qemu/rcu.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "qemu/option.h"
#include "qemu/config-file.h"
#include "sysemu/accel.h"
#include "sysemu/qtest.h"
#include "sysemu/cpus.h"
#include "sysemu/cpu-timers.h"
#include "qemu/guest-random.h"
#include "qemu/main-loop.h"
#include "hw/core/cpu.h"

static void qtest_cpu_synchronize_noop(CPUState *cpu)
{
}

static void qtest_kick_vcpu_thread(CPUState *cpu)
{
    cpus_kick_thread(cpu);
}

static void *qtest_cpu_thread_fn(void *arg)
{
#ifdef _WIN32
    error_report("qtest is not supported under Windows");
    exit(1);
#else
    CPUState *cpu = arg;
    sigset_t waitset;
    int r;

    rcu_register_thread();

    qemu_mutex_lock_iothread();
    qemu_thread_get_self(cpu->thread);
    cpu->thread_id = qemu_get_thread_id();
    cpu->can_do_io = 1;
    current_cpu = cpu;

    sigemptyset(&waitset);
    sigaddset(&waitset, SIG_IPI);

    /* signal CPU creation */
    cpu_thread_signal_created(cpu);
    qemu_guest_random_seed_thread_part2(cpu->random_seed);

    do {
        qemu_mutex_unlock_iothread();
        do {
            int sig;
            r = sigwait(&waitset, &sig);
        } while (r == -1 && (errno == EAGAIN || errno == EINTR));
        if (r == -1) {
            perror("sigwait");
            exit(1);
        }
        qemu_mutex_lock_iothread();
        qemu_wait_io_event(cpu);
    } while (!cpu->unplug);

    qemu_mutex_unlock_iothread();
    rcu_unregister_thread();
    return NULL;
#endif
}

static void qtest_start_vcpu_thread(CPUState *cpu)
{
    char thread_name[VCPU_THREAD_NAME_SIZE];

    cpu->thread = g_malloc0(sizeof(QemuThread));
    cpu->halt_cond = g_malloc0(sizeof(QemuCond));
    qemu_cond_init(cpu->halt_cond);
    snprintf(thread_name, VCPU_THREAD_NAME_SIZE, "CPU %d/DUMMY",
             cpu->cpu_index);
    qemu_thread_create(cpu->thread, thread_name, qtest_cpu_thread_fn, cpu,
                       QEMU_THREAD_JOINABLE);
}

CpusAccelInterface qtest_cpus_interface = {
    .create_vcpu_thread = qtest_start_vcpu_thread,
    .kick_vcpu_thread = qtest_kick_vcpu_thread,

    .cpu_synchronize_post_reset = qtest_cpu_synchronize_noop,
    .cpu_synchronize_post_init = qtest_cpu_synchronize_noop,
    .cpu_synchronize_state = qtest_cpu_synchronize_noop,
    .cpu_synchronize_pre_loadvm = qtest_cpu_synchronize_noop,
};

static int qtest_init_accel(MachineState *ms)
{
    cpus_register_accel_interface(&qtest_cpus_interface);
    return 0;
}

static void qtest_accel_class_init(ObjectClass *oc, void *data)
{
    AccelClass *ac = ACCEL_CLASS(oc);
    ac->name = "QTest";
    ac->init_machine = qtest_init_accel;
    ac->allowed = &qtest_allowed;
}

#define TYPE_QTEST_ACCEL ACCEL_CLASS_NAME("qtest")

static const TypeInfo qtest_accel_type = {
    .name = TYPE_QTEST_ACCEL,
    .parent = TYPE_ACCEL,
    .class_init = qtest_accel_class_init,
};

static void qtest_type_init(void)
{
    type_register_static(&qtest_accel_type);
}

type_init(qtest_type_init);
