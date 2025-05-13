#include <Arduino.h>
#include <FastLED.h>
#include <TaskScheduler.h>

// Configuramos la parte fisica de la tira led
#define LED_PIN     D4        
#define NUM_LEDS    60       
#define BRIGHTNESS  50       
#define LED_TYPE    WS2812B   //Tipo de Tira de LED
#define COLOR_ORDER GRB       
#define MAX_POWER   500      // Límite de corriente en mA

// Variables globales
CRGB leds[NUM_LEDS];
Scheduler runner;

// Variables para animaciones
uint8_t gHue = 0;
uint8_t patternIndex = 0;
uint8_t colorIndex = 0;

// Declaración de funciones de efectos
void rainbowWave();
void colorWipe();
void fade();
void sparkle();
void meteor();
void breathing();
void reverseBlinkEffect();
void amberBlinkEffect();  // Nuevo efecto
void changePattern();
float thermistorRead();

// Configuración de tareas
Task taskPattern(50, TASK_FOREVER, &changePattern);
Task taskRainbow(50, TASK_FOREVER, &rainbowWave);
Task taskColorWipe(100, TASK_FOREVER, &colorWipe);
Task taskFade(30, TASK_FOREVER, &fade);
Task taskSparkle(50, TASK_FOREVER, &sparkle);
Task taskMeteor(30, TASK_FOREVER, &meteor);
Task taskBreathing(20, TASK_FOREVER, &breathing);
Task taskReverseBlink(100, TASK_FOREVER, &reverseBlinkEffect);
Task taskAmberBlink(100, TASK_FOREVER, &amberBlinkEffect); // Nueva tarea

void setup() {
  Serial.begin(115200);
  
  // Inicialización de FastLED con protección de corriente
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip)
    .setDither(BRIGHTNESS < 255);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  
  // Inicialización del planificador
  runner.init();
  runner.addTask(taskPattern);
  runner.addTask(taskRainbow);
  runner.addTask(taskColorWipe);
  runner.addTask(taskFade);
  runner.addTask(taskSparkle);
  runner.addTask(taskMeteor);
  runner.addTask(taskBreathing);
  runner.addTask(taskReverseBlink);
  runner.addTask(taskAmberBlink); // Nueva tarea
  
  // Activar efectos iniciales
  taskRainbow.enable();
  taskPattern.enable();
}

void loop() {
  runner.execute();
  
  // Lectura de comandos seriales
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'B' || cmd == 'b') {
      // Desactivar otros efectos
      taskRainbow.disable();
      taskColorWipe.disable();
      taskFade.disable();
      taskSparkle.disable();
      taskMeteor.disable();
      taskBreathing.disable();
      taskPattern.disable();
      taskAmberBlink.disable();
      // Activar efecto reversa
      taskReverseBlink.enable();
      Serial.println("Efecto: Parpadeo en Reversa");
    }
    else if (cmd == 'I' || cmd == 'i') {
      // Desactivar otros efectos
      taskRainbow.disable();
      taskColorWipe.disable();
      taskFade.disable();
      taskSparkle.disable();
      taskMeteor.disable();
      taskBreathing.disable();
      taskPattern.disable();
      taskReverseBlink.disable();
      // Activar efecto ámbar
      taskAmberBlink.enable();
      Serial.println("Efecto: Intermitente Ámbar");
    }
    else {
      // Desactivar efectos especiales
      taskReverseBlink.disable();
      taskAmberBlink.disable();
      // Reactivar efectos normales
      taskPattern.enable();
      taskRainbow.enable();
      Serial.println("Volviendo a secuencia normal de efectos");
    }
  }
  
  // Control de temperatura
  EVERY_N_SECONDS(10) {
    #ifdef ESP8266
      float temp = (thermistorRead() - 20.0) * 0.98;
      if (temp > 45.0) {
        FastLED.setBrightness(BRIGHTNESS / 2);
      } else {
        FastLED.setBrightness(BRIGHTNESS);
      }
    #endif
  }
}

// Función auxiliar para leer la temperatura
float thermistorRead() {
  int raw = analogRead(A0);
  float volt = raw * 3.3 / 1024.0;
  // Conversión aproximada a temperatura
  return (volt - 0.5) * 100;
}

// Efecto de ola arcoíris
void rainbowWave() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
  FastLED.show();
  gHue++;
}

// Efecto de barrido de color
void colorWipe() {
  static uint16_t pos = 0;
  static CRGB colors[] = {CRGB::Red, CRGB::Green, CRGB::Blue};
  
  leds[pos] = colors[colorIndex];
  FastLED.show();
  pos++;
  
  if (pos >= NUM_LEDS) {
    pos = 0;
    colorIndex = (colorIndex + 1) % 3;
    fadeToBlackBy(leds, NUM_LEDS, 64);
  }
}

// Efecto de desvanecimiento
void fade() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  fill_solid(leds, NUM_LEDS, CHSV(gHue, 255, 255));
  FastLED.show();
  gHue += 2;
}

// Efecto destello
void sparkle() {
  fadeToBlackBy(leds, NUM_LEDS, 128);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(random8(), 200, 255);
  FastLED.show();
}

// Efecto meteoro
void meteor() {
  fadeToBlackBy(leds, NUM_LEDS, 64);
  int pos = beatsin16(13, 0, NUM_LEDS-1);
  leds[pos] += CHSV(gHue, 255, 192);
  FastLED.show();
  gHue += 3;
}

// Efecto respiración
void breathing() {
  uint8_t sinBeat = beatsin8(30, 50, BRIGHTNESS, 0, 0);
  FastLED.setBrightness(sinBeat);
  fill_solid(leds, NUM_LEDS, CHSV(gHue, 255, 255));
  FastLED.show();
  gHue++;
}

// Efecto de parpadeo en reversa
void reverseBlinkEffect() {
  static bool state = false;
  
  // Limpiar todos los LEDs
  FastLED.clear();
  
  if (state) {
    // Encender primeros 15 LEDs en blanco
    fill_solid(leds, 15, CRGB::White);
    // Encender últimos 15 LEDs en blanco
    fill_solid(&leds[NUM_LEDS-15], 15, CRGB::White);
  }
  
  FastLED.show();
  state = !state;  // Alternar estado
}

// Efecto de parpadeo en ámbar
void amberBlinkEffect() {
  static bool state = false;
  
  // Limpiar todos los LEDs
  FastLED.clear();
  
  if (state) {
    // Color ámbar (mezcla de rojo y verde)
    CRGB amberColor = CRGB(255, 191, 0);
    // Encender primeros 15 LEDs
    fill_solid(leds, 15, amberColor);
    // Encender últimos 15 LEDs
    fill_solid(&leds[NUM_LEDS-15], 15, amberColor);
  }
  
  FastLED.show();
  state = !state;  // Alternar estado
}

// Cambio automático de patrones
void changePattern() {
  EVERY_N_SECONDS(10) {
    // Desactivar todos los efectos
    taskRainbow.disable();
    taskColorWipe.disable();
    taskFade.disable();
    taskSparkle.disable();
    taskMeteor.disable();
    taskBreathing.disable();
    taskReverseBlink.disable();
    taskAmberBlink.disable();
    
    // Cambiar al siguiente patrón
    patternIndex = (patternIndex + 1) % 8; // Ahora son 8 patrones
    
    // Activar el nuevo patrón
    switch(patternIndex) {
      case 0:
        taskRainbow.enable();
        Serial.println("Efecto: Ola Arcoíris");
        break;
      case 1:
        taskColorWipe.enable();
        Serial.println("Efecto: Barrido de Color");
        break;
      case 2:
        taskFade.enable();
        Serial.println("Efecto: Desvanecimiento");
        break;
      case 3:
        taskSparkle.enable();
        Serial.println("Efecto: Destello");
        break;
      case 4:
        taskMeteor.enable();
        Serial.println("Efecto: Meteoro");
        break;
      case 5:
        taskBreathing.enable();
        Serial.println("Efecto: Respiración");
        break;
      case 6:
        taskReverseBlink.enable();
        Serial.println("Efecto: Parpadeo en Reversa");
        break;
      case 7:
        taskAmberBlink.enable();
        Serial.println("Efecto: Parpadeo Ámbar");
        break;
    }
  }
}