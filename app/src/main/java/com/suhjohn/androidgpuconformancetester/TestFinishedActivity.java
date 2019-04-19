package com.suhjohn.androidgpuconformancetester;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.loopj.android.http.RequestParams;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public class TestFinishedActivity extends AppCompatActivity implements OnLoopjCompleted {
    private final String commonHeaderFilepath = "tests/testing_common.h";
    private TextView tv;
    private ProgressBar progressBar;
    private int iteration;
    private String kernelFile;
    private String configFile;

    private class LitmustTestTask extends AsyncTask<String, Integer, String> {
        protected String doInBackground(String... arguments) {
            return executeTest(arguments[0], arguments[1]);
        }

        protected void onProgressUpdate(Integer... progress) {
        }

        /***
         * response looks like the following:
         * {
         *  xydistance + offset:scratch_location: TestIterationObject
         * }
         * TODO figure out how to display information in a useful manner
         * TODO send the data to the server
         * @param response
         */
        protected void onPostExecute(String response) {
//            try {
//                JSONObject responseJSON = new JSONObject(response);
//                Iterator<String> keys = responseJSON.keys();
//                List<String> keysList = new ArrayList<>();
//                while (keys.hasNext()) keysList.add(keys.next());
//                Collections.sort(keysList);
//                StringBuilder displayTextBuilder = new StringBuilder();
//                for (String key : keysList) {
//                    JSONObject testResult = (JSONObject) responseJSON.get(key);
//                    displayTextBuilder.append(key);
//                    displayTextBuilder.append("\n");
//                    Iterator<String> testKeys = testResult.keys();
//                    while (testKeys.hasNext()){
//                        String testKey = testKeys.next();
//                        displayTextBuilder.append("        ");
//                        displayTextBuilder.append(testKey + "  " + testResult.get(testKey));
//                        displayTextBuilder.append("\n");
//                    }
//                }
//                tv.setText(displayTextBuilder.toString());
//            } catch (Exception e) {
//                Log.e("onPostExecute:Error", e.getMessage());
//            }
            // StringEntity entity = new StringEntity(jsonObj.toString());
            // entity.setContentType(new BasicHeader(HTTP.CONTENT_TYPE, "application/json"));
            // ServerRestClient.post("test", entity, this);
            tv.setText(response);
        }
    }

    public void taskCompleted(JSONObject response) {
        Log.i("taskCompleted", "Upload complete");
    }

    private void setIteration() {
        Bundle bundle = getIntent().getExtras();
        try {
            iteration = bundle.getInt("iteration");
        } catch (Exception e) {
            iteration = 10;
        }
    }

    // TODO: Take test types as parameters and set the filepaths
    private void setTestFilePath() {
        kernelFile = "tests/MP/kernel.cl";
        configFile = "tests/MP/config.txt";
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setIteration();
        this.setTestFilePath();
        setContentView(R.layout.activity_test_finished);
        tv = findViewById(R.id.activity_test_finished_result);
        progressBar = findViewById(R.id.activity_test_finished_progressbar);
        new LitmustTestTask().execute(kernelFile, configFile);
    }

    protected String executeTest(String kernelFile, String configFile) {
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
