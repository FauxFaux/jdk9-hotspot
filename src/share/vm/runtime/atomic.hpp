/*
 * Copyright (c) 1999, 2014, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_RUNTIME_ATOMIC_HPP
#define SHARE_VM_RUNTIME_ATOMIC_HPP

#include "memory/allocation.hpp"

enum cmpxchg_memory_order {
  memory_order_relaxed,
  // Use value which doesn't interfere with C++2011. We need to be more conservative.
  memory_order_conservative = 8
};

class Atomic : AllStatic {
 public:
  // Atomic operations on jlong types are not available on all 32-bit
  // platforms. If atomic ops on jlongs are defined here they must only
  // be used from code that verifies they are available at runtime and
  // can provide an alternative action if not - see supports_cx8() for
  // a means to test availability.

  // The memory operations that are mentioned with each of the atomic
  // function families come from src/share/vm/runtime/orderAccess.hpp,
  // e.g., <fence> is described in that file and is implemented by the
  // OrderAccess::fence() function. See that file for the gory details
  // on the Memory Access Ordering Model.

  // All of the atomic operations that imply a read-modify-write action
  // guarantee a two-way memory barrier across that operation. Historically
  // these semantics reflect the strength of atomic operations that are
  // provided on SPARC/X86. We assume that strength is necessary unless
  // we can prove that a weaker form is sufficiently safe.

  // Atomically store to a location
  inline static void store    (jbyte    store_value, jbyte*    dest);
  inline static void store    (jshort   store_value, jshort*   dest);
  inline static void store    (jint     store_value, jint*     dest);
  // See comment above about using jlong atomics on 32-bit platforms
  inline static void store    (jlong    store_value, jlong*    dest);
  inline static void store_ptr(intptr_t store_value, intptr_t* dest);
  inline static void store_ptr(void*    store_value, void*     dest);

  inline static void store    (jbyte    store_value, volatile jbyte*    dest);
  inline static void store    (jshort   store_value, volatile jshort*   dest);
  inline static void store    (jint     store_value, volatile jint*     dest);
  // See comment above about using jlong atomics on 32-bit platforms
  inline static void store    (jlong    store_value, volatile jlong*    dest);
  inline static void store_ptr(intptr_t store_value, volatile intptr_t* dest);
  inline static void store_ptr(void*    store_value, volatile void*     dest);

  // See comment above about using jlong atomics on 32-bit platforms
  inline static jlong load(volatile jlong* src);

  // Atomically add to a location. Returns updated value. add*() provide:
  // <fence> add-value-to-dest <membar StoreLoad|StoreStore>
  inline static jint     add    (jint     add_value, volatile jint*     dest);
  inline static size_t   add    (size_t   add_value, volatile size_t*   dest);
  inline static intptr_t add_ptr(intptr_t add_value, volatile intptr_t* dest);
  inline static void*    add_ptr(intptr_t add_value, volatile void*     dest);
  // See comment above about using jlong atomics on 32-bit platforms
  inline static jlong    add    (jlong    add_value, volatile jlong*    dest);

  // Atomically increment location. inc*() provide:
  // <fence> increment-dest <membar StoreLoad|StoreStore>
  inline static void inc    (volatile jint*     dest);
  inline static void inc    (volatile jshort*   dest);
  inline static void inc    (volatile size_t*   dest);
  inline static void inc_ptr(volatile intptr_t* dest);
  inline static void inc_ptr(volatile void*     dest);

  // Atomically decrement a location. dec*() provide:
  // <fence> decrement-dest <membar StoreLoad|StoreStore>
  inline static void dec    (volatile jint*     dest);
  inline static void dec    (volatile jshort*   dest);
  inline static void dec    (volatile size_t*   dest);
  inline static void dec_ptr(volatile intptr_t* dest);
  inline static void dec_ptr(volatile void*     dest);

  // Performs atomic exchange of *dest with exchange_value. Returns old
  // prior value of *dest. xchg*() provide:
  // <fence> exchange-value-with-dest <membar StoreLoad|StoreStore>
  inline static jint         xchg    (jint         exchange_value, volatile jint*         dest);
  inline static unsigned int xchg    (unsigned int exchange_value, volatile unsigned int* dest);
  inline static intptr_t     xchg_ptr(intptr_t     exchange_value, volatile intptr_t*     dest);
  inline static void*        xchg_ptr(void*        exchange_value, volatile void*         dest);

  // Performs atomic compare of *dest and compare_value, and exchanges
  // *dest with exchange_value if the comparison succeeded. Returns prior
  // value of *dest. cmpxchg*() provide:
  // <fence> compare-and-exchange <membar StoreLoad|StoreStore>
  inline static jbyte        cmpxchg    (jbyte        exchange_value, volatile jbyte*        dest, jbyte        compare_value, cmpxchg_memory_order order = memory_order_conservative);
  inline static jint         cmpxchg    (jint         exchange_value, volatile jint*         dest, jint         compare_value, cmpxchg_memory_order order = memory_order_conservative);
  // See comment above about using jlong atomics on 32-bit platforms
  inline static jlong        cmpxchg    (jlong        exchange_value, volatile jlong*        dest, jlong        compare_value, cmpxchg_memory_order order = memory_order_conservative);
  inline static unsigned int cmpxchg    (unsigned int exchange_value, volatile unsigned int* dest, unsigned int compare_value, cmpxchg_memory_order order = memory_order_conservative);
  inline static intptr_t     cmpxchg_ptr(intptr_t     exchange_value, volatile intptr_t*     dest, intptr_t     compare_value, cmpxchg_memory_order order = memory_order_conservative);
  inline static void*        cmpxchg_ptr(void*        exchange_value, volatile void*         dest, void*        compare_value, cmpxchg_memory_order order = memory_order_conservative);
};

// To use Atomic::inc(jshort* dest) and Atomic::dec(jshort* dest), the address must be specially
// aligned, such that (*dest) occupies the upper 16 bits of an aligned 32-bit word. The best way to
// achieve is to place your short value next to another short value, which doesn't need atomic ops.
//
// Example
//  ATOMIC_SHORT_PAIR(
//    volatile short _refcount,  // needs atomic operation
//    unsigned short _length     // number of UTF8 characters in the symbol (does not need atomic op)
//  );

#ifdef VM_LITTLE_ENDIAN
  #define ATOMIC_SHORT_PAIR(atomic_decl, non_atomic_decl)  \
    non_atomic_decl;                                       \
    atomic_decl
#else
  #define ATOMIC_SHORT_PAIR(atomic_decl, non_atomic_decl)  \
    atomic_decl;                                           \
    non_atomic_decl
#endif

#endif // SHARE_VM_RUNTIME_ATOMIC_HPP
