package ups.vision.proyectovision;


import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

public class SecondActivity extends AppCompatActivity {
    Bitmap bitmapOriginal, outputBitmap;
    private ImageView verImgOriginal;

    static {
        System.loadLibrary("parte2-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_second);
        verImgOriginal= findViewById(R.id.imgOriginal1);
        Button btnCamara = findViewById(R.id.btnCamara2);
        Button btnParte1 = findViewById(R.id.btnParte1);
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
        Intent intent = getIntent();
        byte[] byteArray = intent.getByteArrayExtra("capturedImage");
        if (byteArray != null) {
            bitmapOriginal = BitmapFactory.decodeByteArray(byteArray, 0, byteArray.length);
            verImgOriginal.setImageBitmap(bitmapOriginal);
            verImgOriginal.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
    }

}
