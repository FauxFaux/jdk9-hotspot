/*
 * Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.
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

import java.lang.management.MemoryPoolMXBean;
import java.util.EnumSet;
import java.util.ArrayList;

import sun.hotspot.WhiteBox;
import sun.hotspot.code.BlobType;
import com.oracle.java.testlibrary.Asserts;

/*
 * @test AllocationCodeBlobTest
 * @bug 8059624
 * @library /testlibrary /testlibrary/whitebox
 * @build AllocationCodeBlobTest
 * @run main ClassFileInstaller sun.hotspot.WhiteBox
 *                              sun.hotspot.WhiteBox$WhiteBoxPermission
 * @run main/othervm -Xbootclasspath/a:. -XX:+UnlockDiagnosticVMOptions
 *                   -XX:+WhiteBoxAPI -XX:CompileCommand=compileonly,null::*
 *                   -XX:-SegmentedCodeCache AllocationCodeBlobTest
 * @run main/othervm -Xbootclasspath/a:. -XX:+UnlockDiagnosticVMOptions
 *                   -XX:+WhiteBoxAPI -XX:CompileCommand=compileonly,null::*
 *                   -XX:+SegmentedCodeCache AllocationCodeBlobTest
 * @summary testing of WB::allocate/freeCodeBlob()
 */
public class AllocationCodeBlobTest {
    private static final WhiteBox WHITE_BOX = WhiteBox.getWhiteBox();
    private static final long CODE_CACHE_SIZE
            = WHITE_BOX.getUintxVMFlag("ReservedCodeCacheSize");
    private static final int SIZE = 1;

    public static void main(String[] args) {
        // check that Sweeper handels dummy blobs correctly
        new ForcedSweeper(500).start();
        EnumSet<BlobType> blobTypes = BlobType.getAvailable();
        for (BlobType type : blobTypes) {
            new AllocationCodeBlobTest(type).test();
        }
    }

    private final BlobType type;
    private final MemoryPoolMXBean bean;
    private AllocationCodeBlobTest(BlobType type) {
        this.type = type;
        bean = type.getMemoryPool();
    }

    private void test() {
        System.out.printf("type %s%n", type);
        long start = getUsage();
        long addr = WHITE_BOX.allocateCodeBlob(SIZE, type.id);
        Asserts.assertNE(0, addr, "allocation failed");

        long firstAllocation = getUsage();
        Asserts.assertLTE(start + SIZE, firstAllocation,
                "allocation should increase memory usage: "
                + start + " + " + SIZE + " <= " + firstAllocation);

        WHITE_BOX.freeCodeBlob(addr);
        long firstFree = getUsage();
        Asserts.assertLTE(firstFree, firstAllocation,
                "free shouldn't increase memory usage: "
                + firstFree + " <= " + firstAllocation);

        addr = WHITE_BOX.allocateCodeBlob(SIZE, type.id);
        Asserts.assertNE(0, addr, "allocation failed");

        long secondAllocation = getUsage();
        Asserts.assertEQ(firstAllocation, secondAllocation);

        WHITE_BOX.freeCodeBlob(addr);
        System.out.println("allocating till possible...");
        ArrayList<Long> blobs = new ArrayList<>();
        int size = (int) (CODE_CACHE_SIZE >> 7);
        while ((addr = WHITE_BOX.allocateCodeBlob(size, type.id)) != 0) {
            blobs.add(addr);
        }
        for (Long blob : blobs) {
            WHITE_BOX.freeCodeBlob(blob);
        }
    }

    private long getUsage() {
        return bean.getUsage().getUsed();
    }

    private static class ForcedSweeper extends Thread {
        private final int millis;
        public ForcedSweeper(int millis) {
            super("ForcedSweeper");
            setDaemon(true);
            this.millis = millis;
        }
        public void run() {
            try {
                while (true) {
                    WHITE_BOX.forceNMethodSweep();
                    Thread.sleep(millis);
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new Error(e);
            }
        }
    }
}
