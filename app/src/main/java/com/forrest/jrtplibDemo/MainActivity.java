package com.forrest.jrtplibDemo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.forrest.jrtplib.JrtplibUtil;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 测试jlibrtp 调用，涉及底层涉及网络操作，新建一个线程调用接口
        new Thread() {
            @Override
            public void run() {
                new JrtplibUtil().test();
            }
        }.start();
    }
}
