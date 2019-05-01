package com.suhjohn.androidgpuconformancetester.utils;

import android.content.res.AssetManager;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;

public class TestFileHandler {
    private final String configFpFormat = "OpenCL_tests/interwg_base/%s/config.txt";
    private final String kernelFpFormat = "OpenCL_tests/interwg_base/%s/kernel.cl";
    private final String commonHeaderFp = "OpenCL_tests/interwg_base/testing_common.h";
    private AssetManager assetManager;
    private String litmusTestType;

    public TestFileHandler(AssetManager assetManager, String litmusTestType) {
        this.assetManager = assetManager;
        this.litmusTestType = litmusTestType;
    }

    public String readKernel() throws Exception {
        String kernelFile = String.format(kernelFpFormat, this.litmusTestType);
        try {
            int err;
            // Combine testing_common and kernel.cl
            InputStream testingCommonStream = this.assetManager.open(commonHeaderFp);
            int testingCommonSize = testingCommonStream.available();
            byte[] testingCommonBuffer = new byte[testingCommonSize];
            err = testingCommonStream.read(testingCommonBuffer);
            if (err == -1) {
                throw new IOException("Error while reading testing_common.");
            }
            testingCommonStream.close();
            String testingCommonStr = new String(testingCommonBuffer);
            testingCommonStr += "\n";

            InputStream kernelStream = this.assetManager.open(kernelFile);
            int kernelSize = kernelStream.available();
            byte[] kernelBuffer = new byte[kernelSize];
            err = kernelStream.read(kernelBuffer);
            if (err == -1) {
                throw new IOException("Error while reading kernel.");
            }
            kernelStream.close();
            String kernelStr = new String(kernelBuffer);
            return testingCommonStr + kernelStr;
        } catch (Exception e) {
            Log.e("executeQuickTest", e.getMessage());
            e.printStackTrace();
            throw e;
        }
    }

    public String readConfig() throws Exception {
        String configFile = String.format(configFpFormat, this.litmusTestType);
        try {
            int err;
            InputStream configStream = this.assetManager.open(configFile);
            int configSize = configStream.available();
            byte[] configBuffer = new byte[configSize];
            err = configStream.read(configBuffer);
            if (err == -1) {
                throw new IOException("Error while reading config.");
            }
            configStream.close();
            return new String(configBuffer);
        } catch (Exception e) {
            Log.e("executeQuickTest", e.getMessage());
            e.printStackTrace();
            throw e;
        }
    }
}
