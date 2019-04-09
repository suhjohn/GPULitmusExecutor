package com.example.openclexample;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.widget.TextView;

import java.io.InputStream;

public class TestFinishedActivity extends AppCompatActivity {
    TextView tv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        setContentView(R.layout.activity_test_finished);
        tv = (TextView) findViewById(R.id.activity_test_finished_result);
        String kernelFile = "tests/MP/kernel.cl";
        String configFile = "tests/MP/config.txt";

        try {
            int err;
            // Combine testing_common and kernel.cl
            InputStream testingCommonStream = getAssets().open("tests/testing_common.h");
            int testingCommonSize = testingCommonStream.available();
            byte[] testingCommonBuffer = new byte[testingCommonSize];
            err = testingCommonStream.read(testingCommonBuffer);
            testingCommonStream.close();
            String testingCommonString = new String(testingCommonBuffer);
            testingCommonString += "\n";

            InputStream kernelStream = getAssets().open(kernelFile);
            int kernelSize = kernelStream.available();
            byte[] kernelBuffer = new byte[kernelSize];
            err = kernelStream.read(kernelBuffer);
            kernelStream.close();
            String kernelString = new String(kernelBuffer);

            kernelString = kernelString.substring(kernelString.indexOf('\n') + 1);
            kernelString = testingCommonString + kernelString;

            InputStream configStream = getAssets().open(configFile);
            int configSize = configStream.available();
            byte[] configBuffer = new byte[configSize];
            err = configStream.read(configBuffer);
            configStream.close();
            String configString = new String(configBuffer);

            tv.setText(stringFromJNI(configString, kernelString));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /* A native method that is implemented by the
     * 'litmus-test' native library, which is packaged
     * with this application.
     */
    public native String stringFromJNI(String configString, String kernelString);

    /* This is another native method declaration that is *not*
     * implemented by 'litmus-test'. This is simply to show that
     * you can declare as many native methods in your Java code
     * as you want, their implementation is searched in the
     * currently loaded native libraries only the first time
     * you call them.
     *
     * Trying to call this function will result in a
     * java.lang.UnsatisfiedLinkError exception !
     */

    /* this is used to load the 'litmus-test' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.openclexample/lib/libopencl-example.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("litmus-test");
    }
}
