#include <Arduino.h>

// DEF E VARIAVEIS
#define saidaPulso 14
#define saidaDir 35
#define intPin 15 /// interrupt geral
#define pinFCi 5  ///
#define pinFCs 18 //|

int posicaoAbs = 0;      ///
volatile int posRel = 0; ///
int pulsos = 10000000;   ///
int freq = 100000;       ///
int const coreTask = 0;

int timmerPrescaleValue = 40;
int timmerConst = 80000000 / timmerPrescaleValue; //| periodo e constante de tempo dos alarmes.
int timmerSetConst = timmerConst / 2;
volatile bool FCi = false;
volatile bool FCs = false;
volatile int setCounter = 0;

bool ready = true; ///

volatile bool operating = true; // quando false impede que pulsos sejam gerados e posições alteradas pela função de pulso
bool moving = false;
bool stopAll = true; // quando atinge o fim de curso para todos os eixos
bool usarRampa = true;
int commConst = 1000;

// TIMERS
hw_timer_t *tempo1 = NULL;
hw_timer_t *tempo2 = NULL;
hw_timer_t *tempoSet = NULL;

// FUNCOES
void setupAlarmes();
void disparaAlarmes();
void paraAlarmes();
void pulsoUp(int pin);
void pulsoDown(int pin);
void IRAM_ATTR alarmSet();
void IRAM_ATTR alarm1();
void IRAM_ATTR alarm2();
void IRAM_ATTR parar();
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
  // Serial.begin(9600);
  setupPins();
  delay(100);
  setupAlarmes();
  delay(100);
  disparaAlarmes();
  // launchTasks();
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
  posRel = 0;
  paraAlarmes();
  setupAlarmes();
  delay(100);
  if (operating)
  {
    disparaAlarmes();
  }
}
// FUNCOES

void setupAlarmes()
{

  tempoSet = timerBegin(2, timmerPrescaleValue, true);
  timerAttachInterrupt(tempoSet, &alarmSet, true);
  timerAlarmWrite(tempoSet, timmerSetConst / freq, true);
  tempo1 = timerBegin(0, timmerPrescaleValue, true);
  timerAttachInterrupt(tempo1, &alarm1, true);
  timerAlarmWrite(tempo1, timmerConst / freq, true);
  tempo2 = timerBegin(1, timmerPrescaleValue, true);
  timerAttachInterrupt(tempo2, &alarm2, true);
  timerAlarmWrite(tempo2, timmerConst / freq, true);
}

void disparaAlarmes()
{
  timerAlarmEnable(tempoSet);
}

void paraAlarmes()
{
  // timerAlarmDisable(tempo1);
  // timerAlarmDisable(tempo2);

  timerAlarmWrite(tempoSet, timmerSetConst * freq, false);
  timerAlarmDisable(tempoSet);
}

void IRAM_ATTR alarm1()
{

  // if (operating)
  // {
  //   if (posicaoAbs < pulsos)
  //   {
  //     if (FCs)
  //       FC();
  //     else
  //     {
  GPIO.out_w1ts = ((uint32_t)1 << saidaPulso); //"set"
  // GPIO.out_w1tc = ((uint32_t)1 << 22); //"clear"
  // pulsoUp(saidaPulso);
  posicaoAbs++;
  posRel++;
  //     }
  //   }
  //   else if (posicaoAbs > pulsos)
  //   {
  //     if (FCi)
  //       FC();
  //     else
  //     {
  //       pulsoUp(saidaPulso);
  //       posicaoAbs--;
  //       posRel--;
  //     }
  //   }
  //   else
  //   {
  //     paraAlarmes();
  //   }
  // }
}
void IRAM_ATTR alarm2()
{

  // if (operating)
  // {
  // GPIO.out_w1ts = ((uint32_t)1 << 22); //"set"
  GPIO.out_w1tc = ((uint32_t)1 << saidaPulso); //"clear"
                                               // pulsoDown(saidaPulso);
  //}
}
void IRAM_ATTR alarmSet()
{
  // Serial.print("setCounter:");
  // Serial.println(setCounter);
  if (setCounter == 2)
  {
    timerAlarmEnable(tempo1);
    // setCounter ++;
  }
  if (setCounter == 3)
  {
    timerAlarmEnable(tempo2);
    // setCounter ++;
  }
  if (setCounter > 3)
  {
    paraAlarmes();
  }
  setCounter++;
}

void IRAM_ATTR parar()
{
  operating = false;
  paraAlarmes();
}

void IRAM_ATTR pararI()
{
  FCi = false;
}

void IRAM_ATTR pararS()
{
  FCs = false;
}

void FC()
{
  if (stopAll)
  {
    operating = false;
  }
}

void pulsoUp(int pin)
{
  digitalWrite(pin, HIGH);
}

void pulsoDown(int pin)
{
  digitalWrite(pin, LOW);
}

void setupPins()
{
  attachInterrupt(intPin, parar, FALLING);
  attachInterrupt(pinFCi, pararI, FALLING);
  attachInterrupt(pinFCs, pararS, FALLING);
  pinMode(saidaPulso, OUTPUT);
  digitalWrite(saidaPulso, LOW);
  pinMode(saidaDir, OUTPUT);
  digitalWrite(saidaDir, LOW);
  delay(100);
}
void launchTasks()
{
  xTaskCreatePinnedToCore(taskMover, "taskMover", 1000, NULL, 1, &tsaida, coreTask);
  // vTaskDelay(100);
}
void checkMSG() {}