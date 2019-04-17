package com.suhjohn.androidgpuconformancetester;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.support.design.widget.TextInputLayout;
import android.os.Bundle;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;

public class MainActivitiy extends Activity {
    TextInputLayout iterationInputWrapper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        setContentView(R.layout.activity_main);
        iterationInputWrapper = (TextInputLayout)
                findViewById(R.id.activity_main_test_start_form_iteration);

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
        Intent intent = new Intent(MainActivitiy.this, TestFinishedActivity.class);
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
