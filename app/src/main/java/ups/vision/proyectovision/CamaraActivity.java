package ups.vision.proyectovision;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.annotation.NonNull;

import org.opencv.android.CameraActivity;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.Mat;

import java.io.ByteArrayOutputStream;
import java.util.Collections;
import java.util.List;

public class CamaraActivity extends CameraActivity {
    Mat frame;

    CameraBridgeViewBase cameraBridgeViewBase;
    //private ImageView imagenTomada;


    @Override
    protected void onCreate(Bundle savedInstanceState){


        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camara);

        getPermission();

        cameraBridgeViewBase = findViewById(R.id.cameraView);
        //imagenTomada = findViewById(R.id.resultado);
        //tomar la foto boton
        Button captura = findViewById(R.id.captura);

        cameraBridgeViewBase.setCvCameraViewListener(new CameraBridgeViewBase.CvCameraViewListener2(){
            @Override
            public void onCameraViewStarted(int width, int heigt){
                frame = new Mat();
            }
            @Override
            public void onCameraViewStopped() {
                frame.release();
            }
            @Override
            public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame){
                frame = inputFrame.rgba();
                return frame;
            }
        });

        if(OpenCVLoader.initDebug()) {
            cameraBridgeViewBase.enableView();
        }

        //acciones del boton para que capture la foto
        captura.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(frame != null){
                    System.out.println(frame.size());
                    captureImagen();
                }
            }
        });

    }
    //private native void imagenGris(Mat imagen);

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

        Bitmap bitmap = Bitmap.createBitmap(frame.cols(), frame.rows(), Bitmap.Config.ARGB_8888);
        Utils.matToBitmap(frame, bitmap);

//        imagenTomada.setImageBitmap(bitmap);
//        imagenTomada.setScaleType(ImageView.ScaleType.FIT_CENTER);

        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream);
        byte[] byteArray = stream.toByteArray();

        Intent intent = new Intent(CamaraActivity.this, MainActivity.class);
        intent.putExtra("capturedImage", byteArray);
        startActivity(intent);
        finish();
        System.out.println("finaliza --------------------------------------------------");
    }
}