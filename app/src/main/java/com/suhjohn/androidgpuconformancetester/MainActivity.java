package com.suhjohn.androidgpuconformancetester;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    public void executeQuickTest(View view) {
        Intent intent = new Intent(MainActivity.this, TestFinishedActivity.class);
        Bundle bundle = new Bundle();
        bundle.putInt("iteration", 1000);
        intent.putExtras(bundle);
        startActivity(intent);
    }

    public void createCustomTest(View view){
        Intent intent = new Intent(MainActivity.this, CustomTestActivity.class);
        startActivity(intent);
    }
}
