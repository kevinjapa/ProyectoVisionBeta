package ups.vision.proyectovision;


import org.opencv.android.CameraActivity;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.Manifest;


import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import java.util.Collections;
import java.util.List;

import javax.security.auth.Destroyable;

public class MainActivity extends CameraActivity {

    CameraBridgeViewBase cameraBridgeViewBase;

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getPermission();

        cameraBridgeViewBase = findViewById(R.id.cameraView);

        cameraBridgeViewBase.setCvCameraViewListener(new CameraBridgeViewBase.CvCameraViewListener2(){
            @Override
            public void onCameraViewStarted(int width, int heigt){}
            @Override
            public void onCameraViewStopped() {}
            @Override
            public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame){
                return inputFrame.rgba();
            }
        });

        if(OpenCVLoader.initDebug()) {
            cameraBridgeViewBase.enableView();
        }
    }

    @Override
    protected void onResume(){
        super.onResume();
    cameraBridgeViewBase.enableView();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        cameraBridgeViewBase.disableView();
    }

    @Override
    protected void onPause(){
        super.onPause();
        cameraBridgeViewBase.disableView();
    }


    @Override
    protected List<? extends CameraBridgeViewBase> getCameraViewList(){
        return Collections.singletonList(cameraBridgeViewBase);
    }

    void getPermission(){
        if(checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED){
            requestPermissions(new String[]{Manifest.permission.CAMERA}, 101);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults){
        super.onRequestPermissionsResult(requestCode,permissions, grantResults);
        if(grantResults.length>0 && grantResults[0]!= PackageManager.PERMISSION_GRANTED){

        }
    }

}