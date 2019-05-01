package com.suhjohn.androidgpuconformancetester.utils;

import static com.suhjohn.androidgpuconformancetester.utils.TestFileHandler.*;

public class LitmusTest {
    private String configStr;
    private String kernelStr;
    private String options;
    private Integer iterations;
    private Integer xyStride;

    static {
        System.loadLibrary("litmus-test");
    }

    public native String executeLitmusTest(String configString,
                                           String kernelString,
                                           int iterations,
                                           String options,
                                           int x_y_stride);


    public LitmusTest(String kernelStr, String configStr,
                      String options, int iterations, int xyStride) {
        this.kernelStr = kernelStr;
        this.configStr = configStr;
        this.options = options;
        this.iterations = iterations;
        this.xyStride = xyStride;
    }

    public String execute() {
        return executeLitmusTest(
                this.configStr, this.kernelStr, this.iterations,
                this.options, this.xyStride);
    }
}
