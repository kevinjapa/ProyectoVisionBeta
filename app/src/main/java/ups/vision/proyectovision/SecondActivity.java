package ups.vision.proyectovision;


import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

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
