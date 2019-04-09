package com.example.openclexample;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import java.io.InputStream;

public class TestFinishedActivity extends AppCompatActivity {
    TextView tv;
    int iteration;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        Bundle bundle = getIntent().getExtras();
        iteration = bundle.getInt("iteration");
        setContentView(R.layout.activity_test_finished);
        executeTest();
    }

    protected void executeTest() {
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
            String result = executeLitmusTest(configString, kernelString, iteration);
            tv.setText(result);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public native String executeLitmusTest(String configString,
                                           String kernelString,
                                           int iteration);

    static {
        System.loadLibrary("litmus-test");
    }
}
