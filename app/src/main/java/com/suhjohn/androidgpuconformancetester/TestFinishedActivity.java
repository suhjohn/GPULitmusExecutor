package com.suhjohn.androidgpuconformancetester;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.loopj.android.http.JsonHttpResponseHandler;

import org.json.*;

import java.io.IOException;
import java.io.InputStream;

import cz.msebera.android.httpclient.Header;
import cz.msebera.android.httpclient.entity.StringEntity;
import cz.msebera.android.httpclient.message.BasicHeader;
import cz.msebera.android.httpclient.protocol.HTTP;

// TODO refactor code using utils
public class TestFinishedActivity extends AppCompatActivity {
    private final String commonHeaderFilepath = "OpenCL_tests/interwg_base/testing_common.h";
    private final int defaultIterations = 100;
    private TextView tv;
    private ProgressBar progressBar;
    private String litmusTestType;
    private Integer iterations;
    private String options;
    private Integer xyStride;
    private String kernelFile;
    private String configFile;

    private class LitmustTestTask extends AsyncTask<String, Integer, String> {
        protected String doInBackground(String... arguments) {
            return executeTest(arguments[0], arguments[1]);
        }

        protected void onPostExecute(String response) {
            Log.i("onPostExecute", response);
            try {
                JSONObject requestData = new JSONObject();
                requestData.put("result", response);
                requestData.put("test_type", litmusTestType);
                requestData.put("x_y_stride", xyStride);
                StringEntity entity = new StringEntity(requestData.toString());
                entity.setContentType(new BasicHeader(HTTP.CONTENT_TYPE, "application/json"));
                ServerRestClient.post("test", entity, new JsonHttpResponseHandler() {
                    public void onSuccess(int statusCode, Header[] headers, JSONObject response) {
                        Log.i("post.onSuccess", response.toString());
                        Log.i("post.onSuccess", Integer.toString(statusCode));
                    }
                });
            } catch (Exception e) {
                Log.e("onPostExecute", e.getMessage());
            }
            String finalResponse = String.format(
                    "Test: %s\nx_y_stride: %d\n", litmusTestType, xyStride) + response;
            tv.setText(finalResponse);
        }
        
    }

    private void setBundleVariables() {
        Bundle bundle = getIntent().getExtras();
        try {
            iterations = bundle.getInt("iterations");
            xyStride = bundle.getInt("xyStride");
            litmusTestType = bundle.getString("litmusTestType");
            options = bundle.getString("options");
        } catch (Exception e) {
            iterations = defaultIterations;
            xyStride = 32;
            litmusTestType = "MP";
            options = "";
        }
    }

    private void setTestFilePath() {
        kernelFile = String.format("OpenCL_tests/interwg_base/%s/kernel.cl", litmusTestType);
        configFile = String.format("OpenCL_tests/interwg_base/%s/config.txt", litmusTestType);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setBundleVariables();
        this.setTestFilePath();
        setContentView(R.layout.activity_test_finished);
        tv = findViewById(R.id.activity_test_finished_result);
        progressBar = findViewById(R.id.activity_test_finished_progressbar);
        new LitmustTestTask().execute(kernelFile, configFile);
    }

    private String executeTest(String kernelFile, String configFile) {
        try {
            int err;
            // Combine testing_common and kernel.cl
            InputStream testingCommonStream = getAssets().open(commonHeaderFilepath);
            int testingCommonSize = testingCommonStream.available();
            byte[] testingCommonBuffer = new byte[testingCommonSize];
            err = testingCommonStream.read(testingCommonBuffer);
            if (err == -1) {
                throw new IOException("Error while reading testing_common.");
            }
            testingCommonStream.close();
            String testingCommonStr = new String(testingCommonBuffer);
            testingCommonStr += "\n";

            InputStream kernelStream = getAssets().open(kernelFile);
            int kernelSize = kernelStream.available();
            byte[] kernelBuffer = new byte[kernelSize];
            err = kernelStream.read(kernelBuffer);
            if (err == -1) {
                throw new IOException("Error while reading kernel.");
            }
            kernelStream.close();
            String kernelStr = new String(kernelBuffer);

            kernelStr = testingCommonStr + kernelStr;

            InputStream configStream = getAssets().open(configFile);
            int configSize = configStream.available();
            byte[] configBuffer = new byte[configSize];
            err = configStream.read(configBuffer);
            if (err == -1) {
                throw new IOException("Error while reading config.");
            }
            configStream.close();
            String configString = new String(configBuffer);
            return executeLitmusTest(configString, kernelStr, iterations, options, xyStride);
        } catch (Exception e) {
            Log.e("executeQuickTest", e.getMessage());
            e.printStackTrace();
        }
        return "";
    }

    public native String executeLitmusTest(String configString,
                                           String kernelString,
                                           int iterations,
                                           String options,
                                           int x_y_stride);

    static {
        System.loadLibrary("litmus-test");
    }
}
