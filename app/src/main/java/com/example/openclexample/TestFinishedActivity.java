package com.example.openclexample;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import com.loopj.android.http.RequestParams;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;

import cz.msebera.android.httpclient.HttpEntity;
import cz.msebera.android.httpclient.entity.StringEntity;
import cz.msebera.android.httpclient.message.BasicHeader;
import cz.msebera.android.httpclient.protocol.HTTP;

public class TestFinishedActivity extends AppCompatActivity implements OnLoopjCompleted {
    TextView tv;
    int iteration;

    public void taskCompleted(JSONObject response) {
        String response_str = response.toString();
        tv.setText(response_str);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        JSONObject jsonObj;
        Bundle bundle = getIntent().getExtras();
        iteration = bundle.getInt("iteration");
        setContentView(R.layout.activity_test_finished);
        tv = (TextView) findViewById(R.id.activity_test_finished_result);
        String kernelFile = "tests/MP/kernel.cl";
        String configFile = "tests/MP/config.txt";
        String result = executeTest(kernelFile, configFile);
        Log.i("onCreate", result);
        try {
            jsonObj = new JSONObject(result);
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }
        tv.setText(result);
//        StringEntity entity = new StringEntity(jsonObj.toString());
//        entity.setContentType(new BasicHeader(HTTP.CONTENT_TYPE, "application/json"));
//        ServerRestClient.post("test", entity, this);
    }

    protected String executeTest(String kernelFile, String configFile) {
        try {
            int err;
            // Combine testing_common and kernel.cl
            InputStream testingCommonStream = getAssets().open("tests/testing_common.h");
            int testingCommonSize = testingCommonStream.available();
            byte[] testingCommonBuffer = new byte[testingCommonSize];
            err = testingCommonStream.read(testingCommonBuffer);
            if (err == -1) {
                throw new IOException("Error while reading testing_common.");
            }
            testingCommonStream.close();
            String testingCommonString = new String(testingCommonBuffer);
            testingCommonString += "\n";

            InputStream kernelStream = getAssets().open(kernelFile);
            int kernelSize = kernelStream.available();
            byte[] kernelBuffer = new byte[kernelSize];
            err = kernelStream.read(kernelBuffer);
            if (err == -1) {
                throw new IOException("Error while reading kernel.");
            }
            kernelStream.close();
            String kernelString = new String(kernelBuffer);

            kernelString = kernelString.substring(kernelString.indexOf('\n') + 1);
            kernelString = testingCommonString + kernelString;

            InputStream configStream = getAssets().open(configFile);
            int configSize = configStream.available();
            byte[] configBuffer = new byte[configSize];
            err = configStream.read(configBuffer);
            if (err == -1) {
                throw new IOException("Error while reading config.");
            }
            configStream.close();
            String configString = new String(configBuffer);
            return executeLitmusTest(configString, kernelString, iteration);
        } catch (Exception e) {
            Log.e("executeTest", e.getMessage());
            e.printStackTrace();
        }
        return "";
    }

    public native String executeLitmusTest(String configString,
                                           String kernelString,
                                           int iteration);

    static {
        System.loadLibrary("litmus-test");
    }
}
