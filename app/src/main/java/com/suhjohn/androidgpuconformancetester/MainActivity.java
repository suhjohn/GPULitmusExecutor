package com.suhjohn.androidgpuconformancetester;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void executeQuickTest(View view) {
        Intent intent = new Intent(MainActivity.this, TestFinishedActivity.class);
        startActivity(intent);
    }

    public void createCustomTest(View view) {
        Intent intent = new Intent(MainActivity.this, CustomTestActivity.class);
        startActivity(intent);
    }

    public void executeFullTest(View view) {
        Intent intent = new Intent(MainActivity.this, FullTestActivity.class);
        startActivity(intent);
    }
}
