package ups.vision.proyectovision;


import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import androidx.appcompat.app.AppCompatActivity;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;

public class SecondActivity extends AppCompatActivity {
    Bitmap bitmapOriginal, outputBitmap;
    private ImageView verImgOriginal, verImgOutput;
    SharedPreferences sharedPreferences;
    EditText txtDireccionIp;

    static {
        System.loadLibrary("parte2-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_second);

        sharedPreferences = getSharedPreferences("MyPrefs", MODE_PRIVATE);

        verImgOriginal= findViewById(R.id.imgOriginal1);
        verImgOutput= findViewById(R.id.imgOutputReconociento);
        Button btnCamara = findViewById(R.id.btnCamara2);
        Button btnParte1 = findViewById(R.id.btnParte1);
        Button btnReconocer = findViewById(R.id.btnReconocer);
        Button btnHug = findViewById(R.id.btnHog);
        Button btnEnviarPuntos = findViewById(R.id.btnEnviarPuntos);
        TextView lblPrediccion = findViewById(R.id.lblPrediccion);
        txtDireccionIp = findViewById(R.id.txtDireccionIP2);
        initAssetManager(getAssets()); // inicalizamos los assets que tenemos de los modelos

        // Recuperar el valor almacenado en SharedPreferences y colocarlo en el TextView
        String savedIp = sharedPreferences.getString("saved_ip", "11");
        txtDireccionIp.setText(savedIp);

        btnCamara.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Guardar la dirección IP en SharedPreferences antes de cambiar de actividad
                String currentIp = txtDireccionIp.getText().toString();
                SharedPreferences.Editor editor = sharedPreferences.edit();
                editor.putString("saved_ip", currentIp);
                editor.apply();

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
                File directory = getFilesDir();
                String path = new File(directory, "output.csv").getAbsolutePath();

                outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                reconocimiento(bitmapOriginal,outputBitmap,path);
                readCSV(path);
                verImgOutput.setImageBitmap(outputBitmap);
                System.out.println("Si esta entrando en la parte ----------------------------");
            }
        });

        btnHug.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                String resultado=Predecir(bitmapOriginal,outputBitmap);
                verImgOutput.setImageBitmap(outputBitmap);
                lblPrediccion.setText(resultado);
            }
        });
        btnEnviarPuntos.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Obtener la ruta de almacenamiento interno
                File directory = getFilesDir();
                File csvFile = new File(directory, "output.csv");
                File imageFile = new File(directory, "output.png");

                // Guardar la imagen en un archivo
                try (FileOutputStream fos = new FileOutputStream(imageFile)) {
                    bitmapOriginal.compress(Bitmap.CompressFormat.PNG, 100, fos);
                } catch (IOException e) {
                    e.printStackTrace();
                    Toast.makeText(getApplicationContext(), "Error al guardar la imagen", Toast.LENGTH_SHORT).show();
                    return;
                }

                // Comprimir los archivos en un ZIP
                File zipFile = new File(directory, "data.zip");
                try {
                    zipFiles(new File[]{csvFile, imageFile}, zipFile);
                } catch (IOException e) {
                    e.printStackTrace();
                    Toast.makeText(getApplicationContext(), "Error al comprimir los archivos", Toast.LENGTH_SHORT).show();
                    return;
                }
                if (txtDireccionIp.getText().toString().isEmpty()){
                    Toast.makeText(getApplicationContext(), "Ingrese la Direccion", Toast.LENGTH_SHORT).show();
                }
                else
                {
                    // Enviar el archivo ZIP al servidor
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            try {
//                            Socket socket = new Socket("192.168.0.104", 3500);
                                Socket socket = new Socket(txtDireccionIp.getText().toString(), 3500);
                                OutputStream out = socket.getOutputStream();

                                FileInputStream fileInputStream = new FileInputStream(zipFile);
                                byte[] buffer = new byte[1024];
                                int bytesRead;
                                while ((bytesRead = fileInputStream.read(buffer)) != -1) {
                                    out.write(buffer, 0, bytesRead);
                                }
                                fileInputStream.close();
                                out.flush();
                                out.close();
                                socket.close();
                                System.out.println("ZIP Enviado");
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        Toast.makeText(getApplicationContext(), "ZIP Enviado", Toast.LENGTH_SHORT).show();
                                    }
                                });
                            } catch (IOException e) {
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
            }
        });


//      Clase para obtener la imagen de camara con open cv
        Intent intent = getIntent();
        byte[] byteArray = intent.getByteArrayExtra("capturedImage");
        if (byteArray != null) {
            bitmapOriginal = BitmapFactory.decodeByteArray(byteArray, 0, byteArray.length);
            verImgOriginal.setImageBitmap(bitmapOriginal);
            verImgOriginal.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
    }
    // Método para leer el archivo CSV
    private void readCSV(String path) {
        try (BufferedReader br = new BufferedReader(new FileReader(path))) {
            String line;
            while ((line = br.readLine()) != null) {
                String[] values = line.split(",");
                for (int i=0; i< values.length; i++)
//                    for (int j=0; j<= values.length+1)
                System.out.println(values[i]);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    //    Funcion para comprimir

    private void zipFiles(File[] files, File zipFile) throws IOException {
        try (ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zipFile))) {
            byte[] buffer = new byte[1024];
            for (File file : files) {
                try (FileInputStream fis = new FileInputStream(file)) {
                    zos.putNextEntry(new ZipEntry(file.getName()));
                    int length;
                    while ((length = fis.read(buffer)) > 0) {
                        zos.write(buffer, 0, length);
                    }
                    zos.closeEntry();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        }
    }

    // definicion de clases nativas de c++
    private native void reconocimiento(android.graphics.Bitmap in, android.graphics.Bitmap out, String path);
    public native void initAssetManager(AssetManager assetManager);
    private native String Predecir(android.graphics.Bitmap in, android.graphics.Bitmap out);
}
