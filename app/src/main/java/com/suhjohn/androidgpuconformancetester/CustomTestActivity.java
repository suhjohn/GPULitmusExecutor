package com.suhjohn.androidgpuconformancetester;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.TextInputLayout;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;

import java.util.HashSet;

public class CustomTestActivity extends AppCompatActivity {
    TextInputLayout iterationInputWrapper;
    Spinner litmusTestTypeSpinner;
    Spinner stressPatternSpinner;
    Spinner preStressPatternSpinner;

    CheckBox barrierCheckBox;
    CheckBox preStressCheckBox;
    CheckBox shuffleIdCheckBox;
    CheckBox subgroupPreserveCheckBox;
    CheckBox memoryStressCheckBox;

    TextInputLayout xyzStrideInputWrapper;
    TextInputLayout stressIterationsInputWrapper;
    TextInputLayout preStressIterationsInputWrapper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        setContentView(R.layout.activity_custom_test);

        iterationInputWrapper = (TextInputLayout)
                findViewById(R.id.activity_custom_test_iteration_layout);
        litmusTestTypeSpinner = (Spinner)
                findViewById(R.id.activity_custom_litmus_test_type_dropdown);
        stressPatternSpinner = (Spinner)
                findViewById(R.id.activity_custom_test_stress_pattern_dropdown);
        preStressPatternSpinner = (Spinner)
                findViewById(R.id.activity_custom_test_pre_stress_pattern_dropdown);

        barrierCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_barrier_checkbox);
        preStressCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_pre_stress_checkbox);
        shuffleIdCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_shuffle_id_checkbox);
        subgroupPreserveCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_subgroup_preserve_checkbox);
        memoryStressCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_mem_stress_checkbox);

        xyzStrideInputWrapper = (TextInputLayout)
                findViewById(R.id.activity_custom_test_x_y_z_stride_layout);
        stressIterationsInputWrapper = (TextInputLayout)
                findViewById(R.id.activity_custom_test_stress_iterations_layout);
        preStressIterationsInputWrapper = (TextInputLayout)
                findViewById(R.id.activity_custom_test_pre_stress_iterations_layout);
    }

    private void hideKeyboard() {
        View view = getCurrentFocus();
        if (view != null) {
            ((InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE)).
                    hideSoftInputFromWindow(view.getWindowToken(),
                            InputMethodManager.HIDE_NOT_ALWAYS);
        }
    }

    public void executeTest(View view) {
        hideKeyboard();

        // TODO add stressPattern
        int stressPattern = stressPatternSpinner.getSelectedItemPosition();
        int preStressPattern = preStressPatternSpinner.getSelectedItemPosition();
        String litmusTestType = litmusTestTypeSpinner.getSelectedItem().toString();

        boolean isBarrier = barrierCheckBox.isChecked();
        boolean isPreStress = preStressCheckBox.isChecked();
        boolean isShuffleID = shuffleIdCheckBox.isChecked();
        boolean isSubgroupPreserve = subgroupPreserveCheckBox.isChecked();
        boolean isMemStress = memoryStressCheckBox.isChecked();

        EditText iterationText = iterationInputWrapper.getEditText();
        EditText xyzStrideText = xyzStrideInputWrapper.getEditText();
        EditText stressIterationsText = stressIterationsInputWrapper.getEditText();
        EditText preStressIterationsText = preStressIterationsInputWrapper.getEditText();

        HashSet<Boolean> validationResults = new HashSet<>();
        String iterationsStr = iterationText.getText().toString();
        String xyzStrideStr = xyzStrideText.getText().toString();
        String stressIterationsStr = stressIterationsText.getText().toString();
        String preStressIterationsStr = preStressIterationsText.getText().toString();

        validationResults.add(validateIterationInput(iterationsStr));
        validationResults.add(validateXYZStrideInput(xyzStrideStr));
        validationResults.add(validateStressIterationsInput(stressIterationsStr));
        validationResults.add(validateStressIterationsInput(preStressIterationsStr));

        if (validationResults.contains(Boolean.FALSE)) return;

        // TODO: MEM_STRESS distinguishes between stress and non-stress
        StringBuilder executionOptionBuilder = new StringBuilder();
        if (stressPattern != 0) {
            executionOptionBuilder.append(
                    String.format("-D STRESS_PATTERN=%d ", stressPattern - 1));
        }
        if (preStressPattern != 0) {
            executionOptionBuilder.append(
                    String.format("-D PRE_STRESS_PATTERN=%d ", preStressPattern - 1));
        }
        if (isBarrier) {
            executionOptionBuilder.append("-D BARRIER=1 ");
        } else {
            executionOptionBuilder.append("-D BARRIER=0 ");
        }
        if (isPreStress) {
            executionOptionBuilder.append("-D PRE_STRESS=1 ");
        } else {
            executionOptionBuilder.append("-D PRE_STRESS=0 ");
        }
        if (isShuffleID) {
            executionOptionBuilder.append("-D ID_SHUFFLE=1 ");
        } else {
            executionOptionBuilder.append("-D ID_SHUFFLE=0 ");
        }
        if (isSubgroupPreserve) {
            executionOptionBuilder.append("-D SUBGROUP_PRESERVE=1 ");
        } else {
            executionOptionBuilder.append("-D SUBGROUP_PRESERVE=0 ");
        }
        if (isMemStress) {
            executionOptionBuilder.append("-D MEM_STRESS=1 ");
        } else {
            executionOptionBuilder.append("-D MEM_STRESS=0 ");
        }
        executionOptionBuilder.append(
                String.format("-D STRESS_ITERATIONS=%s ", stressIterationsStr));
        executionOptionBuilder.append(
                String.format("-D PRE_STRESS_ITERATIONS=%s ", preStressIterationsStr));


        Integer iterations = Integer.parseInt(iterationsStr);
        // TODO MODIFY X_Y_STRIDE to X_Y_Z when it gets fixed!
        Integer xyStride = Integer.parseInt(xyzStrideStr);
        Intent intent = new Intent(
                CustomTestActivity.this, TestFinishedActivity.class);
        Bundle bundle = new Bundle();

        bundle.putInt("iterations", iterations);
        bundle.putInt("xyStride", xyStride);
        bundle.putString("options", executionOptionBuilder.toString());
        bundle.putString("litmusTestType", litmusTestType);
        intent.putExtras(bundle);

        startActivity(intent);
    }

    private boolean validateIterationInput(String iteration) {
        if (iteration.matches("")) {
            iterationInputWrapper.setError(
                    "Enter a valid iteration number. Suggested iterations: 100 ~ 10000"
            );
            return false;
        }
        try {
            Integer.parseInt(iteration);
        } catch (NumberFormatException e) {
            iterationInputWrapper.setError(
                    "Enter a valid iteration number. Suggested iterations: 100 ~ 10000"
            );
            return false;
        }
        return true;
    }

    private boolean validateXYZStrideInput(String locStride) {
        int locStrideInt;
        if (locStride.matches("")) {
            xyzStrideInputWrapper.setError(
                    "Enter a valid integer between 1~128. "
            );
            return false;
        }
        try {
            locStrideInt = Integer.parseInt(locStride);
        } catch (NumberFormatException e) {
            xyzStrideInputWrapper.setError(
                    "Enter a valid xyzStride number. Default: 32"
            );
            return false;
        }
        if (locStrideInt > 128 || locStrideInt < 1) {
            xyzStrideInputWrapper.setError(
                    "xyzStride must be an integer between 1 ~ 128."
            );
            return false;
        }
        return true;
    }

    private boolean validateStressIterationsInput(String stressIterations) {
        if (stressIterations.matches("")) {
            stressIterationsInputWrapper.setError(
                    "Enter a valid integer between 1~1024. "
            );
            return false;
        }
        try {
            Integer.parseInt(stressIterations);
        } catch (NumberFormatException e) {
            xyzStrideInputWrapper.setError(
                    "Enter a valid xyzStride number. Default: 32"
            );
            return false;
        }
        return true;
    }

    // TODO not yet an argument
    private boolean validateScratchLocationInput(String scratchLocation) {
        return true;
    }

    // TODO not yet an argument
    private boolean validateScratchStrideInput(String scratchStride) {
        return true;
    }

}
