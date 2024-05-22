package ups.vision.proyectovision;


import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;


public class MainActivity extends AppCompatActivity {

    ImageView imagenTomada, imagenFiltro;
    Bitmap bitmapOriginal;
    static {
        System.loadLibrary("proyectovision");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main); // Asegúrate de que este sea el nombre correcto de tu layout

        EdgeToEdge.enable(this);

        Button captura = findViewById(R.id.camara_main);
        Button btnFiltro = findViewById(R.id.btnFiltro);
        imagenTomada= findViewById(R.id.imgOriginal);
        imagenFiltro=findViewById(R.id.imgFiltro);
        captura.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent= new Intent(MainActivity.this, CamaraActivity.class);
                startActivity(intent);
            }
        });
        btnFiltro.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                System.out.println("boton filtro ----------------------------------------");
                if (bitmapOriginal != null) {
                    llamar(bitmapOriginal);
                } else {
                    Toast.makeText(MainActivity.this, "No image to filter", Toast.LENGTH_SHORT).show();
                }
            }
        });
        // Recibir y decodificar el byte array
        Intent intent = getIntent();
        byte[] byteArray = intent.getByteArrayExtra("capturedImage");
        if (byteArray != null) {
            bitmapOriginal = BitmapFactory.decodeByteArray(byteArray, 0, byteArray.length);
            imagenTomada.setImageBitmap(bitmapOriginal);
            imagenTomada.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
    }
    public void llamar(Bitmap bitmapOriginal){
        Bitmap outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
        imagenGris(outputBitmap);
        imagenFiltro.setImageBitmap(outputBitmap);
    }

    native void imagenGris(Bitmap bitmap);
}
