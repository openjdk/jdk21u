/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
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
 */

/**
 * @test
 * @bug 6313661
 * @summary Known-answer-test for TlsMasterSecret generator
 * @author Andreas Sterbenz
 * @modules java.base/sun.security.internal.interfaces
 *          java.base/sun.security.internal.spec
 */

import java.io.*;
import java.util.*;

import java.security.Security;
import java.security.Provider;

import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;

import javax.crypto.spec.*;

import sun.security.internal.spec.*;
import sun.security.internal.interfaces.TlsMasterSecret;

public class TestMasterSecret extends Utils {

    private static int PREFIX_LENGTH = "m-premaster:  ".length();

    public static void main(String[] args) throws Exception {
        Provider provider = Security.getProvider(
                System.getProperty("test.provider.name", "SunJCE"));

        InputStream in = new FileInputStream(new File(BASE, "masterdata.txt"));
        BufferedReader reader = new BufferedReader(new InputStreamReader(in));

        int n = 0;
        int lineNumber = 0;

        String algorithm = null;
        byte[] premaster = null;
        byte[] clientRandom = null;
        byte[] serverRandom = null;
        int protoMajor = 0;
        int protoMinor = 0;
        int preMajor = 0;
        int preMinor = 0;
        byte[] master = null;

        while (true) {
            String line = reader.readLine();
            lineNumber++;
            if (line == null) {
                break;
            }
            if (line.startsWith("m-") == false) {
                continue;
            }
            String data = line.substring(PREFIX_LENGTH);
            if (line.startsWith("m-algorithm:")) {
                algorithm = data;
            } else if (line.startsWith("m-premaster:")) {
                premaster = parse(data);
            } else if (line.startsWith("m-crandom:")) {
                clientRandom = parse(data);
            } else if (line.startsWith("m-srandom:")) {
                serverRandom = parse(data);
            } else if (line.startsWith("m-protomajor:")) {
                protoMajor = Integer.parseInt(data);
            } else if (line.startsWith("m-protominor:")) {
                protoMinor = Integer.parseInt(data);
            } else if (line.startsWith("m-premajor:")) {
                preMajor = Integer.parseInt(data);
            } else if (line.startsWith("m-preminor:")) {
                preMinor = Integer.parseInt(data);
            } else if (line.startsWith("m-master:")) {
                master = parse(data);

                System.out.print(".");
                n++;

                KeyGenerator kg =
                    KeyGenerator.getInstance("SunTlsMasterSecret", provider);
                SecretKey premasterKey =
                    new SecretKeySpec(premaster, algorithm);
                TlsMasterSecretParameterSpec spec =
                    new TlsMasterSecretParameterSpec(premasterKey, protoMajor,
                        protoMinor, clientRandom, serverRandom,
                        null, -1, -1);
                kg.init(spec);
                TlsMasterSecret key = (TlsMasterSecret)kg.generateKey();
                byte[] enc = key.getEncoded();
                if (Arrays.equals(master, enc) == false) {
                    throw new Exception("mismatch line: " + lineNumber);
                }
                if ((preMajor != key.getMajorVersion()) ||
                        (preMinor != key.getMinorVersion())) {
                    throw new Exception("version mismatch line: " + lineNumber);
                }
            } else {
                throw new Exception("Unknown line: " + line);
            }
        }
        if (n == 0) {
            throw new Exception("no tests");
        }
        in.close();
        System.out.println();
        System.out.println("OK: " + n + " tests");
    }

}
