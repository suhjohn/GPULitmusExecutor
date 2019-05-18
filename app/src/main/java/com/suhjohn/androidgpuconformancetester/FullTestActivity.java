package com.suhjohn.androidgpuconformancetester;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import com.loopj.android.http.JsonHttpResponseHandler;
import com.suhjohn.androidgpuconformancetester.utils.LitmusTest;
import com.suhjohn.androidgpuconformancetester.utils.TestFileHandler;

import org.json.JSONObject;

import java.util.Arrays;

import cz.msebera.android.httpclient.Header;
import cz.msebera.android.httpclient.entity.StringEntity;
import cz.msebera.android.httpclient.message.BasicHeader;
import cz.msebera.android.httpclient.protocol.HTTP;

public class FullTestActivity extends AppCompatActivity {
    private TextView tv;
    private static Integer iterations = 10000;
    private static String[] tests = {"MP", "LB", "SB"};
    // TODO change to kv
    private static Integer[][] options = {
            {0, 1, 2}, // LITMUS_TEST_INDEXES
            {1, 2, 4, 8, 16, 32, 64, 128}, // X_Y_STRIDE
            {0, 1}, // MEM_STRESS
            {2, 16, 64, 256, 512}, // STRESS_ITERATIONS
            {0, 1, 2, 3}, // STRESS_PATTERN
            {0, 1}, // PRE_STRESS
            {2, 16, 64, 256, 512}, // PRE_STRESS_ITERATIONS
            {0, 1, 2, 3}, // PRE_STRESS_PATTERN
            {0, 1}, // BARRIER
            {0, 1}, // ID_SHUFFLE
//            {0, 1},
//            {0, 1},
//            {0, 1},
//            {0, 1},
//            {0, 1},
//            {0, 1},
//            {0, 1},
//            {0, 1},
//            {0, 1},
//            {0, 1}
    };
    private static String[] optionNames = {
            "",
            "",
            "MEM_STRESS",
            "STRESS_ITERATIONS",
            "STRESS_PATTERN",
            "PRE_STRESS",
            "PRE_STRESS_ITERATIONS",
            "PRE_STRESS_PATTERN",
            "BARRIER",
            "ID_SHUFFLE"
    };

    private static Integer[] maxOptionIndexes = new Integer[options.length];

    static {
        for (int i = 0; i < options.length; i++) {
            maxOptionIndexes[i] = options[i].length - 1;
        }
    }

    private static Integer[] getNextPermutatedOption(Integer[] optionIndexes) throws Exception {
        if (Arrays.equals(optionIndexes, maxOptionIndexes)) {
            throw new Exception("[MaxOptionException] end of permutation.");
        }
        Integer[] nextPermutation =
                getNextPermutatedOption(optionIndexes, optionIndexes.length - 1);
        while (nextPermutation[2] == 0 && (nextPermutation[3] != 0 || nextPermutation[4] != 0)) {
            nextPermutation = getNextPermutatedOption(
                    nextPermutation, optionIndexes.length - 1);
        }
        while (nextPermutation[5] == 0 && (nextPermutation[6] != 0 || nextPermutation[7] != 0)) {
            nextPermutation = getNextPermutatedOption(
                    nextPermutation, optionIndexes.length - 1);
        }
        return nextPermutation;
    }

    private static Integer[] getNextPermutatedOption(Integer[] optionIndexes, int index) {
        if (optionIndexes[index] == FullTestActivity.options[index].length - 1) {
            optionIndexes[index] = 0;
            return getNextPermutatedOption(optionIndexes, index - 1);
        }
        optionIndexes[index] += 1;
        return optionIndexes;
    }

    // Figure out solution for memory leak
    private class LitmustTestTask extends AsyncTask<Integer, Integer, String> {
        private Integer[] currentOptionIndexes;
        private String litmusTestType;
        private Integer xyStride;

        protected String doInBackground(Integer[] optionIndexes) {
            String kernelString;
            String configString;
            StringBuilder executionOptionBuilder = new StringBuilder();
            int litmusTestIndex = optionIndexes[0];
            int xyStride = options[1][optionIndexes[1]];
            String litmusTestType = tests[litmusTestIndex];
            TestFileHandler testFileHandler = new TestFileHandler(
                    getAssets(), litmusTestType);
            try {
                kernelString = testFileHandler.readKernel();
                configString = testFileHandler.readConfig();
            } catch (Exception e) {
                Log.e("doInBackground", e.getMessage());
                return "";
            }
            for (int i = 2; i < options.length; i++) {
                int optionValue = options[i][optionIndexes[i]];
                executionOptionBuilder.append(
                        String.format("-D %s=%d ", optionNames[i], optionValue)
                );
            }
            String executionOptions = executionOptionBuilder.toString();
            LitmusTest litmusTest = new LitmusTest(
                    kernelString, configString, executionOptions, iterations, xyStride);
            Log.i("doInBackground", "Executing test with following indexes");
            Log.i("doInBackground", Arrays.toString(optionIndexes));
            this.currentOptionIndexes = optionIndexes.clone();
            this.litmusTestType = litmusTestType;
            this.xyStride = xyStride;
            return litmusTest.execute();
        }

        protected void onPostExecute(String response) {
            Log.i("onPostExecute", response);
            try {
                JSONObject requestData = new JSONObject();
                requestData.put("result", response);
                requestData.put("test_type", this.litmusTestType);
                requestData.put("x_y_stride", this.xyStride);
                StringEntity entity = new StringEntity(requestData.toString());
                entity.setContentType(new BasicHeader(HTTP.CONTENT_TYPE, "application/json"));
                ServerRestClient.post("test", entity, new JsonHttpResponseHandler() {
                    public void onSuccess(int statusCode, Header[] headers, JSONObject response) {
//                        Log.i("post.onSuccess", response.toString());
//                        Log.i("post.onSuccess", Integer.toString(statusCode));
                    }
                });
            } catch (Exception e) {
                Log.e("onPostExecute", e.getMessage());
            }
            tv.setText(response);
            try {
                Integer[] nextOptionIndexes = getNextPermutatedOption(this.currentOptionIndexes);
                new LitmustTestTask().execute(nextOptionIndexes);
            } catch (Exception e) {
                Log.e("onPostExecute", e.getMessage());
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_finished);
        tv = findViewById(R.id.activity_test_finished_result);
        // Permutation execution
        Integer[] startOptionIndexes = new Integer[options.length];
        for (int i = 0; i < startOptionIndexes.length; i++) {
            startOptionIndexes[i] = 0;
        }
        startOptionIndexes[0] = 0;
//        Integer[] startOptionIndexes = {0, 3, 0, 0, 0, 0, 0, 0, 0, 0};
        new LitmustTestTask().execute(startOptionIndexes);
    }
}
