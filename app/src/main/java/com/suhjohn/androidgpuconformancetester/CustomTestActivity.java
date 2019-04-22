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

public class CustomTestActivity extends AppCompatActivity {
    TextInputLayout iterationInputWrapper;
    CheckBox barrierCheckBox;
    CheckBox preStressCheckBox;
    CheckBox threadRandomizationCheckBox;
    CheckBox subgroupPreserveCheckBox;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        setContentView(R.layout.activity_main);
        iterationInputWrapper = (TextInputLayout)
                findViewById(R.id.activity_custom_test_iteration_input);
        barrierCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_barrier_checkbox);
        preStressCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_pre_stress_checkbox);
        threadRandomizationCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_thread_randomization_checkbox);
        subgroupPreserveCheckBox = (CheckBox)
                findViewById(R.id.activity_custom_test_subgroup_preserve_checkbox);
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
        EditText iterationText = iterationInputWrapper.getEditText();
        String iteration = iterationText.getText().toString();
        if (!validateIterationInput(iteration)) return;
        Integer iterationAsInt = Integer.parseInt(iteration);
        boolean isBarrier = barrierCheckBox.isChecked();
        boolean isPreStress = preStressCheckBox.isChecked();
        boolean isThreadRandom = threadRandomizationCheckBox.isChecked();
        boolean isSubgroupPreserve = subgroupPreserveCheckBox.isChecked();
        Intent intent = new Intent(
                CustomTestActivity.this, TestFinishedActivity.class);
        Bundle bundle = new Bundle();

        bundle.putInt("iteration", iterationAsInt);
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


}
