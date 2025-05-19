#include <Arduino.h>
#include <FastLED.h>
#include <TaskScheduler.h>

// Configuración de la tira LED
#define LED_PIN     D4        
#define NUM_LEDS    60       
#define BRIGHTNESS  50       
#define LED_TYPE    WS2812B   // Tipo de Tira de LED
#define COLOR_ORDER GRB       
#define MAX_POWER   500       // Límite de corriente en mA

// Variables globales
CRGB leds[NUM_LEDS];
Scheduler runner;

// Variables para animaciones
uint8_t gHue = 0;
uint8_t patternIndex = 0;
uint8_t colorIndex = 0;

// Variables para controlar efectos direccionales
int leftPosition = NUM_LEDS - 1;
int rightPosition = 0;

// Control de comandos y estado
char lastCommand = ' ';
bool inNormalMode = true;
bool inReverseMode = false;
bool inBlinkMode = false;
bool inLeftMode = false;
bool inRightMode = false;
bool inStopMode = false;

// Declaración de funciones de efectos
void rainbowWave();
void colorWipe();
void fade();
void sparkle();
void meteor();
void breathing();
void reverseBlinkEffect();
void amberBlinkEffect();
void directionLeftEffect();
void directionRightEffect();
void stopEffect();
void changePattern();
float thermistorRead();
void disableAllEffects();

// Declaración de funciones de cambio de modo
void switchToNormalMode();
void switchToReverseMode();
void switchToBlinkMode();
void switchToLeftMode();
void switchToRightMode();
void switchToStopMode();

// Configuración de tareas
Task taskPattern(50, TASK_FOREVER, &changePattern);
Task taskRainbow(50, TASK_FOREVER, &rainbowWave);
Task taskColorWipe(100, TASK_FOREVER, &colorWipe);
Task taskFade(30, TASK_FOREVER, &fade);
Task taskSparkle(50, TASK_FOREVER, &sparkle);
Task taskMeteor(30, TASK_FOREVER, &meteor);
Task taskBreathing(20, TASK_FOREVER, &breathing);
Task taskReverseBlink(100, TASK_FOREVER, &reverseBlinkEffect);
Task taskAmberBlink(100, TASK_FOREVER, &amberBlinkEffect);
Task taskDirectionLeft(100, TASK_FOREVER, &directionLeftEffect);
Task taskDirectionRight(100, TASK_FOREVER, &directionRightEffect);
Task taskStop(1000, TASK_FOREVER, &stopEffect);

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
  runner.addTask(taskAmberBlink);
  runner.addTask(taskDirectionLeft);
  runner.addTask(taskDirectionRight);
  runner.addTask(taskStop);
  
  // Activar efectos iniciales
  taskRainbow.enable();
  taskPattern.enable();
  
  // Mostrar instrucciones en el monitor serial
  Serial.println("Sistema inicializado - Comandos disponibles:");
  Serial.println("- 'B': Reversa (Parpadean los primeros y últimos 15 LEDs en blanco)");
  Serial.println("- 'I': Intermitentes (Parpadean los primeros y últimos 15 LEDs en ámbar)");
  Serial.println("- 'L': Izquierda (Animación direccional izquierda usando dos LEDs)");
  Serial.println("- 'R': Derecha (Animación direccional derecha usando dos LEDs)");
  Serial.println("- 'S': Alto (Enciende los primeros y últimos 15 LEDs en rojo)");
  Serial.println("Enviar el mismo comando para desactivar y volver a modo RGB");
}

void loop() {
  runner.execute();
  
  // Lectura de comandos seriales
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Sistema de toggle (activar/desactivar con el mismo comando)
    if (cmd == lastCommand && cmd != ' ') {
      // Si el mismo comando se recibe dos veces, vuelve al modo normal
      switchToNormalMode();
      lastCommand = ' ';
    } else {
      // Procesar nuevo comando
      switch (cmd) {
        case 'B': case 'b':
          switchToReverseMode();
          lastCommand = 'B';
          break;
        case 'I': case 'i':
          switchToBlinkMode();
          lastCommand = 'I';
          break;
        case 'L': case 'l':
          switchToLeftMode();
          lastCommand = 'L';
          break;
        case 'R': case 'r':
          switchToRightMode();
          lastCommand = 'R';
          break;
        case 'S': case 's':
          switchToStopMode();
          lastCommand = 'S';
          break;
      }
    }
  }
  
  // Control de temperatura
  EVERY_N_SECONDS(10) {
    #ifdef ESP8266
      float temp = (thermistorRead() - 20.0) * 0.98;
      if (temp > 45.0) {
        FastLED.setBrightness(BRIGHTNESS / 2);
        Serial.print("Temperatura alta (");
        Serial.print(temp);
        Serial.println("°C) - Reduciendo brillo");
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

// Función para deshabilitar todos los efectos
void disableAllEffects() {
  taskRainbow.disable();
  taskColorWipe.disable();
  taskFade.disable();
  taskSparkle.disable();
  taskMeteor.disable();
  taskBreathing.disable();
  taskReverseBlink.disable();
  taskAmberBlink.disable();
  taskDirectionLeft.disable();
  taskDirectionRight.disable();
  taskStop.disable();
  taskPattern.disable();
  
  // Limpiar todos los LEDs
  FastLED.clear();
}

// === FUNCIONES DE CAMBIO DE MODO ===

void switchToNormalMode() {
  disableAllEffects();
  inNormalMode = true;
  inReverseMode = false;
  inBlinkMode = false;
  inLeftMode = false;
  inRightMode = false;
  inStopMode = false;
  
  taskPattern.enable();
  taskRainbow.enable();
  Serial.println("Volviendo a secuencia normal de efectos");
}

void switchToReverseMode() {
  disableAllEffects();
  inNormalMode = false;
  inReverseMode = true;
  
  taskReverseBlink.enable();
  Serial.println("Efecto: Parpadeo en Reversa");
}

void switchToBlinkMode() {
  disableAllEffects();
  inNormalMode = false;
  inBlinkMode = true;
  
  taskAmberBlink.enable();
  Serial.println("Efecto: Intermitente Ámbar");
}

void switchToLeftMode() {
  disableAllEffects();
  inNormalMode = false;
  inLeftMode = true;
  leftPosition = NUM_LEDS - 1;
  
  taskDirectionLeft.enable();
  Serial.println("Efecto: Dirección Izquierda");
}

void switchToRightMode() {
  disableAllEffects();
  inNormalMode = false;
  inRightMode = true;
  rightPosition = 0;
  
  taskDirectionRight.enable();
  Serial.println("Efecto: Dirección Derecha");
}

void switchToStopMode() {
  disableAllEffects();
  inNormalMode = false;
  inStopMode = true;
  
  taskStop.enable();
  Serial.println("Efecto: Alto (Rojo)");
}

// === FUNCIONES DE EFECTOS ===

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

// Efecto dirección izquierda
void directionLeftEffect() {
  // Limpiar todos los LEDs
  FastLED.clear();
  
  // Primer LED en verde, segundo en rojo
  leds[leftPosition] = CRGB::Green;
  if (leftPosition < NUM_LEDS - 1) {
    leds[leftPosition + 1] = CRGB::Red;
  }
  
  // Mostrar y avanzar posición
  FastLED.show();
  leftPosition--;
  
  // Reiniciar cuando llegue al inicio
  if (leftPosition < 0) {
    leftPosition = NUM_LEDS - 1;
  }
}

// Efecto dirección derecha
void directionRightEffect() {
  // Limpiar todos los LEDs
  FastLED.clear();
  
  // Primer LED en verde, segundo en rojo
  leds[rightPosition] = CRGB::Green;
  if (rightPosition > 0) {
    leds[rightPosition - 1] = CRGB::Red;
  }
  
  // Mostrar y avanzar posición
  FastLED.show();
  rightPosition++;
  
  // Reiniciar cuando llegue al final
  if (rightPosition >= NUM_LEDS) {
    rightPosition = 0;
  }
}

// Efecto alto (rojo fijo)
void stopEffect() {
  static bool state = false;
  
  // Limpiar todos los LEDs
  FastLED.clear();
  
  // Color rojo brillante
  CRGB redColor = CRGB::Red;
  
  // Encender los LEDs en rojo (siempre encendidos o parpadeando)
  if (state || true) {  // Para que siempre esté encendido, usa "true"
    // Encender primeros 15 LEDs
    fill_solid(leds, 15, redColor);
    // Encender últimos 15 LEDs
    fill_solid(&leds[NUM_LEDS-15], 15, redColor);
  }
  
  FastLED.show();
  state = !state;  // Alternar estado (para parpadeo si se desea)
}

// Cambio automático de patrones
void changePattern() {
  EVERY_N_SECONDS(10) {
    if (!inNormalMode) return;  // No cambiar si estamos en un modo especial
    
    // Desactivar todos los efectos
    taskRainbow.disable();
    taskColorWipe.disable();
    taskFade.disable();
    taskSparkle.disable();
    taskMeteor.disable();
    taskBreathing.disable();
    taskReverseBlink.disable();
    taskAmberBlink.disable();
    taskDirectionLeft.disable();
    taskDirectionRight.disable();
    taskStop.disable();
    
    // Cambiar al siguiente patrón
    patternIndex = (patternIndex + 1) % 6;  // Solo 6 para el modo normal
    
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
    }
  }
}