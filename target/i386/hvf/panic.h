/*
 * Copyright (C) 2016 Veertu Inc,
 * Copyright (C) 2017 Google Inc,
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HVF_PANIC_H
#define HVF_PANIC_H

#define VM_PANIC(x) {\
    fprintf(stderr, "%s\n", x); \
    cpu_dump_state(current_cpu, stderr, CPU_DUMP_CODE); \
    exit(EXIT_FAILURE); \
}

#define VM_PANIC_ON(x) {\
    if (x) { \
        fprintf(stderr, "%s\n", #x); \
        cpu_dump_state(current_cpu, stderr, CPU_DUMP_CODE); \
        exit(EXIT_FAILURE); \
    } \
}

#define VM_PANIC_EX(...) {\
    fprintf(stderr, __VA_ARGS__); \
    cpu_dump_state(current_cpu, stderr, CPU_DUMP_CODE); \
    exit(EXIT_FAILURE); \
}

#define VM_PANIC_ON_EX(x, ...) {\
    if (x) { \
        fprintf(stderr, __VA_ARGS__); \
        cpu_dump_state(current_cpu, stderr, CPU_DUMP_CODE); \
        exit(EXIT_FAILURE); \
    } \
}

#endif
