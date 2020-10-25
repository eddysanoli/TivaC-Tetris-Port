//Basado en la libreria por: 
// By Luke Cyca
// https://lukecyca.com
// https://github.com/lukecyca/TetrisThemeArduino

#define Buzzer  (PE_3)
#define _R     (0)
int inter;
float curr_lead_note_time_remaining;
const byte interruptPin = PF_4;
float curr_bass_note_time_remaining;
int curr_lead_note;
int curr_bass_note;
float lead_freq;
float bass_freq;
byte t;
volatile byte sonido = LOW;
// Note frequencies based on http://www.phy.mtu.edu/~suits/notefreqs.html
#define _C0    (16.35)
#define _CS0   (17.32)
#define _D0    (18.35)
#define _DS0   (19.45)
#define _E0    (20.60)
#define _F0    (21.83)
#define _FS0   (23.12)
#define _G0    (24.50)
#define _GS0   (25.96)
#define _A0    (27.50)
#define _AS0   (29.14)
#define _B0    (30.87)
#define _C1    (32.70)
#define _CS1   (34.65)
#define _D1    (36.71)
#define _DS1   (38.89)
#define _E1    (41.20)
#define _F1    (43.65)
#define _FS1   (46.25)
#define _G1    (49.00)
#define _GS1   (51.91)
#define _A1    (55.00)
#define _AS1   (58.27)
#define _B1    (61.74)
#define _C2    (65.41)
#define _CS2   (69.30)
#define _D2    (73.42)
#define _DS2   (77.78)
#define _E2    (82.41)
#define _F2    (87.31)
#define _FS2   (92.50)
#define _G2    (98.00)
#define _GS2   (103.83)
#define _A2    (110.00)
#define _AS2   (116.54)
#define _B2    (123.47)
#define _C3    (130.81)
#define _CS3   (138.59)
#define _D3    (146.83)
#define _DS3   (155.56)
#define _E3    (164.81)
#define _F3    (174.61)
#define _FS3   (185.00)
#define _G3    (196.00)
#define _GS3   (207.65)
#define _A3    (220.00)
#define _AS3   (233.08)
#define _B3    (246.94)
#define _C4    (261.63)
#define _CS4   (277.18)
#define _D4    (293.66)
#define _DS4   (311.13)
#define _E4    (329.63)
#define _F4    (349.23)
#define _FS4   (369.99)
#define _G4    (392.00)
#define _GS4   (415.30)
#define _A4    (440.00)
#define _AS4   (466.16)
#define _B4    (493.88)
#define _C5    (523.25)
#define _CS5   (554.37)
#define _D5    (587.33)
#define _DS5   (622.25)
#define _E5    (659.25)
#define _F5    (698.46)
#define _FS5   (739.99)
#define _G5    (783.99)
#define _GS5   (830.61)
#define _A5    (880.00)
#define _AS5   (932.33)
#define _B5    (987.77)
#define _C6    (1046.50)
#define _CS6   (1108.73)
#define _D6    (1174.66)
#define _DS6   (1244.51)
#define _E6    (1318.51)
#define _F6    (1396.91)
#define _FS6   (1479.98)
#define _G6    (1567.98)
#define _GS6   (1661.22)
#define _A6    (1760.00)
#define _AS6   (1864.66)
#define _B6    (1975.53)
#define _C7    (2093.00)
#define _CS7   (2217.46)
#define _D7    (2349.32)
#define _DS7   (2489.02)
#define _E7    (2637.02)
#define _F7    (2793.83)
#define _FS7   (2959.96)
#define _G7    (3135.96)
#define _GS7   (3322.44)
#define _A7    (3520.00)
#define _AS7   (3729.31)
#define _B7    (3951.07)
#define _C8    (4186.01)
#define _CS8   (4434.92)
#define _D8    (4698.63)
#define _DS8   (4978.03)
#define _E8    (5274.04)
#define _F8    (5587.65)
#define _FS8   (5919.91)
#define _G8    (6271.93)
#define _GS8   (6644.88)
#define _A8    (7040.00)
#define _AS8   (7458.62)
#define _B8    (7902.13)
// beats per minute
#define BPM   (120.0)

float lead_notes[] = {

  _E5, _B4, _C5, _D5, _C5, _B4, _A4, _A4, _C5, _E5, _D5, _C5, _B4, _B4, _C5, _D5, _E5, _C5, _A4, _A4, _R,
  _D5, _F5, _A5, _G5, _F5, _E5, _C5, _E5, _D5, _C5, _B4, _B4, _C5, _D5, _E5, _C5, _A4, _A4, _R,
 
  _E4, _C4, _D4, _B3, _C4, _A3, _GS3, _B3,
  _E4, _C4, _D4, _B3, _C4, _E4, _A4, _A4, _GS4, _R
};


float lead_times[] = {

  1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
  1.5, 0.5, 1.0, 0.5, 0.5, 1.5, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,

  2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0,
  2.0, 2.0, 2.0, 2.0, 1.0, 1.0, 1.0, 1.0, 3.0, 1.0
};

float bass_notes[] = {

  _E2, _E3, _E2, _E3, _E2, _E3, _E2, _E3, _A1, _A2, _A1, _A2, _A1, _A2, _A1, _A2, _GS1, _GS2, _GS1, _GS2, _GS1, _GS2, _GS1, _GS2, _A1, _A2, _A1, _A2, _A1, _B2, _C3, _E3,
  _D2, _D3, _D2, _D3, _D2, _D3, _D2, _D3, _C2, _C3, _C2, _C3, _C2, _C3, _C2, _C3, _B1, _B2, _B1, _B2, _B1, _B2, _B1, _B2, _A1, _A2, _A1, _A2, _A1, _A2, _A1, _A2,

  _A1, _E2, _A1, _E2, _A1, _E2, _A1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2, _A1, _E2, _A1, _E2, _A1, _E2, _A1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2,
  _A1, _E2, _A1, _E2, _A1, _E2, _A1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2, _A1, _E2, _A1, _E2, _A1, _E2, _A1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2, _GS1, _E2
};

float bass_times[] = {

  0.5,  0.5,  0.5,  0.5,  0.5,  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
  0.5,  0.5,  0.5,  0.5,  0.5,  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,

  0.5,  0.5,  0.5,  0.5,  0.5,  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
  0.5,  0.5,  0.5,  0.5,  0.5,  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//BEEP MENU
float Menu_lead_notes[] = {
  _A7, _R, _R, _R, _R, _R, _R
};
float Menu_lead_times[] = {
  0.05, 0.2, 0.5, 0.5, 0.5, 0.5, 0.5
};
float Menu_bass_notes[] = {
  _R, _R, _R, _R, _R, _R, _R
};
float Menu_bass_times[] = {
 0.05, 0.2, 0.5, 0.5, 0.5, 0.5, 0.5
};


//Seleccion en Menu
float Smenu_lead_notes[] = {
  _GS6, _A6, _E8, _B6, _C7, _R, _R
};
float Smenu_lead_times[] = {
  0.1, 0.1, 0.06, 0.1, 0.1, 0.1, 0.1
};
float Smenu_bass_notes[] = {
  _R, _R, _R, _R, _R, _R, _R
};
float Smenu_bass_times[] = {
 0.1, 0.1, 0.06, 0.1, 0.1, 0.1, 0.1
};


//Rotacion de Pieza
float Rot_lead_notes[] = {
  _A6, _E8, _R, _R, _R, _R
};
float Rot_lead_times[] = {
  0.06, 0.08, 0.1, 0.1, 0.1, 0.1
};
float Rot_bass_notes[] = {
 _R, _R, _R, _R, _R, _R
};
float Rot_bass_times[] = {
 0.06, 0.08, 0.1, 0.1, 0.1, 0.1
};

//Limpieza de Linea
float LimpL_lead_notes[] = {
  _D6, _A7, _DS6, _B7, _E6, _DS6, _B7, _E6, _GS6, _E8, _A6, _AS6, _F8, _DS6, _E6, _F6, _CS7,
  _D7, _DS7, _FS6, _G6, _FS7, _GS6, _R, _R,  _C7, _G7, _B7, _G7, _B7, _G7, _B7, _G7, _R, _R,
  _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R
};
float LimpL_lead_times[] = {
  0.06, 0.06, 0.06, 0.08, 0.06, 0.08, 0.08, 0.06, 0.06, 0.06, 0.04, 0.08, 0.06, 0.08, 0.06, 0.04, 0.06,
  0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.3, 0.3,     0.16, 0.18, 0.16, 0.16, 0.14, 0.14, 0.12, 0.1, 0.06, 
  0.06, 0.04, 0.08, 0.06, 0.08, 0.06, 0.04, 0.06, 0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.5, 0.5, 0.5, 0.5
};
float LimpL_bass_notes[] = {
 _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R,  _R, _R, _R, _R,
 _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R 
};
float LimpL_bass_times[] = {
  0.06, 0.06, 0.06, 0.08, 0.06, 0.08, 0.08, 0.06, 0.06, 0.06, 0.04, 0.08, 0.06, 0.08, 0.06, 0.04, 0.06,
  0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.3, 0.3,      0.16, 0.18, 0.16, 0.16, 0.14, 0.14, 0.12, 0.1, 0.06,
  0.06, 0.04, 0.08, 0.06, 0.08, 0.06, 0.04, 0.06, 0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.5, 0.5, 0.5, 0.5
};

//Nuevo Nivel
float NuevNiv_lead_notes[] = {
  _E6, _G1, _C7, _E6, _B7, _C7, _E6, _B7, _G6, _D8, _E7, _G6, _D8, _E7, _G6, _D8, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R
};
float NuevNiv_lead_times[] = {
  0.06, 0.04, 0.1, 0.1, 0.06, 0.1, 0.1, 0.08, 0.1, 0.06, 0.1, 0.1, 0.08, 0.1, 0.1, 0.08, 0.06,
  0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.5, 0.5, 0.5, 0.5
};
float NuevNiv_bass_notes[] = {
 _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R
};
float NuevNiv_bass_times[] = {
  0.06, 0.04, 0.1, 0.1, 0.06, 0.1, 0.1, 0.08, 0.1, 0.06, 0.1, 0.1, 0.08, 0.1, 0.1, 0.08, 0.06,
  0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.5, 0.5, 0.5, 0.5
};

//Colision
float Col_lead_notes[] = {
  _C8, _B7, _CS8, _D8, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R
};
float Col_lead_times[] = {
  0.08, 0.06, 0.08, 0.2, 0.06, 0.1, 0.1, 0.08, 0.1, 0.06, 0.1, 0.1, 0.08, 0.1, 0.1, 0.08, 0.06,
  0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.5, 0.5, 0.5, 0.5
};
float Col_bass_notes[] = {
 _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R, _R
};
float Col_bass_times[] = {
  0.08, 0.06, 0.08, 0.2, 0.06, 0.1, 0.1, 0.08, 0.1, 0.06, 0.1, 0.1, 0.08, 0.1, 0.1, 0.08, 0.06,
  0.04, 0.06, 0.06, 0.06, 0.06, 0.06, 0.5, 0.5, 0.5, 0.5
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// duration is in microseconds
void play_one_note(float frequency, unsigned long duration) {
  unsigned long period = 1000000.0/frequency;

  for (unsigned int cycles=duration/period; cycles>0; cycles--) {
    digitalWrite(Buzzer, HIGH);//Durante la mitad del periodo se mantiene encendido y durante la otra mitad se apaga
    delayMicroseconds( period/2 );
    digitalWrite(Buzzer, LOW);
    delayMicroseconds( period/2 );
  }
    delayMicroseconds(duration % period); //Si la duracion de la nota no era un multiplo del periodo se retrasa el sobrante
}
// El tiempo qeu se pasa en cada nota mientras se simula polifonia(varias notas tocandose al mismo tiempo)
// Si tiene un valor muy pequeno no se escuchan las notas de alta frecuencia
#define POLY_DELTA (14400)

void play_two_notes(float freq1, float freq2, unsigned long duration) { //Toca dos notas para simular polifonia
    for (unsigned long t=0; t<duration; t+=2*POLY_DELTA) {
      play_one_note(freq1, POLY_DELTA);
      play_one_note(freq2, POLY_DELTA);
    }
}
int lead_note_count = sizeof(lead_notes) / sizeof(float);
int bass_note_count = sizeof(bass_notes) / sizeof(float);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(Buzzer, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(PB_4, INPUT_PULLUP);
  pinMode(PD_0, INPUT_PULLUP);
  pinMode(PD_1, INPUT_PULLUP);
  pinMode(PD_2, INPUT_PULLUP);
  pinMode(PD_3, INPUT_PULLUP);
  pinMode(PE_1, INPUT_PULLUP);
  pinMode(PE_0, INPUT_PULLUP); //MODO(menu(0)-juego(1))
  digitalWrite(PE_1, LOW);
  digitalWrite(PD_0, LOW);
  digitalWrite(PD_1, LOW);
  digitalWrite(PD_2, LOW);
  digitalWrite(PD_3, LOW);
  inter = 0;
  Serial.begin(9600);
  Serial.print("Hello");
}

void loop() {
  int curr_lead_note = 0;
  int curr_bass_note = 0;
  float curr_lead_note_time_remaining = lead_times[curr_lead_note];
  float curr_bass_note_time_remaining = bass_times[curr_bass_note];
  float lead_freq, bass_freq, note_value;
  unsigned long duration;
  
  while (curr_lead_note < lead_note_count && curr_bass_note < bass_note_count) {

    if(digitalRead(PE_0) == LOW){ //El jugador esta en el menu
      if(digitalRead(PE_1) == HIGH){
        inter = 1;
        EfectoSonido();
      } else if ((digitalRead(PD_2) == HIGH) || (digitalRead(PD_3) == HIGH)){
        inter = 2;
        EfectoSonido();
      }
    }else if(digitalRead(PE_0) == LOW){ //El jugador esta jugando
      inter = 0;
    }
    
    lead_freq = lead_notes[curr_lead_note];
    bass_freq = bass_notes[curr_bass_note];
    note_value = min(curr_lead_note_time_remaining, curr_bass_note_time_remaining);
    duration = note_value * 1000000 * (60.0/BPM);

    if (lead_freq > 0 && bass_freq > 0) {
      play_two_notes(lead_freq, bass_freq, duration);
    } else if (lead_freq > 0) {
      play_one_note(lead_freq, duration);
    } else if (bass_freq > 0) {
      play_one_note(bass_freq, duration);
    } else if (sonido == HIGH) {
      play_one_note(_AS8, 10000);
      sonido = LOW;
    }else {
      delay( duration/1000 );
    }

    // Advance lead note
    curr_lead_note_time_remaining -= note_value;
    if (curr_lead_note_time_remaining < 0.001) {
      curr_lead_note++;
      curr_lead_note_time_remaining = lead_times[curr_lead_note];
    }

    // Advance bass note
    curr_bass_note_time_remaining -= note_value;
    if (curr_bass_note_time_remaining < 0.001) {
      curr_bass_note++;
      curr_bass_note_time_remaining = bass_times[curr_bass_note];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Estas funciones realizan lamisma funcion que el loop pero permiten tocar los efectos especiales del juego y guardan las variables iniciales de la ubicaciones en el float de la cancion principal
//esto permite continuar con la cancion despues del efecto de sonido en el mismo lugar en el que se quedo anteriormente.

void EfectoSonido(){
  //Se guardan los valores originales 
  float  Original_lead_time_r = curr_lead_note_time_remaining;
  float  Original_bass_time_r = curr_bass_note_time_remaining;
  float Original_lead_f = lead_freq;
  float Original_bass_f = bass_freq;
  
  //Se reemplazan los floats con las notas y tiempos
  if(inter == 1){
    for( byte t = 0; t<7; t++){
      lead_notes[t] = Smenu_lead_notes[t];
      lead_times[t] = Smenu_lead_times[t];
      bass_notes[t] = Smenu_bass_notes[t];
      bass_times[t] = Smenu_bass_times[t];
      }
    inter = 0;
  }else if(inter == 2){
    for( byte t = 0; t<7; t++){
      lead_notes[t] = Menu_lead_notes[t];
      lead_times[t] = Menu_lead_times[t];
      bass_notes[t] = Menu_bass_notes[t];
      bass_times[t] = Menu_bass_times[t];
    }
    inter = 0;
  }
  //Se escriben los nuevos valores 
  float curr_lead_note_time_remaining = lead_times[curr_lead_note];
  float curr_bass_note_time_remaining = bass_times[curr_bass_note];
  float lead_freq, bass_freq, note_value;
  unsigned long duration;

  while (curr_lead_note < lead_note_count && curr_bass_note < bass_note_count) {
    lead_freq = lead_notes[curr_lead_note];
    bass_freq = bass_notes[curr_bass_note];
    note_value = min(curr_lead_note_time_remaining, curr_bass_note_time_remaining);
    duration = note_value * 1000000 * (60.0/BPM);

    if (lead_freq > 0 && bass_freq > 0) {
      play_two_notes(lead_freq, bass_freq, duration);
    } else if (lead_freq > 0) {
      play_one_note(lead_freq, duration);
    } else if (bass_freq > 0) {
      play_one_note(bass_freq, duration);
    } else if (sonido == HIGH) {
      play_one_note(_AS8, 10000);
      sonido = LOW;
    }else {
      delay( duration/1000 );
    }

    // Advance lead note
    curr_lead_note_time_remaining -= note_value;
    if (curr_lead_note_time_remaining < 0.001) {
      curr_lead_note++;
      curr_lead_note_time_remaining = lead_times[curr_lead_note];
    }

    // Advance bass note
    curr_bass_note_time_remaining -= note_value;
    if (curr_bass_note_time_remaining < 0.001) {
      curr_bass_note++;
      curr_bass_note_time_remaining = bass_times[curr_bass_note];
    }
  }
  curr_lead_note_time_remaining = Original_lead_time_r;
  curr_bass_note_time_remaining= Original_bass_time_r;
  lead_freq = Original_lead_f ;
  bass_freq = Original_bass_f;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
