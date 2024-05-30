package ups.vision.proyectovision;


import android.app.ActivityManager;
import android.app.AppOpsManager;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Debug;
import android.os.Handler;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
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
    TextView txtRam,txtMensaje;
    String mensaje;
    Runnable runnable;

    static {
        System.loadLibrary("proyectovision");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        EdgeToEdge.enable(this);

        Button captura = findViewById(R.id.camara_main);
        Button btnIluminacion = findViewById(R.id.btnFiltro);
        Button btnFondo = findViewById(R.id.btnFondo);
        Button btnEnviar = findViewById(R.id.btnEnviar);
        Button btnFondoVerde =findViewById(R.id.tbnFondoVerde);
        EditText txtIP = findViewById(R.id.txtIP);
        txtRam = findViewById(R.id.lblRAM);
        imagenTomada= findViewById(R.id.imgOriginal);
        imagenFiltro=findViewById(R.id.imgFiltro);
        txtMensaje = findViewById(R.id.txtMensaje);

        Handler handler = new Handler();

        if (!hasUsageStatsPermission()) {
            requestUsageStatsPermission();
        }else{
            runnable = new Runnable() {
                @Override
                public void run() {
                    updateUsageStats();
                    handler.postDelayed(this, 5000);
                }
            };
        }
        handler.post(runnable);
        captura.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent= new Intent(MainActivity.this, CamaraActivity.class);
                startActivity(intent);
                finish();
            }
        });
        btnIluminacion.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                if (bitmapOriginal != null) {
                    outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                    iluminacion(bitmapOriginal,outputBitmap);
                    imagenFiltro.setImageBitmap(outputBitmap);
                } else {
                    Toast.makeText(MainActivity.this, "Imagen Vacio", Toast.LENGTH_SHORT).show();
                }
            }
        });
        btnFondo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mensaje=txtMensaje.getText().toString();
                if (mensaje.isEmpty())
                    mensaje=" ";
                if (bitmapOriginal != null) {
                    outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                    medianBlur(bitmapOriginal, outputBitmap);
//                    detectorBordes(bitmapOriginal, outputBitmap);
//                    efectoCartoon(outputBitmap,outputBitmap);
                    fondoVerdeCartoon(outputBitmap, outputBitmap);
                    textoImagen(outputBitmap,outputBitmap,mensaje);
                    imagenFiltro.setImageBitmap(outputBitmap);
                } else {
                    Toast.makeText(MainActivity.this, "No image to filter", Toast.LENGTH_SHORT).show();
                }
            }
        });
        btnFondoVerde.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(bitmapOriginal != null){
                    outputBitmap = bitmapOriginal.copy(bitmapOriginal.getConfig(), true);
                    fondoVerde(bitmapOriginal,outputBitmap);
                    imagenFiltro.setImageBitmap(outputBitmap);
                }
                else Toast.makeText(MainActivity.this, "Sin imagen", Toast.LENGTH_SHORT).show();

            }
        });
        btnEnviar.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v) {
                System.out.println("direccion"+txtIP.getText().toString());
                if (txtIP.getText().toString().isEmpty())
                {
                    Toast.makeText(MainActivity.this, "Ingresa una IP", Toast.LENGTH_SHORT).show();
                }
                else {
                    if(outputBitmap != null){
                        // Envía la imagen transformada al servidor Flask
                        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
                        outputBitmap.compress(Bitmap.CompressFormat.PNG, 100, outputStream);
                        byte[] byteArray = outputStream.toByteArray();
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                try {
//                                    Socket socket = new Socket("192.168.0.112", 3500);
                                    Socket socket = new Socket(txtIP.getText().toString(), 3500);
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
                    else Toast.makeText(MainActivity.this, "Sin imagen", Toast.LENGTH_SHORT).show();
                }
            }
        });
        Intent intent = getIntent();
        byte[] byteArray = intent.getByteArrayExtra("capturedImage");
        if (byteArray != null) {
            bitmapOriginal = BitmapFactory.decodeByteArray(byteArray, 0, byteArray.length);
            imagenTomada.setImageBitmap(bitmapOriginal);
            imagenTomada.setScaleType(ImageView.ScaleType.FIT_CENTER);
        }
    }
    //Filtros
    private native void fondoVerdeCartoon(android.graphics.Bitmap in, android.graphics.Bitmap out);
    private native void medianBlur(android.graphics.Bitmap in, android.graphics.Bitmap out);
    private native void iluminacion(android.graphics.Bitmap in, android.graphics.Bitmap out);
    private native void textoImagen(android.graphics.Bitmap in, android.graphics.Bitmap out, String mensaje);
    private native void fondoVerde(android.graphics.Bitmap in, android.graphics.Bitmap out);


    //USO DE RECURSOS
    private void updateUsageStats() {
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        int[] pids = { android.os.Process.myPid() };
        ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
        activityManager.getMemoryInfo(memoryInfo);
        long totalMemory = memoryInfo.totalMem;
        Debug.MemoryInfo[] memoryInfoArray = activityManager.getProcessMemoryInfo(pids);
        long usedMemoryApp = memoryInfoArray[0].getTotalPss() * 1024L;

        String ramUsageApp = usedMemoryApp / (1024 * 1024) + "MB";
        txtRam.setText(ramUsageApp);
        System.out.println("Ram usada; "+ramUsageApp);

    }

    private boolean hasUsageStatsPermission() {
        AppOpsManager appOps = (AppOpsManager) getSystemService(Context.APP_OPS_SERVICE);
        int mode = appOps.checkOpNoThrow(AppOpsManager.OPSTR_GET_USAGE_STATS, android.os.Process.myUid(), getPackageName());
        return mode == AppOpsManager.MODE_ALLOWED;
    }
    private void requestUsageStatsPermission() {
        Intent intent = new Intent(Settings.ACTION_USAGE_ACCESS_SETTINGS);
        startActivity(intent);
    }


}
