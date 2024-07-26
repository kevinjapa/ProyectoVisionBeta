package ups.vision.proyectovision;


import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;

public class SecondActivity extends AppCompatActivity {
    Bitmap bitmapOriginal, outputBitmap;
    private ImageView verImgOriginal, verImgOutput;

    static {
        System.loadLibrary("parte2-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_second);
        verImgOriginal= findViewById(R.id.imgOriginal1);
        verImgOutput= findViewById(R.id.imgOutputReconociento);
        Button btnCamara = findViewById(R.id.btnCamara2);
        Button btnParte1 = findViewById(R.id.btnParte1);
        Button btnReconocer = findViewById(R.id.btnReconocer);
        Button btnHug = findViewById(R.id.btnHog);
        initAssetManager(getAssets());
//        AssetManager assetManager = getAssets();
//        String cacheDir = getCacheDir().getAbsolutePath();
//        loadCascade(assetManager, cacheDir);
        btnCamara.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent camara = new Intent( SecondActivity.this, CamaraActivity.class);
                startActivity(camara);
                finish();
            }
        });
        btnParte1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent parte1= new Intent(SecondActivity.this,MainActivity.class);
                startActivity(parte1);
                finish();
            }
        });
        btnReconocer.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.out.println("--------------------------------------");
                outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                reconocimiento(bitmapOriginal,outputBitmap);
                verImgOutput.setImageBitmap(outputBitmap);
                System.out.println("Si esta entrando en la parte ----------------------------");
            }
        });
        btnHug.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                verImgOutput.setImageBitmap(bitmapOriginal);
//                outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
                bitmapOriginal.compress(Bitmap.CompressFormat.PNG, 100, outputStream);
                byte[] byteArray = outputStream.toByteArray();
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
//                                    Socket socket = new Socket("192.168.0.112", 3500);
                            Socket socket = new Socket("192.168.0.111", 3500);
                            OutputStream out = socket.getOutputStream();
                            out.write(byteArray);
                            out.flush();
                            out.close();
                            socket.close();
                            System.out.println("Imagen Enviada");
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    Toast.makeText(getApplicationContext(), "Imagen Enviado", Toast.LENGTH_SHORT).show();
                                }
                            });
                        }
                        catch (IOException e){
                            e.printStackTrace();
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    Toast.makeText(getApplicationContext(), "Error", Toast.LENGTH_SHORT).show();
                                }
                            });
                        }
                    }
                }).start();
            }
        });
        Intent intent = getIntent();
        byte[] byteArray = intent.getByteArrayExtra("capturedImage");
        if (byteArray != null) {
            bitmapOriginal = BitmapFactory.decodeByteArray(byteArray, 0, byteArray.length);
            verImgOriginal.setImageBitmap(bitmapOriginal);
            verImgOriginal.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
    }
// definicion de clases nativas de c++
    private native void reconocimiento(android.graphics.Bitmap in, android.graphics.Bitmap out);
    public native void initAssetManager(AssetManager assetManager);
}
