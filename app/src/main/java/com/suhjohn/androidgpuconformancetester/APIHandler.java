package com.suhjohn.androidgpuconformancetester;

import android.util.Log;

import com.loopj.android.http.JsonHttpResponseHandler;

import org.json.JSONException;
import org.json.JSONObject;

import cz.msebera.android.httpclient.Header;

public class APIHandler {
    public static void hello(final OnLoopjCompleted listener) {
        JsonHttpResponseHandler handler = new JsonHttpResponseHandler() {
            public void onSuccess(int statusCode, Header[] headers, JSONObject response) {
                // If the response is JSONObject instead of expected JSONArray
                Log.d("asd", "---------------- this is response : " + response);
                listener.taskCompleted(response);
            }
        };

        ServerRestClient.get("hello", null, handler);
    }
}
