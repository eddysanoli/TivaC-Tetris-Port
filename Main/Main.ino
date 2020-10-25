//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 */
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <SD.h>
#include <SPI.h>

File Archivo;

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "lcd_registers.h"
#include "constantes.h"

// LIBRERÍA LCD
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};

// FUNCIONES JUEGO

// Botones y Palancas
#define BTN2 PF_0
#define BTN1 PF_4
#define P1_SW PA_6
#define P1_URX PE_2
#define P1_URY PE_3

#define MENU PC_5
#define DER PC_4
#define IZQ PD_6
#define PRESS PC_7

// Modo de juego
#define SinglePlayer  0
#define MultiPlayer   1

// Estado de Juego
#define Playing 0
#define GameOver 1
#define Success 2

// Tipo de Marco de Área de Juego
#define AreaJuego_Delgado 0
#define AreaJuego_Grueso  1
#define MenuA_Grueso      2
#define MenuA_Delgado     3
#define MenuB_Grueso      4
#define MenuB_Delgado     5
#define Punteado_Amarillo 6
#define Punteado_Verde    7
#define Menu_Amarillo     8

// Tamaños de Tetromino
#define Big 0
#define Small 1

// Tipos de Tetromino
#define TI 0
#define TO 1
#define TL 2
#define TJ 3
#define TS 4
#define TZ 5
#define TT 6

// Estados de tetromino en juego
#define Caida 1
#define Standby 0

// Estados de "obstaculización" en alguna dirección
#define Clear 1
#define Blocked 0

// Esquema de Color para el Bloque
#define Normal 0
#define Alternativo 1

// Tipo de juego
#define A 0
#define B 1

//***************************************************************************************************************************************
// Declaración de Funciones
//***************************************************************************************************************************************

// Funciones de Control LCD
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);

// Funciones Dibujado de Figuras Geométricas
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);

// Funciones Dibujado de Gráficos
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);
void LCD_SpriteCS(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset, char colorswap, int OldColor, int NewColor);
void LCD_BitmapSD(int x, int y, int width, int height, String TXT);

//***************************************************************************************************************************************
// Variables globales
//***************************************************************************************************************************************

int Modo = 0;
int GameAreaX = 0; 
int GameAreaY = 0;
int StatsX = 0;
int StatsY = 0;
byte NoRotaciones = 0;
int HighScore[3] = {0, 0, 0};
String HighName[3];
byte HighLevel[3] = {0, 0, 0};

//***************************************************************************************************************************************
// Setup: Menú y Selección de Modo
//***************************************************************************************************************************************
void setup() {
  
  // INICIALIZACIÓN LCD ---------------------------------------------------------
  // DESCRIPCIÓN: Se coloca la configuración necesaria para poder "interfazar" con la
  // pantalla LCD conectada a la TIVA-C. Esto incluye la utilización del lector de SD

  // Conexión con Pantalla
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.begin(9600);
  randomSeed(analogRead(PE_0));
  Serial.println("TETRIS (NES) TM");
  Serial.println("Eduardo Santizo y Alejandro Windevoxhel");
  Serial.println("------------------------------------------");
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);   
  pinMode(P1_SW, INPUT_PULLUP);
  LCD_Init();
  LCD_Clear(0x00);

  // Conexión con Lector de Tarjetas SD
  SPI.setModule(0);
  pinMode(PA_3, OUTPUT);
  pinMode(IZQ, OUTPUT);
  pinMode(DER, OUTPUT);
  pinMode(PRESS, OUTPUT);
  pinMode(MENU, OUTPUT); 
  if (!SD.begin(4)) Serial.println("Inicialización SD: Inicialización fallida!");
  else Serial.println("Inicialización SD: Inicialización existosa!");
  Serial.println("------------------------------------------");
  Serial.println("");

  // CRÉDITOS INICIALES ---------------------------------------------------------
  // DESCRIPCIÓN: Copia de la pantalla de créditos inicial de Tetris NES

  digitalWrite(MENU, LOW);
  
  int CreditosX = 100, CreditosY = 50;
  
  Write_Text("TM AND ",                   CreditosX, CreditosY, 0xffff, 0x0000);
  Write_Text("c ",                        CreditosX + 7*8, CreditosY, 0x541f, 0x0000);
  Write_Num (1987, 4,                     CreditosX + 9*8, CreditosY, 0x9ff0, 0x0000);
  Write_Text("V/O ELECTRONORGTECHNICA",   CreditosX - 4*8, CreditosY + 2*8, 0xfa00, 0x0000);
  Write_Text("(*ELORG*)",                 CreditosX + 2*8, CreditosY + 4*8, 0xfa00, 0x0000);
  Write_Text("TETRIS",                    CreditosX - 6*8, CreditosY + 6*8, 0x9ff0, 0x0000);
  Write_Text("LICENSED TO",               CreditosX + 1*8, CreditosY + 6*8, 0xffff, 0x0000);
  Write_Text("NINTENDO",                  CreditosX + 13*8, CreditosY + 6*8,0xfa00, 0x0000);
  Write_Text("c",                         CreditosX - 2*8, CreditosY + 8*8, 0x541f, 0x0000); 
  Write_Num (1989, 4,                     CreditosX + 0*8, CreditosY + 8*8, 0x9ff0, 0x0000);
  Write_Text("NINTENDO",                  CreditosX + 5*8, CreditosY + 8*8, 0xfa00, 0x0000);
  Write_Text("ALL RIGHTS RESERVED",       CreditosX - 3*8, CreditosY + 10*8, 0xffff, 0x0000);
  Write_Text("ORIGINAL CONCEPT, DESIGN",  CreditosX - 5*8, CreditosY + 12*8, 0xffff, 0x0000);
  Write_Text("AND PROGRAM",               CreditosX + 2*8, CreditosY + 14*8, 0xffff, 0x0000);
  Write_Text("BY",                        CreditosX - 2*8, CreditosY + 16*8, 0xffff, 0x0000);
  Write_Text("ALEXEY PAZHITNOV",          CreditosX + 1*8, CreditosY + 16*8, 0xfa00, 0x0000);

  // Se espera a que se presione "START" o pase cierto tiempo para que desaparezcan los créditos
  int TiempoCreditos = 0;
  while (digitalRead(BTN1) == HIGH && digitalRead(P1_SW) == HIGH && TiempoCreditos < 3000){
    TiempoCreditos += 1;
    delay(10);
  }
 
  // SETUP DE PANTALLA DE INICIO --------------------------------------------------------------
  // DESCRIPCIÓN: Se grafica el título del juego, el palacio ruso que lo acompaña, así como un
  // un marco de tetrominos grises y todo el texto que acompaña la pantalla principal.

  // Palacio Ruso y Título ======================
  
  LCD_Clear(0x00);
  int TituloX = 60, TituloY = 45;
  LCD_BitmapSD(TituloX, TituloY, 206, 63, "Tetris.txt");
  LCD_BitmapSD(TituloX + 139, TituloY + 76, 62, 61, "Palacio.txt");

  // Push Start, Nintendo, Copyright y 1989 ======================

  int PosTextoX = TituloX + 4*8; 
  int PosTextoY = TituloY + 8*13;
  Write_Text("PUSH START", PosTextoX, PosTextoY, 0xffff, 0x0000);
  Write_Text("c", PosTextoX + 2*8, PosTextoY + 5*8, 0xee84, 0x0000);
  Write_Num(1989, 4, PosTextoX + 4*8, PosTextoY + 5*8, 0xffff, 0x0000);
  LCD_Sprite(PosTextoX + 10*8, PosTextoY + 5*8, 48, 8, Nintendo, 1, 0, 1, 0);

  // Marco Gris de Tetrominos ======================

  // Marco Superior  
  int SaltoSpriteDerecha = 0, SaltoSpriteAbajo = 0;
  for (int Fila = 0; Fila < 4; Fila++){
    for (int Columna = 0; Columna < 40; Columna++){
      LCD_Sprite(SaltoSpriteDerecha, SaltoSpriteAbajo, 8, 8, TM, 20, MarcoSup[Fila][Columna], 1, 0);
      SaltoSpriteDerecha += 8;
    }
    SaltoSpriteAbajo += 8;
    SaltoSpriteDerecha = 0;
  }
  
  // Marco Inferior
  SaltoSpriteDerecha = 24; SaltoSpriteAbajo = 208;
  for (int Fila = 0; Fila < 4; Fila++){
    for (int Columna = 0; Columna < 37; Columna++){
      LCD_Sprite(SaltoSpriteDerecha, SaltoSpriteAbajo, 8, 8, TM, 20, MarcoInf[Fila][Columna], 1, 0);
      SaltoSpriteDerecha += 8;
    }
    SaltoSpriteAbajo += 8;
    SaltoSpriteDerecha = 24;
  }

  // Marco Izquierdo
  SaltoSpriteDerecha = 0; SaltoSpriteAbajo = 32;
  for (int Fila = 0; Fila < 26; Fila++){
    for (int Columna = 0; Columna < 3; Columna++){
      LCD_Sprite(SaltoSpriteDerecha, SaltoSpriteAbajo, 8, 8, TM, 20, MarcoIzq[Fila][Columna], 1, 0);
      SaltoSpriteDerecha += 8;
    }
    SaltoSpriteAbajo += 8;
    SaltoSpriteDerecha = 0;
  }

  // Marco Derecho  
  SaltoSpriteDerecha = 288; SaltoSpriteAbajo = 32;
  for (int Fila = 0; Fila < 22; Fila++){
    for (int Columna = 0; Columna < 4; Columna++){
      LCD_Sprite(SaltoSpriteDerecha, SaltoSpriteAbajo, 8, 8, TM, 20, MarcoDer[Fila][Columna], 1, 0);
      SaltoSpriteDerecha += 8;
    }
    SaltoSpriteAbajo += 8;
    SaltoSpriteDerecha = 288;
  }  

  // INTERACCIÓN CON PANTALLA DE INICIO --------------------------------------------------------------
  // DESCRIPCIÓN: Cada bloque comentado se ejecuta de manera secuencial conforme se van completando
  // las diferentes condiciones dadas. Menú en el que se selecciona si jugarán 1 o 2 jugadores.

  // Se espera a que se presione "START"
  while (digitalRead(BTN1) == HIGH && digitalRead(P1_SW) == HIGH);

  // "PUSH START" parpadea para confirmar la selección
  for (int i = 0; i < 8; i++){
    Write_Text("PUSH START", PosTextoX, PosTextoY, 0xffff, 0x0000);
    delay(25);
    Write_Text("PUSH START", PosTextoX, PosTextoY, 0x0000, 0x0000);
    delay(25);
  }

  // Modos de juego: "1P" y "2P"
  Write_Num (1, 1, PosTextoX, PosTextoY - 8, 0xffff, 0x0000);
  Write_Text("PLAYER", PosTextoX + 2*8, PosTextoY - 8, 0xffff, 0x0000);
  Write_Num (2, 1, PosTextoX, PosTextoY + 8, 0xffff, 0x0000);
  Write_Text("PLAYERS", PosTextoX + 2*8, PosTextoY + 8, 0xffff, 0x0000);
  Write_Text(">", PosTextoX - 2*8, PosTextoY - 8, 0xffff, 0x0000);

  // Se espera a la selección del modo de juego
  char Modo = SinglePlayer;
  while (digitalRead(BTN1) == HIGH && digitalRead(P1_SW) == HIGH){

    // Se alterna entre las opciones disponibles presionando un solo botón
    if (digitalRead(BTN2) == LOW || analogRead(P1_URY) > 4000 || analogRead(P1_URY) < 100){      
      Write_Text(">", PosTextoX - 8*2, PosTextoY - 8 + (16*Modo), 0x0000, 0x0000);
      Modo = (Modo + 1) % 2;
      Write_Text(">", PosTextoX - 8*2, PosTextoY - 8 + (16*Modo), 0xffff, 0x0000);
      delay(300);
    }
  }

  // Se hace parpadear la "Flecha de selección" para confirmar el modo elegido
  for (int i = 0; i < 10; i++){
    Write_Text(">", PosTextoX - 8*2, PosTextoY - 8 + (16*Modo), 0xffff, 0x0000);
    delay(25);
    Write_Text(">", PosTextoX - 8*2, PosTextoY - 8 + (16*Modo), 0x0000, 0x0000);
    delay(25);
  }

}

//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  
  // MODO "1 JUGADOR" ---------------------------------------------------------
  // DESCRIPCIÓN: En este modo se permite al jugador acceder a la experiencia tradicional de
  // Tetris (NES). Esto incluye la selección de modos A y B, así como la selección de música
  // y un scoreboard con los punteos más altos.
  
  if (Modo == SinglePlayer){

    // DECLARACIÓN DE VARIABLES A UTILIZAR

    int Lineas = 0;                                                                       // Indica: El número de líneas limpiadas en la "Game Area" 
    int DeltaLineas = 0;                                                                  // Indica: Número de líneas incrementadas al limpiar líneas
    int Score = 0;                                                                        // Indica: Punteo del jugador          
    byte Altura = 0;                                                                      // Indica: Altura de la "basura" que aparecerá al inicio del tipo de juego B.                                                           
    byte SoftDrop = 0;                                                                    // Indica: Número de bloques que el jugador ha dejado caer "duro" el tetromino hacia abajo
    byte PosX = 0; byte PosY = 0;                                                         // Indica: Posición inicial (En Matriz "Game Area") de tetromino activo
    byte TipoTetromino = random(0,6);                                                     // Indica: Tipo de tetromino a colocar en el área de juego
    byte EstadoTetromino;                                                                 // Indica: Si el jugador está controlando la caida de un tetromino (Caida) o si se esperará a actualizar datos (Standby)
    byte NoFilas;                                                                         // Indica: No. de filas de la matriz de bloques del tetromino actual. La mayoría tienen un tamaño de 3x3
    byte EstadoJuego = Playing;                                                           // Indica: Si el juego está en proceso o ha finalizado
    
    byte Nivel = 0;                                                                       // Indica: Nivel actual. Incrementa cada 10 líneas limpiadas
    byte NivelBase = 0;                                                                   // Indica: Valor seleccionado por el jugador a antes de iniciar el juego.
    
    int TetrominosColocados[7] = {0,0,0,0,0,0,0};                                         // Indica: No. de tetrominos colocados de cada tipo


    // PANTALLA "MENU SELECCIÓN MÚSICA Y TIPO DE JUEGO"
    LCD_Clear(0x00);

    int GameMenuX = 56; int GameMenuY = 24;                                               // Punto de referencia: Esquina superior izquierda del marco que dice "Game Type"
    Crear_Marco(GameMenuX, GameMenuY, 14, 3, Menu_Amarillo);
    Write_Text(" GAME  TYPE ", GameMenuX + 8, GameMenuY + 8, 0xffff, 0x0000);

    Crear_Marco(GameMenuX + 4*8, GameMenuY + 4*8, 10, 3, MenuA_Delgado);
    Write_Text(" A-TYPE ", GameMenuX + 5*8, GameMenuY + 5*8, 0xffff, 0x0000);
    Crear_Marco(GameMenuX + 16*8, GameMenuY + 4*8, 10, 3, MenuB_Delgado);
    Write_Text(" B-TYPE ", GameMenuX + 17*8, GameMenuY + 5*8, 0xffff, 0x0000);

    Crear_Marco(GameMenuX, GameMenuY + 9*8, 14, 3, Menu_Amarillo);
    Write_Text(" MUSIC TYPE ", GameMenuX + 8, GameMenuY + 10*8, 0xffff, 0x0000);
    Crear_Marco(GameMenuX + 9*8, GameMenuY + 13*8, 12, 11, Punteado_Amarillo);
    Write_Text("MUSIC][", GameMenuX + 11*8, GameMenuY + 15*8, 0xffff, 0x0000);
    Write_Num(1, 1, GameMenuX + 18*8, GameMenuY + 15*8, 0xffff, 0x0000);
    Write_Text("MUSIC][", GameMenuX + 11*8, GameMenuY + 17*8, 0xffff, 0x0000);
    Write_Num(2, 1, GameMenuX + 18*8, GameMenuY + 17*8, 0xffff, 0x0000);
    Write_Text("MUSIC][", GameMenuX + 11*8, GameMenuY + 19*8, 0xffff, 0x0000);
    Write_Num(3, 1, GameMenuX + 18*8, GameMenuY + 19*8, 0xffff, 0x0000);
    Write_Text("  OFF ", GameMenuX + 11*8, GameMenuY + 21*8, 0xffff, 0x0000);
    
    byte TipoJuego = A;                                                                   // El cursor de selección inicia en la opción para el tipo de juego A.
    byte TipoMusica = 1;                                                                  // El cursor de selección inicia en la opción para la canción 1
    while(digitalRead(P1_SW) == HIGH){
      Write_Text("}", GameMenuX + 5*8 + TipoJuego*12*8, GameMenuY + 5*8, 0xffff, 0x0000);
      Write_Text("{", GameMenuX + 12*8 + TipoJuego*12*8, GameMenuY + 5*8, 0xffff, 0x0000);
      Write_Text("}", GameMenuX + 10*8, GameMenuY + 15*8 + (TipoMusica - 1)*2*8, 0xffff, 0x0000);
      Write_Text("{", GameMenuX + 19*8, GameMenuY + 15*8 + (TipoMusica - 1)*2*8, 0xffff, 0x0000);
      delay(25);
      Write_Text("}", GameMenuX + 5*8 + TipoJuego*12*8, GameMenuY + 5*8, 0x0000, 0x0000);
      Write_Text("{", GameMenuX + 12*8 + TipoJuego*12*8, GameMenuY + 5*8, 0x0000, 0x0000);
      Write_Text("}", GameMenuX + 10*8, GameMenuY + 15*8 + (TipoMusica - 1)*2*8, 0x0000, 0x0000);
      Write_Text("{", GameMenuX + 19*8, GameMenuY + 15*8 + (TipoMusica - 1)*2*8, 0x0000, 0x0000);
      delay(25);

      // Movimiento lateral de palanca
      if(TipoJuego == A && analogRead(P1_URX) > 4000){                                    // Si se está en la opción A y se mueve la palanca a la derecha, se cambia al tipo B
        TipoJuego = B;
        
      }
      else if (TipoJuego == B && analogRead(P1_URX) < 300){                               // Si se está en la opción B y se mueve la palanca a la izquierda, se cambia al tipo A
        TipoJuego = A;
      }

      // Movimiento vertical de palanca
      if(TipoMusica != 4 && analogRead(P1_URY) > 4000){                                   // Si la opción es distinta a 4 y se mueve la palanca hacia abajo, se cambia de tipo de canción (Hacia abajo)
        TipoMusica ++;
        delay(80);
      }
      else if(TipoMusica != 1 && analogRead(P1_URY) < 100){                               // Si la opción es distinta de 1 y se mueve la palanca hacia arriba, se cambia de tipo de canción (Hacia arriba)
        TipoMusica --;
        delay(80);
      }
      
    }

    // PANTALLA "GAME TYPE A"
    LCD_Clear(0x00);
    
    int GameTypeX = 56; int GameTypeY = 24;                                               // Punto de referencia: Esquina superior izquierda de marco rojo
    if (TipoJuego == A){
      
      Crear_Marco(GameTypeX, GameTypeY, 26, 25, MenuA_Delgado);                           // Marco externo y título (TYPE A)
      Crear_Marco(GameTypeX + 8*8, GameTypeY - 8, 10, 3, MenuA_Delgado);
      Write_Text(" A-TYPE ", GameTypeX + 9*8, GameTypeY, 0xffff, 0x0000);

      Crear_Marco(GameTypeX + 5*8, GameTypeY + 4*8, 7, 3, MenuA_Grueso);                  // Título LEVEL
      Write_Text("LEVEL", GameTypeX + 6*8, GameTypeY + 5*8, 0xffff, 0x0000);

      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 11, 3, Punteado_Verde);               // Marco verde de niveles
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 3, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 5, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 7, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 9, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 11, 5, Punteado_Verde); 
      Write_Num(0, 1, GameTypeX + 4*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(1, 1, GameTypeX + 6*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(2, 1, GameTypeX + 8*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(3, 1, GameTypeX + 10*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(4, 1, GameTypeX + 12*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(5, 1, GameTypeX + 4*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(6, 1, GameTypeX + 6*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(7, 1, GameTypeX + 8*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(8, 1, GameTypeX + 10*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(9, 1, GameTypeX + 12*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);

      Read_HighScoreSD(HighScore, HighName, HighLevel, "AScore.txt");                     // Se obtienen los valores de High Score para el tipo de juego A.
      Crear_Marco(GameTypeX + 2*8, GameTypeY + 14*8, 22, 3, Punteado_Amarillo);
      Crear_Marco(GameTypeX + 2*8, GameTypeY + 14*8, 22, 9, Punteado_Amarillo);
      Write_Text("NAME  SCORE  LV", GameTypeX + 7*8, GameTypeY + 15*8, 0xffff, 0x0000);
      
      Write_Num(1, 1, GameTypeX + 4*8, GameTypeY + 17*8, 0xffff, 0x0000);                 // High Score: Primer Lugar
      Write_Text(HighName[0], GameTypeX + 6*8, GameTypeY + 17*8, 0xffff, 0x0000);          
      Write_Num(HighScore[0], 6, GameTypeX + 13*8, GameTypeY + 17*8, 0xffff, 0x0000);
      Write_Num(HighLevel[0], 2, GameTypeX + 20*8, GameTypeY + 17*8, 0xffff, 0x0000);
      
      Write_Num(2, 1, GameTypeX + 4*8, GameTypeY + 19*8, 0xffff, 0x0000);                 // High Score: Segundo Lugar
      Write_Text(HighName[1], GameTypeX + 6*8, GameTypeY + 19*8, 0xffff, 0x0000);          
      Write_Num(HighScore[1], 6, GameTypeX + 13*8, GameTypeY + 19*8, 0xffff, 0x0000);
      Write_Num(HighLevel[1], 2, GameTypeX + 20*8, GameTypeY + 19*8, 0xffff, 0x0000);
      
      Write_Num(3, 1, GameTypeX + 4*8, GameTypeY + 21*8, 0xffff, 0x0000);                 // High Score: Tercer Lugar
      Write_Text(HighName[2], GameTypeX + 6*8, GameTypeY + 21*8, 0xffff, 0x0000);          
      Write_Num(HighScore[2], 6, GameTypeX + 13*8, GameTypeY + 21*8, 0xffff, 0x0000);
      Write_Num(HighLevel[2], 2, GameTypeX + 20*8, GameTypeY + 21*8, 0xffff, 0x0000);

      byte FilaVertical = 0;                                                              // Variable que indica si se está en la fila superior o inferior de la cuadrícula verde
      while(digitalRead(P1_SW) == HIGH){                                                  // Selección del nivel deseado
        if(NivelBase > 4) FilaVertical = 1;
        else FilaVertical = 0;
        
        Write_Num(NivelBase, 1, GameTypeX + 4*8 + 2 + 2*8*(NivelBase % 5), GameTypeY + 8*8 + 2*8*FilaVertical, 0xfa60, 0xfd00);
        delay(25);
        Write_Num(NivelBase, 1, GameTypeX + 4*8 + 2 + 2*8*(NivelBase % 5), GameTypeY + 8*8 + 2*8*FilaVertical, 0xfa60, 0x0000);
        delay(25);

        // Movimiento horizontal de palanca
        if((NivelBase % 5) != 4 && analogRead(P1_URX) > 3000) {                           // Si aún no se está en el tope derecho y se mueve la palanca a la derecha 
          NivelBase ++;
          delay(80);
        }
        else if((NivelBase % 5) != 0 && analogRead(P1_URX) < 300){                        // Si aún no se está en el tope izquierdo y se mueve la palanca a la izquierda
          NivelBase --;
          delay(80);
        }

        // Movimiento vertical de palanca
        if(FilaVertical == 0 && analogRead(P1_URY) > 4000){                               // Si se está en la fila superior y se mueve la palanca hacia abajo
          NivelBase += 5;
          delay(80);
        }
        else if(FilaVertical == 1 && analogRead(P1_URY) < 100){                           // Si se está en la fila inferior y se mueve la palanca hacia arriba
          NivelBase -= 5;
          delay(80);
        }
      }

      Lineas = 0;                                                                         // El jugador inicia con 0 líneas limpiadas.
    }

    // PANTALLA "GAME TYPE B"
    if (TipoJuego == B){

      Crear_Marco(GameTypeX, GameTypeY, 26, 25, MenuB_Delgado);                           // Marco externo y título (TYPE B)
      Crear_Marco(GameTypeX + 8*8, GameTypeY - 8, 10, 3, MenuB_Delgado);
      Write_Text(" B-TYPE ", GameMenuX + 9*8, GameTypeY, 0xffff, 0x0000);

      Crear_Marco(GameTypeX + 5*8, GameTypeY + 4*8, 7, 3, MenuB_Grueso);
      Write_Text("LEVEL", GameTypeX + 6*8, GameTypeY + 5*8, 0xffff, 0x0000);

      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 11, 3, Punteado_Verde);               // Marco verde de niveles
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 3, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 5, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 7, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 9, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 3*8, GameTypeY + 7*8, 11, 5, Punteado_Verde); 
      Write_Num(0, 1, GameTypeX + 4*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(1, 1, GameTypeX + 6*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(2, 1, GameTypeX + 8*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(3, 1, GameTypeX + 10*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(4, 1, GameTypeX + 12*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(5, 1, GameTypeX + 4*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(6, 1, GameTypeX + 6*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(7, 1, GameTypeX + 8*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(8, 1, GameTypeX + 10*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(9, 1, GameTypeX + 12*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);

      Crear_Marco(GameTypeX + 16*8, GameTypeY + 4*8, 8, 3, MenuB_Grueso);
      Write_Text("HEIGHT", GameTypeX + 17*8, GameTypeY + 5*8, 0xffff, 0x0000);

      Crear_Marco(GameTypeX + 16*8, GameTypeY + 7*8, 7, 3, Punteado_Verde);               // Marco verde de alturas (Por cada "altura" se suman 3 filas de bloques basura al tablero).
      Crear_Marco(GameTypeX + 16*8, GameTypeY + 7*8, 3, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 16*8, GameTypeY + 7*8, 5, 5, Punteado_Verde); 
      Crear_Marco(GameTypeX + 16*8, GameTypeY + 7*8, 7, 5, Punteado_Verde); 
      Write_Num(0, 1, GameTypeX + 17*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(1, 1, GameTypeX + 19*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(2, 1, GameTypeX + 21*8 + 2, GameTypeY + 8*8, 0xfa60, 0x0000);
      Write_Num(3, 1, GameTypeX + 17*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(4, 1, GameTypeX + 19*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);
      Write_Num(5, 1, GameTypeX + 21*8 + 2, GameTypeY + 10*8, 0xfa60, 0x0000);

      Read_HighScoreSD(HighScore, HighName, HighLevel, "BScore.txt");                     // Se obtienen los valores de High Score para el tipo de juego A.
      Crear_Marco(GameTypeX + 2*8, GameTypeY + 14*8, 22, 3, Punteado_Amarillo);
      Crear_Marco(GameTypeX + 2*8, GameTypeY + 14*8, 22, 9, Punteado_Amarillo);
      Write_Text("NAME  SCORE  LV", GameTypeX + 7*8, GameTypeY + 15*8, 0xffff, 0x0000);
      
      Write_Num(1, 1, GameTypeX + 4*8, GameTypeY + 17*8, 0xffff, 0x0000);                 // High Score: Primer Lugar
      Write_Text(HighName[0], GameTypeX + 6*8, GameTypeY + 17*8, 0xffff, 0x0000);          
      Write_Num(HighScore[0], 6, GameTypeX + 13*8, GameTypeY + 17*8, 0xffff, 0x0000);
      Write_Num(HighLevel[0], 2, GameTypeX + 20*8, GameTypeY + 17*8, 0xffff, 0x0000);
      
      Write_Num(2, 1, GameTypeX + 4*8, GameTypeY + 19*8, 0xffff, 0x0000);                 // High Score: Segundo Lugar
      Write_Text(HighName[1], GameTypeX + 6*8, GameTypeY + 19*8, 0xffff, 0x0000);          
      Write_Num(HighScore[1], 6, GameTypeX + 13*8, GameTypeY + 19*8, 0xffff, 0x0000);
      Write_Num(HighLevel[1], 2, GameTypeX + 20*8, GameTypeY + 19*8, 0xffff, 0x0000);
      
      Write_Num(3, 1, GameTypeX + 4*8, GameTypeY + 21*8, 0xffff, 0x0000);                 // High Score: Tercer Lugar
      Write_Text(HighName[2], GameTypeX + 6*8, GameTypeY + 21*8, 0xffff, 0x0000);          
      Write_Num(HighScore[2], 6, GameTypeX + 13*8, GameTypeY + 21*8, 0xffff, 0x0000);
      Write_Num(HighLevel[2], 2, GameTypeX + 20*8, GameTypeY + 21*8, 0xffff, 0x0000);
      
      byte FilaVertical = 0;                                                              // Variable que indica si se está en la fila superior o inferior de la cuadrícula verde
      while(digitalRead(P1_SW) == HIGH){                                                  // Selección del nivel deseado
        if(NivelBase > 4) FilaVertical = 1;
        else FilaVertical = 0;
        
        Write_Num(NivelBase, 1, GameTypeX + 4*8 + 2 + 2*8*(NivelBase % 5), GameTypeY + 8*8 + 2*8*FilaVertical, 0xfa60, 0xfd00);
        delay(25);
        Write_Num(NivelBase, 1, GameTypeX + 4*8 + 2 + 2*8*(NivelBase % 5), GameTypeY + 8*8 + 2*8*FilaVertical, 0xfa60, 0x0000);
        delay(25);

        // Movimiento horizontal de palanca
        if((NivelBase % 5) != 4 && analogRead(P1_URX) > 3000) {                           // Si aún no se está en el tope derecho y se mueve la palanca a la derecha 
          NivelBase ++;
          delay(80);
        }
        else if((NivelBase % 5) != 0 && analogRead(P1_URX) < 300){                        // Si aún no se está en el tope izquierdo y se mueve la palanca a la izquierda
          NivelBase --;
          delay(80);
        }

        // Movimiento vertical de palanca
        if(FilaVertical == 0 && analogRead(P1_URY) > 4000){                               // Si se está en la fila superior y se mueve la palanca hacia abajo
          NivelBase += 5;
          delay(80);
        }
        else if(FilaVertical == 1 && analogRead(P1_URY) < 100){                           // Si se está en la fila inferior y se mueve la palanca hacia arriba
          NivelBase -= 5;
          delay(80);
        }
      }

      Altura = 0;
      delay(200);
      while(digitalRead(P1_SW) == HIGH){                                                  // Selección de la altura deseada
        if(Altura > 2) FilaVertical = 1;
        else FilaVertical = 0;
        
        Write_Num(Altura, 1, GameTypeX + 17*8 + 2 + 2*8*(Altura % 3), GameTypeY + 8*8 + 2*8*FilaVertical, 0xfa60, 0xfd00);
        delay(25);
        Write_Num(Altura, 1, GameTypeX + 17*8 + 2 + 2*8*(Altura % 3), GameTypeY + 8*8 + 2*8*FilaVertical, 0xfa60, 0x0000);
        delay(25);

        // Movimiento horizontal de palanca
        if((Altura % 3) != 2 && analogRead(P1_URX) > 3000) {                              // Si aún no se está en el tope derecho y se mueve la palanca a la derecha 
          Altura ++;
          delay(80);
        }
        else if((Altura % 3) != 0 && analogRead(P1_URX) < 300){                           // Si aún no se está en el tope izquierdo y se mueve la palanca a la izquierda
          Altura --;
          delay(80);
        }

        // Movimiento vertical de palanca
        if(FilaVertical == 0 && analogRead(P1_URY) > 4000){                               // Si se está en la fila superior y se mueve la palanca hacia abajo
          Altura += 3;
          delay(80);
        }
        else if(FilaVertical == 1 && analogRead(P1_URY) < 100){                           // Si se está en la fila inferior y se mueve la palanca hacia arriba
          Altura -= 3;
          delay(80);
        }
      }

      Lineas = 25;                                                                        // El jugador debe limpiar 25 líneas para poder ganar
    }
    
    // PANTALLA "GAME AREA"
    // JUEGO: SETUP ==================================================
    
    Clear_Matrix(P1_Buffer);                                                              // Se limpian todos los bloques del buffer para que en iteraciones posteriores no se aparezca desde el inicio con los bloques del juego pasado
    Clear_Matrix(P1_GameArea);                                                            // Se limpian todos los bloques de la game area con el mismo efecto.

    if(TipoJuego == B) Create_Garbage(Altura, P1_GameArea, P1_Buffer);                    // Si el tipo de juego es "B" entonces se llena el tablero con 3 filas de bloques "basura" por altura escogida
    
    NoFilas = Get_TetrominoData(TipoTetromino, TetrominoActivo);                          // Obtiene el No. de filas de matriz de bloques de tetromino elegido / Se actualizan los valores de "TetrominoActivo"
    switch(NoFilas){                                                                      // Posiciones iniciales de caida (En matriz "Game Area") del "Tetromino Activo"
      case 2:  PosX = 5; PosY = 2; break;
      case 3:  PosX = 5; PosY = 1; break;
      case 4:  PosX = 4; PosY = 0; break;
      default: PosX = 5; PosY = 1; break;
    }

    // JUEGO: GRAFICACIÓN DE MARCOS Y CONTADORES ===============================
    
    LCD_Clear(0x00);
    GameAreaX = 113; GameAreaY = 49;                                                      // Coordenadas del marco que delimita la "GAME AREA" de un jugador    

    Crear_Marco(GameAreaX, GameAreaY, 12, 22, AreaJuego_Grueso);                          // Marco: "GAME AREA"
    
    Crear_Marco(GameAreaX, GameAreaY - 8*3, 12, 3, AreaJuego_Delgado);                    // Marco: Contador de número de líneas limpiadas
    Write_Text("LINES-", GameAreaX + 2*8, GameAreaY - 2*8, 0xffff, 0x0000);

    StatsX = GameAreaX - 8*12;   StatsY = GameAreaY + 8*4;
    Crear_Marco(StatsX, StatsY, 10, 18, AreaJuego_Delgado);                               // Marco: "STATISTICS": Contador del número y tipo de piezas colocadas en el tablero
    LCD_Sprite(StatsX + 8, StatsY + 8, 64, 8, Statistics, 1, 0, 1, 0);                    // Se coloca un sprite en lugar de texto ya que emplea un tamaño irregular no encontrado en las otras letras del juego
    Update_Stats(StatsX, StatsY, 0, TetrominosColocados); 

    Crear_Marco(GameAreaX - 8*11, GameAreaY - 8*2, 8, 3, AreaJuego_Delgado);              // Marco: Tipo de juego "A" o "B"
    if (TipoJuego == A) Write_Text("A-TYPE", GameAreaX - 8*10, GameAreaY - 8, 0xffff, 0x0000);
    if (TipoJuego == B) Write_Text("B-TYPE", GameAreaX - 8*10, GameAreaY - 8, 0xffff, 0x0000);
    
    Crear_Marco(GameAreaX + 8*14, GameAreaY - 8*3, 8, 9, AreaJuego_Delgado);              // Marco: "TOP" y "SCORE"
    Write_Text("TOP", GameAreaX + 8*15, GameAreaY - 8, 0xffff, 0x0000);
    Write_Num (HighScore[0], 6, GameAreaX + 8*15, GameAreaY, 0xffff, 0x0000);             // Se imprime el punteo más alto obtenido en el tipo de juego seleccionado
    Write_Text("SCORE", GameAreaX + 8*15, GameAreaY + 8*2, 0xffff, 0x0000);
    Write_Num (Score, 6, GameAreaX + 8*15, GameAreaY + 8*3, 0xffff, 0x0000); 
    
    Crear_Marco(GameAreaX + 8*15, GameAreaY + 8*7, 6, 7, AreaJuego_Grueso);               // Marco: "Preview" de la siguiente pieza que utilizará el jugador
    Write_Text("NEXT", GameAreaX + 8*16, GameAreaY + 8*8, 0xffff, 0x0000);
    
    Crear_Marco(GameAreaX + 8*14, GameAreaY + 8*15, 7, 4, AreaJuego_Delgado);             // Marco: Nivel en el que se encuentra el jugador (Incrementa luego de limpiar 10 líneas)
    Write_Text("LEVEL", GameAreaX + 8*15, GameAreaY + 8*16, 0xffff, 0x0000);

    if (TipoJuego == B){
      Crear_Marco(GameAreaX + 8*14, GameAreaY + 8*19, 8, 4, AreaJuego_Delgado);           // Marco: Altura elegida por el jugador. Solo graficado cuando el modo de juego es de tipo "B"
      Write_Text("HEIGHT", GameAreaX + 8*15, GameAreaY + 8*20, 0xffff, 0x0000);
      Write_Num (Altura, 1, GameAreaX + 8*18, GameAreaY + 8*21, 0xffff, 0x0000);
    }

    // JUEGO: EJECUCIÓN ===============================================
    
    while (EstadoJuego == Playing){
      
      // Configuración de Tetromino
      EstadoTetromino = Caida;                                                            // Se indica que el tetromino comenzará a caer apenas se ingrese al ciclo while
      TetrominosColocados[TipoTetromino] += 1;                                            // Se suma 1 al contador del tipo de tetromino elegido. Aún no se actualiza su representación gráfica
      TipoTetromino = random(0,6);                                                        // Se genera el valor del siguiente tetromino. Esto debe hacerse después de usar Get_TetrominoData


      // Actualización de Nivel
      if (TipoJuego == A) Nivel = NivelBase + (Lineas - (Lineas % 10)) / 10;              // TIPO A: Nivel será igual al nivel base más el valor de "decenas" del contador de líneas limpiadas
      if (TipoJuego == B) Nivel = NivelBase;                                              // TIPO B: Nivel siempre será igual al nivel base

      // Preview y Statistics
      Write_Num(Nivel, 2, 113 + 8*17, 49 + 8*17, 0xffff, 0x0000);                         // UPDATE: Nivel acorde al número de líneas limpiadas
      Update_Stats(StatsX, StatsY, Nivel, TetrominosColocados);                           // UPDATE: Colores tetrominos pequeños y valores de contadores bajo "Statistics"       
      Update_Preview(GameAreaX + 8*16, GameAreaY + 8*9, TipoTetromino, Nivel);            // UPDATE: Preview para la siguiente pieza. Nota: Debe ir directamente abajo de la generación aleatoria de "Tipo Tetromino"  
      Write_Num(Lineas, 3, GameAreaX + 8*8, GameAreaY - 2*8, 0xffff, 0x0000);             // UPDATE: Contador de líneas o filas limpiadas
      
      // Caida de Tetromino en "Game Area"
      while (EstadoTetromino == Caida){

        // Botón Joystick: Rotar Pieza
        if (digitalRead(P1_SW) == LOW){
          //Serial.println("BOTON");
          if(CheckRotation(P1_GameArea, PosX, PosY, NoFilas, TetrominoActivo) == Clear) Rotate_TetrominoData(TetrominoActivo, NoFilas); 
        }

        // Joystick Izquierda: Mover Pieza a la Izquierda
        else if (analogRead(P1_URX) < 300){
          //Serial.println("IZQUIERDA");
          if(CheckLeft(P1_GameArea, PosX, PosY, NoFilas, TetrominoActivo) == Clear) PosX--;
        }

        // Joystick Derecha: Mover Pieza a la Derecha
        else if (analogRead(P1_URX) > 3000){
          //Serial.println("DERECHA");
          if(CheckRight(P1_GameArea, PosX, PosY, NoFilas, TetrominoActivo) == Clear) PosX++;
        }

        // Joystick Abajo: "Acelerar" la pieza hacia abajo
        else if (analogRead(P1_URY) > 3000){
          //Serial.println("IZQUIERDA");
          if (CheckDown(P1_GameArea, PosX, PosY, NoFilas, TetrominoActivo) == Clear) PosY ++;
          SoftDrop += 1;
        }

        // Botón Izquierdo TIVA-C: Pausa
        if (digitalRead(BTN1) == LOW){
          delay(400);
          while (digitalRead(BTN1) == HIGH);
        }

        if (CheckDown(P1_GameArea, PosX, PosY, NoFilas, TetrominoActivo) == Clear) PosY ++;
        if (CheckDown(P1_GameArea, PosX, PosY, NoFilas, TetrominoActivo) == Blocked) EstadoTetromino = Standby;
        Update_Buffer(GameAreaX, GameAreaY - 8, P1_GameArea, P1_Buffer, PosX, PosY, NoFilas, TetrominoActivo, Nivel);

      }

      // Game Area
      Update_GameArea(P1_GameArea, P1_Buffer);                                                  // UPDATE: La matriz de bloques para la "Game Area" utilizando los datos del "Buffer" 

      // No de líneas completadas
      DeltaLineas = Update_ClearLines(GameAreaX, GameAreaY, P1_GameArea, P1_Buffer, Nivel);     // UPDATE: Actualizar la "Game Area" y "Buffer" si se han completado filas

      if(TipoJuego == A) Lineas += DeltaLineas;                                                 // Si Tipo Juego = A entonces cada línea limpiada se suma al contador de líneas
      if(TipoJuego == B) Lineas -= DeltaLineas;                                                 // Si Tipo juego = B entonces cada línea limpiada se descuenta del contador de líneas 

      // Score
      Score += Get_ScoreIncrement(DeltaLineas, Nivel, SoftDrop);                                // UPDATE: Score. Se suman puntos si hubo un "Soft Drop" o si se limpiaron líneas
      Write_Num (Score, 6, GameAreaX + 8*15, GameAreaY + 8*3, 0xffff, 0x0000);                  // UPDATE: Contador visual del Score
      SoftDrop = 0;                                                                             // Se reinicia el valor de SoftDrop  
      
      NoFilas = Get_TetrominoData(TipoTetromino, TetrominoActivo);                              // GET: Se obtiene la matriz de bloques para el siguiente tetromino activo.
      switch(NoFilas){                                                                          // GET: Se decide la posición inicial del tetromino en función del número de filas de su matriz de bloques
        case 2:  PosX = 5; PosY = 2; break;
        case 3:  PosX = 5; PosY = 1; break;
        case 4:  PosX = 4; PosY = 0; break;
        default: PosX = 5; PosY = 1; break;
      }

      // Si antes de que el nuevo tetromino se coloque, se sabe que
      // no existirá suficiente espacio para el mismo, se establece
      // que se ha llegado a un GameOver
      if (CheckDown(P1_GameArea, PosX, PosY, NoFilas, TetrominoActivo) == Blocked) EstadoJuego = GameOver; 
      if (TipoJuego == B && Lineas == 0) EstadoJuego = Success;
      
    }

    // JUEGO: GAME OVER  ================================

    if (EstadoJuego == GameOver){
      GameOver_Screen(GameAreaX, GameAreaY, Nivel, Palette);
    }

    // JUEGO: SUCCESS ==================================

    if (EstadoJuego == Success){
      Crear_Marco(GameAreaX + 8, GameAreaY + 8*8, 10, 3, AreaJuego_Delgado);
      Write_Text("SUCCESS!", GameAreaX + 2*8, GameAreaY + 9*8, 0xffff, 0x0000);
    }
    
    while(digitalRead(P1_SW) == HIGH);

    // JUEGO: UPDATE HIGH SCORE ==========================
    if (Score > HighScore[2]){
      LCD_Clear(0x00);
      
      if (TipoJuego == A){
        Crear_Marco(GameTypeX, GameTypeY, 26, 25, MenuA_Delgado);                           // Marco externo y título (TYPE A)
        Crear_Marco(GameTypeX + 8*8, GameTypeY - 8, 10, 3, MenuA_Delgado);
        Write_Text(" A-TYPE ", GameMenuX + 9*8, GameTypeY, 0xffff, 0x0000); 
      }
      
      if (TipoJuego == B){
        Crear_Marco(GameTypeX, GameTypeY, 26, 25, MenuB_Delgado);                           // Marco externo y título (TYPE B)
        Crear_Marco(GameTypeX + 8*8, GameTypeY - 8, 10, 3, MenuB_Delgado);
        Write_Text(" B-TYPE ", GameMenuX + 9*8, GameTypeY, 0xffff, 0x0000); 
      }
      
      Crear_Marco(GameTypeX + 2*8, GameTypeY + 14*8, 22, 3, Punteado_Amarillo);
      Crear_Marco(GameTypeX + 2*8, GameTypeY + 14*8, 22, 9, Punteado_Amarillo);
      Write_Text("NAME  SCORE  LV", GameTypeX + 7*8, GameTypeY + 15*8, 0xffff, 0x0000);
      Write_Num(1, 1, GameTypeX + 4*8, GameTypeY + 17*8, 0xffff, 0x0000);    
      Write_Num(2, 1, GameTypeX + 4*8, GameTypeY + 19*8, 0xffff, 0x0000);  
      Write_Num(3, 1, GameTypeX + 4*8, GameTypeY + 21*8, 0xffff, 0x0000); 

      Write_Text("CONGRATULATIONS", GameTypeX + 6*8, GameTypeY + 4*8, 0xfa60, 0x0000);
      Write_Text("YOU ARE A", GameTypeX + 8*8, GameTypeY + 7*8, 0xffff, 0x0000);
      Write_Text("TETRIS MASTER", GameTypeX + 6*8, GameTypeY + 9*8, 0xffff, 0x0000);
      Write_Text("PLEASE ENTER YOUR NAME", GameTypeX + 2*8, GameTypeY + 12*8, 0xffff, 0x0000);

      byte Lugar = 0;                                                                       // Valor del "lugar" en el que quedó el jugador (Primero, segundo o tercero)
      if(Score > HighScore[0]){                                                             // Si el Score supera el score más alto del scoreboard
        Lugar = 0;
        HighScore[2] = HighScore[1];
        HighLevel[2] = HighLevel[1];
        HighName[2] = HighName[1];
        
        HighScore[1] = HighScore[0];
        HighLevel[1] = HighLevel[0];
        HighName[1] = HighName[0];

        HighScore[0] = Score;
        HighLevel[0] = Nivel;
        HighName[0] = " ";
      }
      else if(Score > HighScore[1] && Score < HighScore[0]){                                // Si el Score supera el score "medio" pero no el más alto
        Lugar = 1;
        HighScore[2] = HighScore[1];
        HighLevel[2] = HighLevel[1];
        HighName[2] = HighName[1];
        
        HighScore[1] = Score;
        HighLevel[1] = Nivel;
        HighName[1] = " ";
      }
      else if(Score < HighScore[1] && Score > HighScore[2]) {                               // Si el Score supera el score más bajo del scoreboard pero no el resto
        Lugar = 2;
        HighScore[2] = Score;
        HighLevel[2] = Nivel;
        HighName[2] = " ";
      }

      Write_Text(HighName[0], GameTypeX + 6*8, GameTypeY + 17*8, 0xffff, 0x0000);           // High Score: Primer Lugar        
      Write_Num(HighScore[0], 6, GameTypeX + 13*8, GameTypeY + 17*8, 0xffff, 0x0000);
      Write_Num(HighLevel[0], 2, GameTypeX + 20*8, GameTypeY + 17*8, 0xffff, 0x0000);

      Write_Text(HighName[1], GameTypeX + 6*8, GameTypeY + 19*8, 0xffff, 0x0000);           // High Score: Segundo Lugar
      Write_Num(HighScore[1], 6, GameTypeX + 13*8, GameTypeY + 19*8, 0xffff, 0x0000);
      Write_Num(HighLevel[1], 2, GameTypeX + 20*8, GameTypeY + 19*8, 0xffff, 0x0000);
      
      Write_Text(HighName[2], GameTypeX + 6*8, GameTypeY + 21*8, 0xffff, 0x0000);           // High Score: Tercer Lugar   
      Write_Num(HighScore[2], 6, GameTypeX + 13*8, GameTypeY + 21*8, 0xffff, 0x0000);
      Write_Num(HighLevel[2], 2, GameTypeX + 20*8, GameTypeY + 21*8, 0xffff, 0x0000);

      byte Letra = 26;
      byte NoCaracter = 0;
      char Nombre[6] = {32, 32, 32, 32, 32, 32};
      while(digitalRead(P1_SW) == HIGH){
        if (Letra == 0) Nombre[NoCaracter] = 32;
        else Nombre[NoCaracter] = (39 - (Letra + 13)) + 65;
        
        Letra = Letra % 27;

        LCD_TextCS(GameTypeX + 6*8 + (NoCaracter)*8, GameTypeY + 17*8 + 2*8*(Lugar), 8, 8, Letras, 40, Letra + 13, 1, 0, 0xffff, 0xfd00);
        delay(25);
        LCD_TextCS(GameTypeX + 6*8 + (NoCaracter)*8, GameTypeY + 17*8 + 2*8*(Lugar), 8, 8, Letras, 40, Letra + 13, 1, 0, 0xffff, 0x0000);
        delay(25);

        // Movimiento horizontal de palanca
        if(NoCaracter != 5 && analogRead(P1_URX) > 3000) {                                // Palanca Derecha
          NoCaracter ++;
          delay(80);
        }
        else if(NoCaracter != 0 && analogRead(P1_URX) < 300){                             // Palanca Izquierda
          NoCaracter --;
          delay(80);
        }

        // Movimiento vertical de palanca
        if(analogRead(P1_URY) > 4000){                                                    // Palanca Abajo
          Letra ++;
          delay(80);
        }
        else if(analogRead(P1_URY) < 100){                                                // Palanca Arriba
          if(Letra == 0) Letra = 26;
          else Letra --;
          delay(80);
        }
      }

      HighName[Lugar] = String(Nombre);
      Write_HighScoreSD(TipoJuego, HighScore, HighName, HighLevel);
    }
    

  }

  // MODO "2 JUGADORES" ---------------------------------------------------------
  // DESCRIPCIÓN: Modo versus donde se enfrentan dos jugadores en un tetris tipo A. No se muestran
  // estadísticas sobre aspectos como el número de piezas colocadas o el número de líneas limpiadas,
  // únicamente el punteo y el preview de la siguiente pieza.
  
  if (Modo == MultiPlayer){

  }

}


//***************************************************************************************************************************************
// Interrupciones para Controles
//***************************************************************************************************************************************

void P1SW_Presionado(){
   // attachInterrupt(digitalPinToInterrupt(P1_SW), P1SW_Presionado, RISING);
}


//***************************************************************************************************************************************
// Funciones de Juego
//***************************************************************************************************************************************

// =====================================================================
// GRÁFICO: CREACIÓN DE MARCOS GRISES EN PANTALLA DE JUEGO
// =====================================================================

void Crear_Marco(int CoordX, int CoordY, int Ancho, int Alto, byte TipoMarco){

  byte NoSprite = 0;
  byte CambioColor = 0;
  int ColorViejo = 0xffff;
  int ColorNuevo = 0xffff;
  
  switch(TipoMarco){
    case AreaJuego_Delgado: CambioColor = 0; NoSprite = 7; break;
    case AreaJuego_Grueso:  CambioColor = 0; NoSprite = 15; break;
    case MenuA_Grueso:      CambioColor = 0; NoSprite = 23; break;
    case MenuA_Delgado:     CambioColor = 0; NoSprite = 31; break;
    case MenuB_Grueso:      CambioColor = 1; ColorViejo = 0xfa60; ColorNuevo = 0x651f; NoSprite = 23; break;
    case MenuB_Delgado:     CambioColor = 1; ColorViejo = 0xfa60; ColorNuevo = 0x651f; NoSprite = 31; break;
    case Menu_Amarillo:     CambioColor = 0; NoSprite = 39; break;
    case Punteado_Amarillo: CambioColor = 0; NoSprite = 47; break;
    case Punteado_Verde:    CambioColor = 1; ColorViejo = 0xff91; ColorNuevo = 0x7788; NoSprite = 47; break;
    default:                CambioColor = 0; NoSprite = 7; break;
  }

  // Marco Superior
  LCD_SpriteCS(CoordX, CoordY, 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);                    // Sprite: Esquina superior izquierda
  NoSprite --;                                                                                                            // Sprite: Sección transversal superior
  for (int Tiles = 0; Tiles < (Ancho - 2); Tiles ++){                                                                     // Se rellena el espacio entre esquinas superiores con el sprite seleccionado
    LCD_SpriteCS(CoordX + (8 + 8*Tiles), CoordY, 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);        
  }
  NoSprite --;                                                                                                            // Sprite: Esquina superior derecha
  LCD_SpriteCS(CoordX + 8*(Ancho - 1), CoordY, 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);   

  // Marco Lateral Izquierdo
  NoSprite --;                                                                                                            // Sprite: Sección transversal izquierda
  for (int Tiles = 0; Tiles < (Alto - 2); Tiles ++){                                                                      // Se rellena el espacio entre las esquinas izquierdas
    LCD_SpriteCS(CoordX, CoordY + (8 + 8*Tiles), 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);   
  }
  
  // Marco Lateral Derecho
  NoSprite --;                                                                                                            // Sprite: Sección transversal derecha
  for (int Tiles = 0; Tiles < (Alto - 2); Tiles ++){
    LCD_SpriteCS(CoordX + 8*(Ancho - 1), CoordY + (8 + 8*Tiles), 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);   
  }

  // Marco Inferior
  NoSprite --;                                                                                                            // Sprite: Esquina inferior izquierda 
  LCD_SpriteCS(CoordX, CoordY + 8*(Alto - 1), 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);   
  NoSprite --;                                                                                                            // Sprite: Sección transversal inferior
  for (int Tiles = 0; Tiles < (Ancho - 2); Tiles ++){
    LCD_SpriteCS(CoordX + (8 + 8*Tiles), CoordY + 8*(Alto - 1), 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);   
  }
  NoSprite --;                                                                                                            // Sprite: Esquina inferior derecha
  LCD_SpriteCS(CoordX + 8*(Ancho - 1), CoordY + 8*(Alto - 1), 8, 8, Marcos, 48, NoSprite, 1, 0, CambioColor, ColorViejo, ColorNuevo);              

}

// =====================================================================
// GRÁFICO: GAME OVER SCREEN
// =====================================================================

void GameOver_Screen(int CoordX, int CoordY, byte Nivel, const int Color[][2]){
  Nivel = Nivel % 10;                                                                   // Existe una paleta por nivel. Con mod se asegura que se va a "ciclar" entre ellas.
  delay(800);
    
  for (int i = 0; i < 20; i++){
    FillRect(CoordX + 8, CoordY + 8 + 8*i, 8*10 - 1, 2, Color[Nivel][1]);
    FillRect(CoordX + 8, CoordY + 10 + 8*i, 8*10 - 1, 3, 0xffff);
    FillRect(CoordX + 8, CoordY + 13 + 8*i, 8*10 - 1, 2, Color[Nivel][0]);
    FillRect(CoordX + 8, CoordY + 15 + 8*i, 8*10 - 1, 1, 0x0000);
    delay(100);
  }
}

// =====================================================================
// GRÁFICO: ACTUALIZAR LOS TETROMINOS Y CONTADORES BAJO "STATISTICS"
// =====================================================================

void Update_Stats(int CoordX, int CoordY, byte Nivel, int TetrominosColocados[7]){

   Nivel = Nivel % 10;                                                                       // Existe una paleta por nivel. Con mod se asegura que se va a "ciclar" entre ellas.
   
   // Nota: Siempre dibujar los tetrominos pequeños en este orden para evitar overlap
   Tetromino(CoordX + 8 + 3, CoordY + 8*14 - 2, TI, 0, Small, Nivel);                        // Tetromino I
   Write_Num (TetrominosColocados[TI], 3, CoordX + 8*5, CoordY + 8*16 - 6, 0xe280, 0x0000);          
   Tetromino(CoordX + 8*2 + 1, CoordY + 8*12, TL, 0, Small, Nivel);                          // Tetromino L  
   Write_Num (TetrominosColocados[TL], 3, CoordX + 8*5, CoordY + 8*14 - 6, 0xe280, 0x0000);          
   Tetromino(CoordX + 8*2 + 1, CoordY + 8*10, TS, 0, Small, Nivel);                          // Tetromino S
   Write_Num (TetrominosColocados[TS], 3, CoordX + 8*5, CoordY + 8*12 - 6, 0xe280, 0x0000);          
   Tetromino(CoordX + 8*2 + 4, CoordY + 8*8 + 6, TO, 0, Small, Nivel);                       // Tetromino O 
   Write_Num (TetrominosColocados[TO], 3, CoordX + 8*5, CoordY + 8*10 - 6, 0xe280, 0x0000);          
   Tetromino(CoordX + 8*2 + 1, CoordY + 8*6, TZ, 0, Small, Nivel);                           // Tetromino Z
   Write_Num (TetrominosColocados[TZ], 3, CoordX + 8*5, CoordY + 8*8 - 6, 0xe280, 0x0000);           
   Tetromino(CoordX + 8*2 + 1, CoordY + 8*4, TJ, 0, Small, Nivel);                           // Tetromino J
   Write_Num (TetrominosColocados[TJ], 3, CoordX + 8*5, CoordY + 8*6 - 6, 0xe280, 0x0000);          
   Tetromino(CoordX + 8*2 + 1, CoordY + 8*2, TT, 0, Small, Nivel);                           // Tetromino T
   Write_Num(TetrominosColocados[TT], 3, StatsX + 8*5, StatsY + 8*4 - 6, 0xe280, 0x0000);            
}


// =====================================================================
// GRÁFICO: ACTUALIZAR EL BUFFER (ÁREA DE JUEGO RENDERIZADA)
// =====================================================================

void Update_Buffer(int CoordX, int CoordY, byte GameArea[][12], byte Buffer[][12], byte PosTAX, byte PosTAY, byte NoFilas, byte TetrominoActivo[][4], byte Nivel){

  // DESCRIPCIÓN: La "Game Area" sobre la que se mueven los tetrominos está compuesta por dos capas: Una capa base
  // que actualiza sus datos hasta que finaliza la caida del tetromino o "Game Area" y una capa "volatil" que se 
  // actualiza con cada movimiento del tetromino activo o "Buffer". El "Buffer" es el que se emplea como base para 
  // desplegar los gráficos en pantalla, mientras que la "Game Area" se utiliza para realizar decisiones dentro del
  // juego (Por ejemplo: Se emplea para determinar si la pieza puede caer, moverse hacia los lados o rotar). Esta
  // función utiliza los datos de la "Game Area" y el "Tetromino Activo" y los utiliza para dictarle al buffer donde
  // debe renderizar bloques y donde no.
  
  // Ajuste del nivel ingresado
  Nivel = Nivel % 10;                                                                     // Existe una paleta por nivel. Con mod se asegura que se va a "ciclar" entre ellas. 

  // Renderización de la "Game Area" de acuerdo al Estado del Tetromino Activo
  for(int i = 2; i < 22; i++){  
    //Serial.print(i);
    //Serial.print(": ");                                                   
    for(int j = 1; j < 11; j++){ 

      // CONDICIÓN 1: Región de Superposición
      // Si se ha llegado a la región en la que se encuentra la "Onion skin" o matriz superpuesta del tetromino 
      // activo (Primer IF) entonces se pasa a revisar las posiciones comunes (Segunda cadena de IF's). Se utilizan
      // tres reglas para comparar la matriz de bloques del "Tetromino Activo" (Onion Skin) y la matriz de la "Game Area"
      //    1. Si contienen un cero común, se coloca un cuadro negro y se escribe 0 en buffer (IF)
      //    2. Si GameArea != 0 y TetrominoActivo == 0 entonces Buffer = Game Area (Else IF)
      //    3. Si GameArea == 0 y TetrominoActivo != 0 entonces Buffer = Tetromino Activo (Else IF)
      if ((j >= PosTAX && j < PosTAX + NoFilas) && (i >= PosTAY && i < PosTAY + NoFilas)){
        if (GameArea[i][j] == 0 && TetrominoActivo[i - PosTAY][j - PosTAX] == 0){
          FillRect(CoordX + 8*j, CoordY + 8*i, 8 - 1, 8, 0x0000);
          Buffer[i][j] = 0;
        }
        else if (GameArea[i][j] != 0 && TetrominoActivo[i - PosTAY][j - PosTAX] == 0){
          Buffer[i][j] = GameArea[i][j];
        }
        else if (GameArea[i][j] == 0 && TetrominoActivo[i - PosTAY][j - PosTAX] != 0){
          Buffer[i][j] = TetrominoActivo[i - PosTAY][j - PosTAX];
        }
        else if (GameArea[i][j] != 0 && TetrominoActivo[i - PosTAY][j - PosTAX] != 0){
          FillRect(CoordX + 8*j, CoordY + 8*i, 8 - 1, 8, 0xffff);
          Buffer[i][j] = 0;
        }
      }

      // CONDICIÓN 2: Linea Superior de la Región de Superposición
      // Si se ha llegado a la fila que se encuentra arriba del tetromino actual (Más una posición adelante y detrás del tetromino)
      // se reescriben los valores presentes en el "Buffer" con los presentes en la "Game Area" y se colocan cuadros negros donde
      // existan espacios vacíos en la "Game Area" (If).
      if ((j >= (PosTAX - 1) && j <= PosTAX + NoFilas) && (i < PosTAY && i >= PosTAY - 2)){
        if (GameArea[i][j] == 0){
          FillRect(CoordX + 8*j, CoordY + 8*i, 8 - 1, 8, 0x0000);
        }
        Buffer[i][j] = GameArea[i][j];
      }

      // CONDICIÓN 3: Laterales de la Región de Superposición.
      // Si se ha llegado a las filas que se encuentran a los lados del tetromino actual (Exactamente a los lados) se realiza
      // el mismo proceso que en la condición 2: Se reescriben valores y se dibujan cuadros negros.
      if ((j == PosTAX - 1 || j == PosTAX + NoFilas) && (i >= PosTAY && i < PosTAY + NoFilas)){
        if (GameArea[i][j] == 0){
          FillRect(CoordX + 8*j, CoordY + 8*i, 8 - 1, 8, 0x0000);
        }
        Buffer[i][j] = GameArea[i][j];
      }

      //Serial.print(Buffer[i][j]);
      
      // RESULTADO: Se grafican los valores actualizados del "Buffer" en pantalla
      if (Buffer[i][j] != 0) {
        
        // Existen tres tipos de bloques distintos en tetris (NES): Un bloque vacío con orilla de color (3), un bloque sólido con el
        // mismo color que la orilla del anterior (2) y un bloque sólido con un color alternativo (1). En las matrices de bloques encontradas
        // en "constantes.h", como se puede observar, el código para el bloque con color alternativo es 1, entonces se chequea si se utilizará
        // un color alternativo al hacer "Bloques[][] == 1".
        if (Buffer[i][j] == 1){
          LCD_SpriteCS(CoordX + 8*j, CoordY + 8*i, 8, 8, TG, 3, Buffer[i][j] - 1, 1, 0, 1, Palette[0][1], Palette[Nivel][1]);
        }
        else{
          LCD_SpriteCS(CoordX + 8*j, CoordY + 8*i, 8, 8, TG, 3, Buffer[i][j] - 1, 1, 0, 1, Palette[0][0], Palette[Nivel][0]);
        }

      }
    }
    //Serial.println("");
  }
  //Serial.println("");
}

// =====================================================================
// GRÁFICO: ACTUALIZAR EL PREVIEW DEL SIGUIENTE BLOQUE QUE CAERÁ
// =====================================================================

void Update_Preview(int CoordX, int CoordY, byte TipoTetromino, byte Nivel){
  
  FillRect(CoordX, CoordY, 32, 32, 0x0000);

  switch (TipoTetromino){
    case TI: CoordY = CoordY - 8; break;
    case TO: CoordX = CoordX + 8; CoordY = CoordY + 8; break;
    default: CoordX = CoordX + 4;
  }                              

  if (TipoTetromino == TI) Write_Text("NEXT", CoordX, CoordY, 0xffff, 0x0000);
  Tetromino(CoordX, CoordY, TipoTetromino, 0, Big, Nivel);
}

// =====================================================================
// GRÁFICO: CREAR TETROMINO CON PATRÓN ESPECÍFICO
// =====================================================================

void Tetromino(int CoordX, int CoordY, byte TipoTetromino, byte NoRotaciones, byte Size, byte Nivel){

  // Se obtiene(n): 
  // - El tamaño de la matriz de bloques (NoFilas)
  // - Los datos del tipo de tetromino seleccionado y se guardan en "Bloques"
  byte NoFilas = 0;
  byte Bloques[4][4];
  NoFilas = Get_TetrominoData(TipoTetromino, Bloques);
  
  // Rotación de los bloques
  byte BloquesRotados[NoFilas][NoFilas]; 
  while (NoRotaciones > 0){   
    Rotate_TetrominoData(Bloques, NoFilas);
    NoRotaciones --;
  }

  // No de pixeles de ancho y alto en el sprite a utilizar
  byte NoPixeles = 8;
  if (Size == Big) NoPixeles = 8;                                                       // Si se grafica un tetromino grande, se especifica que este es un cuadrado de 8x8
  if (Size == Small) NoPixeles = 6;                                                     // Si se grafica un tetromino pequeño, se especifica que este es un cuadrado de 6x6

  // Ajuste del nivel ingresado para estar en un rango de 0 a 99
  Nivel = Nivel % 10;                                                                   // Existe una paleta por década. Con mod se asegura que se va a "ciclar" entre ellas. 

  // "Graficación" del Tetromino
  for(int i = 0; i < NoFilas; i++){                                                     
    for(int j = 0; j < NoFilas; j++){ 

      
      // Si valor en "Bloques" != 0
      // Se grafica el sprite deseado
      if(Bloques[i][j] != 0) {
        // Existen tres tipos de bloques distintos en tetris (NES): Un bloque vacío con orilla de color (3), un bloque sólido con el
        // mismo color que la orilla del anterior (2) y un bloque sólido con un color alternativo (1). En las matrices de bloques encontradas
        // en "constantes.h", como se puede observar, el código para el bloque con color alternativo es 1, entonces se chequea si se utilizará
        // un color alternativo al hacer "Bloques[][] == 1".

        if (Bloques[i][j] == 1){
          if (NoPixeles == 8) LCD_SpriteCS(CoordX + (NoPixeles*j), CoordY + (NoPixeles*i), 8, 8, TG, 3, Bloques[i][j] - 1, 1, 0, 1, Palette[0][1], Palette[Nivel][1]);
          if (NoPixeles == 6) LCD_SpriteCS(CoordX + (NoPixeles*j), CoordY + (NoPixeles*i), 6, 6, TP, 3, Bloques[i][j] - 1, 1, 0, 1, Palette[0][1], Palette[Nivel][1]);
        }
        else{
          if (NoPixeles == 8) LCD_SpriteCS(CoordX + (NoPixeles*j), CoordY + (NoPixeles*i), 8, 8, TG, 3, Bloques[i][j] - 1, 1, 0, 1, Palette[0][0], Palette[Nivel][0]);
          if (NoPixeles == 6) LCD_SpriteCS(CoordX + (NoPixeles*j), CoordY + (NoPixeles*i), 6, 6, TP, 3, Bloques[i][j] - 1, 1, 0, 1, Palette[0][0], Palette[Nivel][0]);
        }
        
      }
      
    }
  }
}


// =====================================================================
// JUEGO: ESCRIBIR "HIGH SCORES" EN TARJETA SD
// =====================================================================

void Write_HighScoreSD(byte TipoJuego, int HighScore[], String HighName[], byte HighLevel[]){
  
  if(TipoJuego == A) {
    SD.remove("AScore.txt");
    Archivo = SD.open("AScore.txt", FILE_WRITE);
  }
  else if(TipoJuego == B) {
    SD.remove("BScore.txt");
    Archivo = SD.open("BScore.txt", FILE_WRITE);
  }

  if(Archivo){
    for(int i = 0; i < 3; i++){
      Archivo.print("|");
      Archivo.print(HighName[i]);

      Archivo.print("|");
      int Decada = 100000;
      for (int j = 0; j < 6; j++){
        Archivo.print(((HighScore[i] % (Decada*10)) - (HighScore[i] % Decada)) / Decada);
        Decada = Decada / 10;
      }

      Archivo.print("|");
      Decada = 10;
      for (int j = 0; j < 2; j++){
        Archivo.print((HighLevel[i] - (HighLevel[i] % Decada)) / Decada);
        Decada = Decada / 10;
      }
      Archivo.println("");
    }
  }

  Archivo.close();
}

// =====================================================================
// JUEGO: LEER O EXTRAER "HIGH SCORES" DE TARJETA SD
// =====================================================================

void Read_HighScoreSD(int Scores[], String Names[], byte NivelRecord[], String NombreTXT){

  // DESCRIPCIÓN: Se leen los high scores guardados en el archivo especificado. Cada punteo y nombre está escrito en el 
  // TXT en el siguiente orden: |NOMBRE|5000. Por lo tanto, en esta función se leen las partes entre los símbolos '|' y 
  // se almacenan en diferentes variables. Lo que se debe tomar en cuenta es que los caracteres leidos se retornarán como 
  // chars por lo que se debe hacer la traducción de los mismos para poder ser utilizados en el programa. 
  
  int NoCaracteres = NombreTXT.length();                              // Se obtiene el número de caracteres en el String escrito
  char NombreArchivo[NoCaracteres + 1];                               // Se crea un array con el tamaño del número de caracteres
  NombreTXT.toCharArray(NombreArchivo, NoCaracteres + 1);             // Se almacena cada caracter individual 
  Archivo = SD.open(NombreArchivo);  

  Scores[0] = 0;                                                      // Se debe inicializar en 0 los scores y los niveles máximos para no contar sobre los datos previos
  Scores[1] = 0;                                                      // Ejemplo: Si los datos previos eran 1200 y se leen unos nuevos que eran 200, el algoritmo retornará
  Scores[2] = 0;                                                      // un valor de 1400. Esto no es lo deseado, por lo que se debe inicializar cada valor en 0.
  NivelRecord[0] = 0;
  NivelRecord[1] = 0;
  NivelRecord[2] = 0;
  char Caracteres[6] = {0, 0, 0, 0, 0, 0};
  byte Digito;
  int Decadas;



  if (Archivo){
    // Se lee una de las 3 filas de HIGH SCORE.
    for (int i = 0; i < 3; i++){
      // LETRAS: Si se detecta el separador '|' se traducen los siguientes 6 caracteres a letras
      while (Archivo.read() != '|');
      for(int j = 0; j < 6; j ++){
        Caracteres[j] = Archivo.read();
      }
      Names[i] = String(Caracteres);

      // NÚMEROS: Si se detecta el separador '|' se traducen los siguientes 6 caracteres a un único número
      while (Archivo.read() != '|');
      Decadas = 1;
      for(int j = 0; j < 6; j ++){
        Digito = Archivo.read();
        Scores[i] += (Digito - 48)*(100000 / Decadas);
        Decadas = 10*Decadas;
      }

      // NIVELES: Si se detecta el separador '|' se traducen los siguientes 2 caracteres a un número (Nivel)
      while(Archivo.read() != '|');
      Decadas = 1;
      for(int j = 0; j < 2; j++){
        Digito = Archivo.read();
        Digito = Digito - 48;
        NivelRecord[i] += Digito*(10 / Decadas);
        Decadas = 10*Decadas; 
      }
    }
  }
  Archivo.close();
  
}

// =====================================================================
// JUEGO: CALCULAR PUNTEO EN BASE AL NÚMERO DE LÍNEAS LIMPIADAS Y BLOQUES RECORRIDOS EN "SOFT DROP"
// =====================================================================

int Get_ScoreIncrement(int DeltaLineas, int Nivel, byte SoftDrop){

  int DeltaScore = 0;
  
  if (DeltaLineas != 0){
    switch(DeltaLineas){
      case 1: DeltaScore = 40*(Nivel + 1); break;
      case 2: DeltaScore = 100*(Nivel + 1); break;
      case 3: DeltaScore = 300*(Nivel + 1); break;
      case 4: DeltaScore = 1200*(Nivel + 1); break;
      default: DeltaScore = 0; break;
    }
  }

  DeltaScore += SoftDrop;
  return(DeltaScore);
}

// =====================================================================
// JUEGO: CHEQUEAR SI SE HA COMPLETADO UNA LÍNEA
// =====================================================================

int Update_ClearLines(int CoordX, int CoordY, byte GameArea[][12], byte Buffer[][12], int Nivel){

  // DESCRIPCIÓN: Cuando se ha completado una línea, la matriz de bloques de "Game Area" debe actualizarse
  // para limpiar las líneas completadas, colocar líneas negras donde existían dichas líneas y luego "correr"
  // los bloques que estaban por encima de las líneas limpiadas hacia abajo. Para detectar si una línea se ha
  // completado se aprovechará el hecho que la matriz de bloques almacena los diferentes tipos de bloques como
  // valores entre 1 a 3. Entonces se tomará una fila y se multiplicarán todos los valores en dicha fila. Si
  // existe un 0 o espacio vacío en la fila, entonces el resultado (Detector) retornará un 0 y se sabrá que la
  // línea no ha sido completada. Si la línea retorna un valor distinto de 0 (El número es irrelevante), entonces
  // se guarda el índice de dicha línea.

  // PRIMERO: Detectar líneas completadas
  byte ActivarDelay = 0;
  byte IndicesLineasLimpiadas[4] = {0,0,0,0};                           // 4 Posiciones porque en Tetris ese es el número máximo de líneas que se puede limpiar a la vez.
  int NoFilasLimpiadas = 0;                                             // Utilizado como el índice para los valores dentro de "IndicesLineasLimpiadas".

  // Nota: Dado el orden en el que se recorren las filas de "Game Area"
  // los valores del array de indices seguirán un patrón descendente, con
  // el valor [0] siendo el más grande y el [4] el más pequeño.
  for(int i = 2; i < 22; i++){
    for(int j = 1; j < 11; j++){
      if(GameArea[i][j] == 0) break;
      if(j == 10 && GameArea[i][j] != 0) {
        IndicesLineasLimpiadas[NoFilasLimpiadas] = i; 
        NoFilasLimpiadas ++;
      }
    }
    if(NoFilasLimpiadas == 4) break;
  }

  // SEGUNDO: Colocar líneas negras en las líneas completadas
  // Una línea ha sido completada cuando el valor dentro del array "IndicesLineasLimpiadas" != 0
  // Luego de limpiar la línea se coloca un breve delay para permitir que se aprecie "el borrado"
  for(int i = 0; i < 4; i++){
    if (IndicesLineasLimpiadas[i] != 0){
      for (int j = 0; j < 10; j++){
        FillRect(CoordX + 8 + 8*j, CoordY + 8*IndicesLineasLimpiadas[i] - 8, 8 - 1, 8, 0x0000);
        delay(100);
      }
    }
  }


  // Ajuste del nivel para que retorne un número entre 0 y 9
  Nivel = Nivel % 10;                                                                 
  
  // TERCERO: Correr los bloques por arriba de la línea completada, hacia abajo
  // En este caso se modifica la matriz de bloques de "Game Area".
  for(int i = 0; i < 4; i++){
    if (IndicesLineasLimpiadas[i] != 0){
      for(int j = IndicesLineasLimpiadas[i]; j > 1; j--){
        for (int k = 1; k < 11; k ++){
          Buffer[j][k] = GameArea[j-1][k];
          GameArea[j][k] = GameArea[j-1][k];
          if(GameArea[j][k] == 0)       FillRect(CoordX + 8*k, CoordY + 8*j - 8, 8 - 1, 8, 0x0000);
          else if(GameArea[j][k] == 1)  LCD_SpriteCS(CoordX + 8*k, CoordY + 8*j - 8, 8, 8, TG, 3, GameArea[j][k] - 1, 1, 0, 1, Palette[0][1], Palette[Nivel][1]);
          else                          LCD_SpriteCS(CoordX + 8*k, CoordY + 8*j - 8, 8, 8, TG, 3, GameArea[j][k] - 1, 1, 0, 1, Palette[0][0], Palette[Nivel][0]);
        }
      }
    }
  }

  // NOTA: Inicialmente podría parecer que "Update_Buffer" se encargará de graficar los
  // cambios dados, sin embargo, se debe recordar que esta función no grafica espacios
  // vacíos más allá de aquellos encontrados en los alrededores de "Tetromino Activo"
  // (Por cuestiones de "performance"). Por lo tanto, esta función (Update Clear Lines) 
  // se debe encargar de reescribir los espacios vacíos.
  return(NoFilasLimpiadas);
  
}

// =====================================================================
// JUEGO: CHEQUEAR SI SE PUEDE MOVER A LA IZQUIERDA
// =====================================================================

byte CheckLeft(byte GameArea[][12], byte PosX, byte PosY, byte NoFilas, byte TetrominoActivo[][4]){
  
  // Se utiliza una especie de sistema de "Ray Casting". Se determina cual es el bloque del tetromino activo 
  // más a la izquierda en cada fila y luego se calcula su distancia hacia el bloque más cercano a la izquierda.
  // Se encuentra la distancia más pequeña entre todas las filas. Si dicha distancia es menor a 1 (El tetromino 
  // activo está pegado a un obstáculo) se retorna un "Blocked" y se impide el movimiento del tetromino.

  byte PosBloque = 0;
  byte PosObstaculo = 0;
  byte Distancia = 11;
  
  //Serial.println("LEFT");
  
  // Se repite el proceso para cada fila del tetromino activo
  for(int i = 0; i < NoFilas; i++){
    
    // PRIMERO: Se determina cual es el bloque "más a la izquierda" de la fila actual
    for(int j = 0; j < NoFilas; j++){
      if (TetrominoActivo[i][j] != 0){                                                              // Si se encontró un bloque (Valor != 0)
        PosBloque = PosX + j;                                                                       // Se toma nota de la posición del bloque dentro del buffer
        break;                                                                                      // Y se rompe el for loop con variable "J"
      }
      if (TetrominoActivo[i][NoFilas - 1] == 0){                                                    // Si se llega al final del tetromino y no se encontró bloques
        PosBloque = 11;                                                                             // Se asigna a "PosBloque" la posición más alta que puede tener dentro de "Game Area": 11
      }
    }

    //Serial.print("Pos Bloque / Fila ");
    //Serial.print(i);
    //Serial.print(": ");
    //Serial.println(PosBloque);
    
    // SEGUNDO: Se calcula la posición del obstáculo más cercano (A la izquierda) al bloque encontrado
    // Solo se calcula si "PosBloque != 11". De lo contrario, se asigna un valor de 0 a "PosObstaculo".
    // Esto se hace ya que si PosBloque == 11 significa que no existe restricción sobre la fila dada. Entonces
    // se asigna PosObstaculo == 0 para que la resta de ambas posiciones retorne el valor más grande posible.
    if (PosBloque != 11){
      for(int j = PosBloque - 1; j >= 0; j--){
        if (GameArea[PosY + i][j] != 0){                                                              // Cuando se encuentre un bloque o frontera
          PosObstaculo = j;                                                                           // Se almacena la posición del obstáculo encontrado
          break;                                                                                      // Y se rompe el for loop con variable "J"
        }
      }
    }
    else PosObstaculo = 0;
    
    //Serial.print("Pos Obstaculo / Fila ");
    //Serial.print(i);
    //Serial.print(": ");
    //Serial.println(PosObstaculo);

    //Serial.print("Distancia ");
    //Serial.print(i);
    //Serial.print(": ");
    //Serial.println(PosBloque - PosObstaculo);
    
    // TERCERO: Se actualiza el valor de distancia en caso sea menor al valor previamente encontrado
    if (Distancia > (PosBloque - PosObstaculo)) Distancia = PosBloque - PosObstaculo;
    
  }

  //Serial.print("Distancia más pequeña: ");
  //Serial.println(Distancia);
  //Serial.println("");

  // CUARTO: Se "da permiso" de moverse en caso la distancia más pequeña sea mayor a 1.
  if (Distancia > 1) return(Clear);
  else return(Blocked);
  
}


// =====================================================================
// JUEGO: CHEQUEAR SI SE PUEDE MOVER A LA DERECHA
// =====================================================================

byte CheckRight(byte GameArea[][12], byte PosX, byte PosY, byte NoFilas, byte TetrominoActivo[][4]){
  
  // Se utiliza una especie de sistema de "Ray Casting". Se determina cual es el bloque del tetromino activo 
  // más a la derecha en cada fila y luego se calcula su distancia hacia el bloque más cercano a la derecha.
  // Se encuentra la distancia más pequeña entre todas las filas. Si dicha distancia es menor a 1 (El tetromino 
  // activo está pegado a un obstáculo) se retorna un "Blocked".

  byte PosBloque = 0;
  byte PosObstaculo = 0;
  byte Distancia = 11;

  // Se repite el proceso para cada fila del tetromino activo
  for(int i = 0; i < NoFilas; i++){
    
    // PRIMERO: Se determina cual es el bloque "más a la derecha" de la fila actual
    for(int j = NoFilas - 1; j >= 0; j--){
      if (TetrominoActivo[i][j] != 0){                                                              // Si se encontró un bloque (Valor != 0)
        PosBloque = PosX + j;                                                                       // Se toma nota de la posición del bloque dentro del buffer
        break;                                                                                      // Y se rompe el for loop con variable "J"
      }
      if (TetrominoActivo[i][0] == 0){                                                              // Si se llega al final del tetromino y no se encontró bloques
        PosBloque = 0;                                                                              // Se asigna a "PosBloque" la posición más baja que puede tener dentro de "Game Area": 0
      }
    }

    // SEGUNDO: Se calcula la posición del obstáculo más cercano (A la derecha) al bloque encontrado
    // Solo se calcula si "PosBloque != 0". De lo contrario, se asigna un valor de 11 a "PosObstaculo".
    // Esto se hace ya que si PosBloque == 0 significa que no existe restricción sobre la fila dada. Entonces
    // se asigna PosObstaculo == 11 para que la resta de ambas posiciones retorne el valor más grande posible.
    if (PosBloque != 0){
      for(int j = PosBloque + 1; j <= 11; j++){
        if (GameArea[PosY + i][j] != 0){                                                              // Cuando se encuentre un bloque o frontera
          PosObstaculo = j;                                                                           // Se almacena la posición del obstáculo encontrado
          break;                                                                                      // Y se rompe el for loop con variable "J"
        }
      }
    }
    else PosObstaculo = 11;
    
    // TERCERO: Se actualiza el valor de distancia en caso sea menor al valor previamente encontrado
    if (Distancia > (PosObstaculo - PosBloque)) Distancia = PosObstaculo - PosBloque;
    
  }

  // CUARTO: Se "da permiso" de moverse en caso la distancia más pequeña sea mayor a 1.
  if (Distancia > 1) return(Clear);
  else return(Blocked);
  
}


// =====================================================================
// JUEGO: CHEQUEAR SI SE PUEDE MOVER HACIA ABAJO
// =====================================================================

byte CheckDown(byte GameArea[][12], byte PosX, byte PosY, byte NoFilas, byte TetrominoActivo[][4]){
  
  // Se utiliza una especie de sistema de "Ray Casting". Se determina cual es el bloque del tetromino activo 
  // "más abajo" de cada columna y luego se calcula su distancia hacia el bloque más cercano hacia abajo.
  // Se encuentra la distancia más pequeña entre todas las columnas. Si dicha distancia es menor a 1 (El tetromino 
  // activo está pegado a un obstáculo) se retorna un "Blocked".

  byte PosBloque = 0;
  byte PosObstaculo = 0;
  byte Distancia = 22;

  // Se repite el proceso para cada columna del tetromino activo
  for(int i = 0; i < NoFilas; i++){
    
    // PRIMERO: Se determina cual es el bloque "más abajo" de la columna actual
    for(int j = NoFilas - 1; j >= 0; j--){
      if (TetrominoActivo[j][i] != 0){                                                              // Si se encontró un bloque (Valor != 0)
        PosBloque = PosY + j;                                                                       // Se toma nota de la posición del bloque dentro del buffer
        break;                                                                                      // Y se rompe el for loop con variable "J"
      }
      if (TetrominoActivo[0][i] == 0){                                                              // Si se llega al final del tetromino y no se encontró bloques
        PosBloque = 0;                                                                              // Se asigna a "PosBloque" la posición más baja que puede tener dentro de "Game Area": 0
      }
    }

    //Serial.print("Pos Bloque / Columna ");
    //Serial.print(i);
    //Serial.print(": ");
    //Serial.println(PosBloque);
    
    // SEGUNDO: Se calcula la posición del obstáculo más cercano (Hacia abajo) al bloque encontrado
    // Solo se calcula si "PosBloque != 0". De lo contrario, se asigna un valor de 21 a "PosObstaculo".
    // Esto se hace ya que si PosBloque == 0 significa que no existe restricción sobre la fila dada. Entonces
    // se asigna PosObstaculo == 22 para que la resta de ambas posiciones retorne el valor más grande posible.
    if (PosBloque != 0){
      for(int j = PosBloque + 1; j <= 22; j++){
        if (GameArea[j][PosX + i] != 0){                                                                // Cuando se encuentre un bloque o frontera
          PosObstaculo = j;                                                                           // Se almacena la posición del obstáculo encontrado
          break;                                                                                      // Y se rompe el for loop con variable "J"
        }
      }
    }
    else PosObstaculo = 22;

    //Serial.print("Pos Obstaculo / Columna ");
    //Serial.print(i);
    //Serial.print(": ");
    //Serial.println(PosObstaculo);

    //Serial.print("Distancia ");
    //Serial.print(i);
    //Serial.print(": ");
    //Serial.println(PosObstaculo - PosBloque);
    
    // TERCERO: Se actualiza el valor de distancia en caso sea menor al valor previamente encontrado
    if (Distancia > (PosObstaculo - PosBloque)) Distancia = PosObstaculo - PosBloque;
    
  }

  //Serial.print("Distancia más pequeña: ");
  //Serial.println(Distancia);
  //Serial.println("");
  
  // CUARTO: Se "da permiso" de moverse en caso la distancia más pequeña sea mayor a 1.
  if (Distancia > 1) return(Clear);
  else return(Blocked);
  
}

// =====================================================================
// JUEGO: CHEQUEAR SI SE PUEDE ROTAR EL TETROMINO
// =====================================================================

byte CheckRotation(byte GameArea[][12], byte PosX, byte PosY, byte NoFilas, byte TetrominoActivo[][4]){
  
  // Se utiliza un preview o "sombra" del tetromino rotado para determinar si la rotación deseada será
  // válida o no. Para esto se copian los datos actuales de la matriz de bloques en "CopiaTetrominoActivo" 
  // y se les aplica la rotación deseada. Luego se coloca la copia como una "Onion skin" sobre la "Game Area"
  // y se compara si existen puntos donde existen conflictos o valores distintos de cero comunes. Si es así
  // entonces se impide la rotación de la pieza. 

  // PRIMERO: Se copian los datos de TetrominoActivo en CopiaTetrominoActivo
  byte CopiaTetrominoActivo[4][4];
  for (int i = 0; i < NoFilas; i++){
    for (int j = 0; j < NoFilas; j++){
      CopiaTetrominoActivo[i][j] = TetrominoActivo[i][j];
    }
  }
  
  // SEGUNDO: Se rotan los datos de CopiaTetrominoActivo
  Rotate_TetrominoData(CopiaTetrominoActivo, NoFilas);

  // TERCERO: Se chequea para comprobar que no existan conflictos
  for (int i = 0; i < NoFilas; i++){
    for (int j = 0; j < NoFilas; j++){
      if (CopiaTetrominoActivo[i][j] != 0 && GameArea[PosY + i][PosX + j] != 0){
        return(Blocked);
      }
    }
  }

  // Si "TERCERO" se completa sin encontrar conflictos, entonces se retorna Clear.
  return(Clear);
  
  
}

// =====================================================================
// JUEGO: ACTUALIZAR LA GAME AREA CON LOS DATOS DEL BUFFER
// =====================================================================

void Update_GameArea(byte GameArea[][12], byte Buffer[][12]){
  for (int i = 1; i < 23; i++){
    for (int j = 0; j < 12; j++){
      GameArea[i][j] = Buffer[i][j];
    }
  }
}

// =====================================================================
// JUEGO: LIMPIAR LOS DATOS DEL BUFFER
// =====================================================================

void Clear_Matrix(byte Matrix[][12]){
  for (int i = 1; i < 22; i++){
    for (int j = 1; j < 11; j++){
      Matrix[i][j] = 0;
    }
  }
}

// =====================================================================
// JUEGO: POBLAR BUFFER Y GAME AREA CON BLOQUES BASURA DE ACUERDO CON 'ALTURA'
// =====================================================================

void Create_Garbage(byte Altura, byte GameArea[][12], byte Buffer[][12]){
  for (int i = 21; i >= (21 - (Altura*3 - 1)); i--){
    for (int j = 1; j < 11; j++){
      Buffer[i][j] = random(0,3);
      GameArea[i][j] = Buffer[i][j];
    }
  }
}

// =====================================================================
// JUEGO: OBTENER DATOS DE MATRIZ DE BLOQUES SELECCIONADA
// =====================================================================

byte Get_TetrominoData(byte TipoTetromino, byte Bloques[][4]){

  // Tamaño de la matriz de bloques según el tipo de tetrominos
  byte NoFilas = 3;
  switch(TipoTetromino){
    case TI: NoFilas = 4; break;                                                      // Si se elige un tetromino "I" se indica que su matriz de bloques es de 4x4
    case TO: NoFilas = 2; break;                                                      // Si se elige un tetromino "O" se indica que su matriz de bloques es de 2x2
    default: NoFilas = 3; break;                                                      // Si se elige cualquier otro tetromino se indica que su matriz de bloques es de 3x3
  }

  // Se copian (Celda por celda) los datos del tetromino seleccionado en la matriz ingresada.
  switch(TipoTetromino){
    case TI:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = I[i][j]; break;
    case TO:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = O[i][j]; break;
    case TL:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = L[i][j]; break;
    case TJ:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = J[i][j]; break;
    case TS:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = S[i][j]; break;
    case TZ:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = Z[i][j]; break;
    case TT:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = T[i][j]; break;
    default:  for(int i = 0; i < NoFilas; i++) for(int j = 0; j < NoFilas; j++) Bloques[i][j] = T[i][j]; break;
  }
  
  return(NoFilas);
}

// =====================================================================
// JUEGO: ROTAR MATRIZ DE BLOQUES DE TETROMINO ACTIVO
// =====================================================================

void Rotate_TetrominoData(byte TetrominoData[][4], byte NoFilas){

  // Se crea una variable temporal donde se almacena la matriz rotada
  // Si no se utiliza este intermediario, puede que no se obtengan los
  // resultados deseados.
  byte BloquesRotados[NoFilas][NoFilas];  
  
  // Rotación de los bloques del tetromino en sentido horario
  for(int i = 0; i < NoFilas; i++){
    for(int j = 0; j < NoFilas; j++){
      BloquesRotados[j][(NoFilas - 1)-i] = TetrominoData[i][j];                     
    }
  }

  // Se actualiza la matriz del tetromino activo. Se utiliza un for loop ya 
  // que el "Arduino IDE" carece de una manera más simple de igualar matrices
  for(int i = 0; i < NoFilas; i++){
    for(int j = 0; j < NoFilas; j++){
        TetrominoData[i][j] = BloquesRotados[i][j];                              
    }
  }
}

// =====================================================================
// TEXTO: ESCRITURA UTILIZANDO SPRITES DE TEXTO (8x8)
// =====================================================================

void Write_Text(String Texto, int CoordX, int CoordY, int ColorTexto, int ColorFondo) {
 
  int NoCaracteres = Texto.length();                        // Se obtiene el número de caracteres en el String escrito
  char Caracteres[NoCaracteres + 1];                        // Se crea un array con el tamaño del número de caracteres
  Texto.toCharArray(Caracteres, NoCaracteres + 1);          // Se almacena cada caracter individual 

  byte Flip = 1;                                            // Si se ingresa 0, el caracter se "flipea" horizontalmente. Solo utilizado para el triángulo apuntando a la izquierda
  byte IndiceSprite = 10;                                   // Si se ingresa un valor válido, el algoritmo coloca como default un espacio
  for (int i = 0; i < NoCaracteres; i++){
    switch(Caracteres[i]){
      case 99:  IndiceSprite = 0; break;                     // ASCII: 'c'  |  Sprite: Copyright                           
      case 62:  IndiceSprite = 1; break;                     // ASCII: '>'  |  Sprite: ->
      case 42:  IndiceSprite = 2; break;                     // ASCII: '*'  |  Sprite: "
      case 41:  IndiceSprite = 3; break;                     // ASCII: ')'  |  Sprite: )
      case 40:  IndiceSprite = 4; break;                     // ASCII: '('  |  Sprite: (
      case 91:  IndiceSprite = 5; break;                     // ASCII: '['  |  Sprite: Guión pegado a la izquierda
      case 93:  IndiceSprite = 6; break;                     // ASCII: ']'  |  Sprite: Guión pegado a la derecha
      case 33:  IndiceSprite = 7; break;                     // ASCII: '!'  |  Sprite: !
      case 47:  IndiceSprite = 8; break;                     // ASCII: '/'  |  Sprite: /
      case 123: IndiceSprite = 9; Flip = 0; break;           // ASCII: '{'  |  Sprite: Triangulo apuntando a la izquierda (Triangulo derecho "flipeado") 
      case 125: IndiceSprite = 9; break;                     // ASCII: '}'  |  Sprite: Triangulo apuntando a la derecha
      case 39:  IndiceSprite = 10; break;                    // ASCII: '''  |  Sprite: '
      case 44:  IndiceSprite = 11; break;                    // ASCII: ','  |  Sprite: ,
      case 45:  IndiceSprite = 12; break;                    // ASCII: '-'  |  Sprite: -
      case 32:  IndiceSprite = 13; break;                    // ASCII: ' '  |  Sprite: Espacio
      default:                                              // ASCII: Letras
        IndiceSprite = 39 - (Caracteres[i] - 65);
        break;
    }
    // Parámetros(Coordenada X, Coordenada Y, Ancho, Alto, NombreSprite, Total de Sprites, No. de Sprite, Flip, Offset, Color Texto, Color Fondo) 
    LCD_TextCS(CoordX + 8*i, CoordY, 8, 8, Letras, 40, IndiceSprite, Flip, 0, ColorTexto, ColorFondo);
  }
}

// =====================================================================
// TEXTO: ESCRITURA DE NÚMEROS UTILIZANDO SPRITES DE TEXTO (8x8)
// =====================================================================

void Write_Num(int Numero, byte NoDigitos, int CoordX, int CoordY, int ColorTexto, int ColorFondo){
  
  int Divisor = 10;
  int Digitos[NoDigitos + 1];
  
  Digitos[0] = Numero % Divisor;

  for (int i = 1; i < NoDigitos; i++){
    Digitos[i] = (Numero % (Divisor * 10) - Numero % Divisor) / Divisor; 
    Divisor = Divisor * 10;
  }

  for (int i = 0; i < NoDigitos; i++){
    int IndiceSprite = 9 - Digitos[i];
    LCD_TextCS(CoordX + 8*(NoDigitos - 1) - 8*i, CoordY, 8, 8, Numeros, 10, IndiceSprite, 1, 0, ColorTexto, ColorFondo);
  }

}

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}

//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 

//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}

//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]); 
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  
  if(flip){
    for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
      for (int i = 0; i < width; i++){
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k+1]);
          k = k - 2;
      } 
    }
  }
  
  else{
    for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
      for (int i = 0; i < width; i++){
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k+1]);
          k = k + 2;
      } 
    }
  }
  
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Sprite "Color swapped" - La función permite cambiar una de las tonalidades del sprite a dibujar
//***************************************************************************************************************************************
void LCD_SpriteCS(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset, char colorswap, int OldColor, int NewColor){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  
  if(flip){
    for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
      for (int i = 0; i < width; i++){
        if (colorswap == 1 && highByte(OldColor) == bitmap[k] && lowByte(OldColor) == bitmap[k+1]){
          LCD_DATA(NewColor >> 8);
          LCD_DATA(NewColor);
        }
        else{
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k+1]);
        }
        k = k - 2;
      } 
    }
  }
  
  else{
    for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
      for (int i = 0; i < width; i++){
        if (colorswap == 1 && highByte(OldColor) == bitmap[k] && lowByte(OldColor) == bitmap[k+1]){
          LCD_DATA(NewColor >> 8);
          LCD_DATA(NewColor);
        }
        else{
          LCD_DATA(bitmap[k]);
          LCD_DATA(bitmap[k+1]);
        }
        k = k + 2;
      }
    } 
  }
  
  digitalWrite(LCD_CS, HIGH);
}


//***************************************************************************************************************************************
// Texto "Color swapped" - La función permite cambiar el color del texto y su fondo
//***************************************************************************************************************************************

void LCD_TextCS(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset, int ColorTexto, int ColorFondo){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  
  if(flip){
    for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
      for (int i = 0; i < width; i++){
        if (bitmap[k] == 0xff && bitmap[k+1] == 0xff){
          LCD_DATA(ColorTexto >> 8);
          LCD_DATA(ColorTexto);
        }
        else if(bitmap[k] == 0x00 && bitmap[k+1] == 0x00){
          LCD_DATA(ColorFondo >> 8);
          LCD_DATA(ColorFondo);
        }
        k = k - 2;
      } 
    }
  }
  
  else{
    for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
      for (int i = 0; i < width; i++){
        if (bitmap[k] == 0xff && bitmap[k+1] == 0xff){
          LCD_DATA(ColorTexto >> 8);
          LCD_DATA(ColorTexto);
        }
        else if(bitmap[k] == 0x00 && bitmap[k+1] == 0x00){
          LCD_DATA(ColorFondo >> 8);
          LCD_DATA(ColorFondo);
        }
        k = k + 2;
      }
    } 
  }
  
  digitalWrite(LCD_CS, HIGH);
}


//***************************************************************************************************************************************
// Stream Sprite desde SD - Elimina la necesidad de guardar bitmaps en memoria, pero es más lento y no tiene la capacidad de desfasar o invertir el bitmap.
//***************************************************************************************************************************************
void LCD_BitmapSD(int x, int y, int width, int height, String TXT){
  
  // NOTA: Al utilizar esta función para desplegar Bitmaps desde una SD, se debe asegurar que el sprite ya esté en la posición en la que se
  // debería de desplegar. No debe existir necesidad de rotarla o alterarla de ninguna manera. Para asegurarse de esto al convertir los bitmaps en el
  // programa "LCD Image Converter" irse a Options / Conversion / Line Scan Direction. Ahí se debe colocar forward. Luego ya se coloca lo demás:
  // "Preset: Color R5G6B5" y "Image / Blocksize = 8 bits".
  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  int NoCaracteres = TXT.length();                          // Se obtiene el número de caracteres en el String escrito
  char TextFileName[NoCaracteres + 1];                      // Se crea un array con el tamaño del número de caracteres
  TXT.toCharArray(TextFileName, NoCaracteres + 1);          // Se almacena cada caracter individual 
  Archivo = SD.open(TextFileName);  
  
  char NibbleH = 0; 
  char NibbleL = 0;
  byte ConversionH = 0;
  byte ConversionL = 0;
  int DatosSD[2];

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);

  if (Archivo){
    for (int j = 0; j < height; j++){
      for (int i = 0; i < width; i++){
        for (int k = 0; k < 2; k++){
          while (Archivo.read() != 'x');
          NibbleH = Archivo.read();
          NibbleL = Archivo.read();
          if (NibbleH > 96) ConversionH = 87; else ConversionH = 48;
          if (NibbleL > 96) ConversionL = 87; else ConversionL = 48;
          DatosSD[k] = (NibbleH - ConversionH)*16 + (NibbleL - ConversionL);
          LCD_DATA(DatosSD[k]);
        }
      }
    } 
  }
  
  Archivo.close();
  digitalWrite(LCD_CS, HIGH);
}
