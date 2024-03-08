#include <Arduino.h>

// DEF E VARIAVEIS
#define saidaX 25         ///
#define saidaY 26         //|
#define saidaZ 27         //| Saídas do ESP
#define dirX 35           //|
#define dirY 32           //|
#define dirZ 33           //|
#define intPin 15         /// interrupt geral
#define pinFCXi 5         ///
#define pinFCXs 18        //|
#define pinFCYi 19        //|fins de curso
#define pinFCYs 21        //|
#define pinFCZi 22        //|
#define pinFCZs 23        ///
int posX = 0;             ///
int posY = 0;             //| posições absolutas em: (pulsos de entrada)x2
int posZ = 0;             ///
volatile int posXrel = 0; ///
volatile int posYrel = 0; //| posições relativas ao movimento anterior. utilizado para rampas
volatile int posZrel = 0; ///
int pulsosX = 10000000;   ///
int pulsosY = 10000000;   //| quantidade de pulsos para o proximo movimento. equivale a pulsos de entrada vezes dois
int pulsosZ = 10000000;   ///
int freqX = 10000;        ///
int freqY = 10000;        //| frequencia do alarme que gera os pulsos
int freqZ = 10000;        ///
int accRampaX = 1;        // aceleracao da rampa
int accRampaY = 1;        // aceleracao da rampa
int accRampaZ = 1;        // aceleracao da rampa
int freqRampaX = 100;     ///
int freqRampaY = 100;     //| frequencia de inicio da rampa
int freqRampaZ = 100;     ///
int freqStart = 100;
int tempoRampa = 2; // tempo da rampa em segundos
int rampaContX = 1;
int rampaContY = 1;
int rampaContZ = 1;
int timmerConstX = 1000000 / freqX; ///
int timmerConstY = 1000000 / freqY; //| periodo e constante de tempo dos alarmes.
int timmerConstZ = 1000000 / freqZ; ///
volatile bool FCXi = false;
volatile bool FCXs = false;
volatile bool FCYi = false;
volatile bool FCYs = false;
volatile bool FCZi = false;
volatile bool FCZs = false;
bool readyX = true;             ///
bool readyY = true;             //| determina se o eixo esta disponível para movimento
bool readyZ = true;             ///
volatile bool operating = true; // quando false impede que pulsos sejam gerados e posições alteradas pela função de pulso
bool moving = false;
bool stopAll = true; // quando atinge o fim de curso para todos os eixos
bool usarRampa = true;
int commConst = 1000;

// TIMERS
hw_timer_t *tempoX = NULL;
hw_timer_t *tempoY = NULL;
hw_timer_t *tempoZ = NULL;

// FUNCOES
void setupAlarmes();
void disparaAlarmes();
void paraAlarmes();
void pulso(int pin);
void setarRampas();
void IRAM_ATTR alarmX();
void IRAM_ATTR alarmY();
void IRAM_ATTR alarmZ();
void IRAM_ATTR parar();
void IRAM_ATTR pararX();
void IRAM_ATTR pararY();
void IRAM_ATTR pararZ();
void FC();
void setupPins();
void launchTasks();
void checkMSG();

// TASKS
TaskHandle_t tcontrole, tsaida;
void taskControle(void *parameters);
void taskMover(void *parameters);

// MAIN E LOOP
void setup()
{
  Serial.begin(9600);
  setupPins();
  delay(100);
  launchTasks();
}

void loop()
{
  vTaskDelete(NULL);
}

// COD. TASKS

void taskControle(void *parameters)
{
  while (true)
  {
    String msg = Serial.readString();

    vTaskDelay(commConst / portTICK_PERIOD_MS);
  }
}

void taskMover(void *parameters)
{
  posXrel = 0;
  posYrel = 0;
  posZrel = 0;
  paraAlarmes();
  setupAlarmes();
  if (operating)
  {
    disparaAlarmes();
  }
}
// FUNCOES

void setupAlarmes()
{
  tempoX = timerBegin(0, 40, true);
  timerAttachInterrupt(tempoX, &alarmX, true);
  timerAlarmWrite(tempoX, timmerConstX * freqX, true);
  tempoY = timerBegin(1, 40, true);
  timerAttachInterrupt(tempoY, &alarmY, true);
  timerAlarmWrite(tempoY, timmerConstY * freqY, true);
  tempoZ = timerBegin(2, 40, true);
  timerAttachInterrupt(tempoZ, &alarmZ, true);
  timerAlarmWrite(tempoZ, timmerConstZ * freqZ, true);
}

void disparaAlarmes()
{
  timerAlarmEnable(tempoX);
  timerAlarmEnable(tempoY);
  timerAlarmEnable(tempoZ);
}

void paraAlarmes()
{
  timerAlarmDisable(tempoX);
  timerAlarmDisable(tempoY);
  timerAlarmDisable(tempoZ);
}

void IRAM_ATTR alarmX()
{

  if (operating)
  {
    if (posX < pulsosX)
    {
      if (FCXs)
        FC();
      else
      {
        pulso(saidaX);
        posX++;
        posXrel++;
      }
    }
    else if (posX > pulsosX)
    {
      if (FCXi)
        FC();
      else
      {
        pulso(saidaX);
        posX--;
      }
    }
  }
  else if (digitalRead(saidaX) == HIGH)
  {
    if (posX < pulsosX)
    {
      pulso(saidaX);
      posX++;
    }
    else if (posX > pulsosX)
    {
      pulso(saidaX);
      posX--;
    }
  }
}
void IRAM_ATTR alarmY()
{
  if (operating)
  {
    if (posY < pulsosY)
    {
      if (FCYs)
        FC();
      else
      {
        pulso(saidaY);
        posY++;
      }
    }
    if (posY > pulsosY)
    {
      if (FCYi)
        FC();
      else
      {
        pulso(saidaY);
        posY--;
      }
    }
  }
  else if (digitalRead(saidaY) == HIGH)
  {
    if (posY < pulsosY)
    {
      pulso(saidaY);
      posY++;
    }
    if (posY > pulsosY)
    {
      pulso(saidaY);
      posY--;
    }
  }
}
void IRAM_ATTR alarmZ()
{
  if (operating)
  {
    if (posZ < pulsosZ)
    {
      if (FCZs)
        FC();
      else
      {
        pulso(saidaZ);
        posZ++;
      }
    }
    if (posZ > pulsosZ)
    {
      if (FCZi)
        FC();
      else
      {
        pulso(saidaZ);
        posZ--;
      }
    }
  }
  else if (digitalRead(saidaZ) == HIGH)
  {
    if (posZ < pulsosZ)
    {
      pulso(saidaZ);
      posZ++;
    }
    if (posZ > pulsosZ)
    {
      pulso(saidaZ);
      posZ--;
    }
  }
}

void IRAM_ATTR parar()
{
  operating = false;
  paraAlarmes();
}

void IRAM_ATTR pararXi()
{
  FCXi = false;
}

void IRAM_ATTR pararXs()
{
  FCXs = false;
}

void IRAM_ATTR pararYi()
{
  FCYi = false;
}

void IRAM_ATTR pararYs()
{
  FCYs = false;
}

void IRAM_ATTR pararZi()
{
  FCZi = false;
}

void IRAM_ATTR pararZs()
{
  FCZs = false;
}

void FC()
{
  if (stopAll)
  {
    operating = false;
  }
}

void pulso(int pin)
{
  digitalWrite(pin, !digitalRead(pin));
}

void setarRampas()
{
  int rampaContX = 1;
  int rampaContY = 1;
  int rampaContZ = 1;
}

void setupPins()
{
  attachInterrupt(intPin, parar, FALLING);
  attachInterrupt(pinFCXi, pararXi, FALLING);
  attachInterrupt(pinFCXs, pararXs, FALLING);
  attachInterrupt(pinFCYi, pararYi, FALLING);
  attachInterrupt(pinFCYs, pararYs, FALLING);
  attachInterrupt(pinFCZi, pararZi, FALLING);
  attachInterrupt(pinFCZs, pararZs, FALLING);
  pinMode(saidaX, OUTPUT);
  pinMode(saidaY, OUTPUT);
  pinMode(saidaZ, OUTPUT);
  digitalWrite(saidaX, LOW);
  digitalWrite(saidaY, LOW);
  digitalWrite(saidaZ, LOW);
  pinMode(dirX, OUTPUT);
  pinMode(dirY, OUTPUT);
  pinMode(dirZ, OUTPUT);
  digitalWrite(dirX, LOW);
  digitalWrite(dirY, LOW);
  digitalWrite(dirZ, LOW);
  delay(100);
}
void launchTasks()
{
  vTaskDelay(100);
}
void checkMSG() {}