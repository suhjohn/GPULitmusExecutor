package com.suhjohn.androidgpuconformancetester;

import org.json.JSONObject;

public interface OnLoopjCompleted {
    void taskCompleted(JSONObject results);
}