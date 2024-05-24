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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;


public class MainActivity extends AppCompatActivity {

    ImageView imagenTomada, imagenFiltro;
    Bitmap bitmapOriginal, outputBitmap;

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
        Button btnBorde = findViewById(R.id.btnBorde);
        Button btnEnviar = findViewById(R.id.btnEnviar);

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
        btnBorde.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (bitmapOriginal != null) {
                    outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                    detectorBordes(bitmapOriginal, outputBitmap);
                    System.out.println("boton borde");
                    imagenFiltro.setImageBitmap(outputBitmap);
                } else {
                    Toast.makeText(MainActivity.this, "No image to filter", Toast.LENGTH_SHORT).show();
                }
            }
        });
        btnEnviar.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v) {

                // Envía la imagen transformada al servidor Flask
                ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
                outputBitmap.compress(Bitmap.CompressFormat.PNG, 100, outputStream);
                byte[] byteArray = outputStream.toByteArray();
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            Socket socket = new Socket("192.168.0.114", 3500);
                            OutputStream out = socket.getOutputStream();
                            out.write(byteArray);
                            out.flush();
                            out.close();
                            socket.close();
                            System.out.println("Imagen Enviada");
                        }
                        catch (IOException e){
                            e.printStackTrace();
                        }
                    }
                }).start();
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
    private native void detectorBordes(android.graphics.Bitmap in, android.graphics.Bitmap out);
}
