package ups.vision.proyectovision;


import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;


public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main); // Asegúrate de que este sea el nombre correcto de tu layout

        EdgeToEdge.enable(this);

        Button captura = findViewById(R.id.camara_main);

        captura.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent= new Intent(MainActivity.this, CamaraActivity.class);
                startActivity(intent); // Utiliza startActivity() en lugar de startActivities()
            }
        });
    }
}
