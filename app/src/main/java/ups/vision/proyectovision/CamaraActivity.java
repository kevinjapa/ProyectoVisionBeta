package ups.vision.proyectovision;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.view.Surface;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;

import org.opencv.android.CameraActivity;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.Mat;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

public class CamaraActivity extends CameraActivity {
    Mat frame;
    Bitmap bitmap;
    TextView lblFPS;
    private long lastTime = 0;
    private int framesCount = 0, fps = 0;

    CameraBridgeViewBase cameraBridgeViewBase;

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camara);

        getPermission();

        cameraBridgeViewBase = findViewById(R.id.cameraView);
        Button captura = findViewById(R.id.captura);
//        Button btnGrabar= findViewById(R.id.tbnGrabar);
        lblFPS= findViewById(R.id.lblFPS);
        lblFPS.setTextColor(getResources().getColor(R.color.white));
        cameraBridgeViewBase.setCvCameraViewListener(new CameraBridgeViewBase.CvCameraViewListener2(){
            @Override
            public void onCameraViewStarted(int width, int heigt){
                frame = new Mat();
                lastTime = System.currentTimeMillis();
            }
            @Override
            public void onCameraViewStopped() {
                frame.release();
            }
            @Override
            public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame){
                frame = inputFrame.rgba();
                //calculo de FPS
                framesCount++;
                long currentTime = System.currentTimeMillis();
                if (currentTime - lastTime >= 1000) {
                    fps = framesCount;
                    framesCount = 0;
                    lastTime = currentTime;
                    lblFPS.setText("FPS: "+fps);

//                    runOnUiThread(new Runnable() {
//                        @Override
//                        public void run() {
//                            lblFPS.setText("FPS: "+fps);
//                        }
//                    });
                }
                return frame;
            }
        });
        if(OpenCVLoader.initDebug()) {
            cameraBridgeViewBase.enableView();
        }
        captura.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(frame != null){
                    System.out.println(frame.size());
                    captureImagen();
                }
            }
        });
//        btnGrabar.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                if(isRecording){
////                    stopRecording();
//                    btnGrabar.setText("Grabar");
//                } else {
////                    startRecording();
//                    btnGrabar.setText("Detener");
//                }
//                isRecording = !isRecording;
//            }
//        });
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
        if(checkSelfPermission(android.Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED){
            requestPermissions(new String[]{Manifest.permission.CAMERA}, 101);
        }
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults){
        super.onRequestPermissionsResult(requestCode,permissions, grantResults);
        if(grantResults.length>0 && grantResults[0]!= PackageManager.PERMISSION_GRANTED){

        }
    }

    public void captureImagen() {
        Core.rotate(frame, frame, Core.ROTATE_90_CLOCKWISE);
        bitmap = Bitmap.createBitmap(frame.cols(), frame.rows(), Bitmap.Config.ARGB_8888);
        Utils.matToBitmap(frame, bitmap);
//        imagenTomada.setImageBitmap(bitmap);
//        imagenTomada.setScaleType(ImageView.ScaleType.FIT_CENTER);

        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream);
        byte[] byteArray = stream.toByteArray();

        int targetSize = 1000000; // 1MB
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();

        while (byteArray.length > targetSize && width > 1 && height > 1) {
            width /= 2;
            height /= 2;
            Bitmap resizedBitmap = Bitmap.createScaledBitmap(bitmap, width, height, true);
            stream.reset();
            resizedBitmap.compress(Bitmap.CompressFormat.PNG, 100, stream);
            byteArray = stream.toByteArray();
        }
        int quality = 50;
        while (byteArray.length > 1000000 && quality > 10) { // Ajustar si el tamaño es mayor a 1MB
            stream.reset();
            quality -= 10;
            bitmap.compress(Bitmap.CompressFormat.PNG, quality, stream);
            byteArray = stream.toByteArray();
        }

        Intent intent = new Intent(CamaraActivity.this, MainActivity.class);
        intent.putExtra("capturedImage", byteArray);
        startActivity(intent);
        finish();
    }
}