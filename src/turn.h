#pragma once

#define abs(x) (((x)<0)?(-(x)):(x))


struct queue {
  int32_t data[175];
  int32_t front;
  int32_t rear;
  int32_t sum;
  int32_t count;
  int32_t size;
};


typedef struct _buffers BUFFER;
struct _buffers {
  struct queue pitches; /* PITCH */
  int32_t lastTurnTime; /* Tempo da ultima virada */
  int32_t delta; /* para depuracao */
  int32_t d2; /* Duracao da virada */
  int16_t status; /* status = 1 => fazendo a virada; status = 0 => em nado */
  int16_t count; /* quantidade de viradas contabilizadas */ 
  int16_t p; /* marcacao para calculo da variacao */

};

void contaViradas (int32_t xacc, int32_t yacc, int32_t zacc, int32_t time, BUFFER * buf);
void inicializaBuffers (BUFFER * buf);




typedef struct _data_log LOG;
  struct _data_log{ 
  uint32_t time ;  
  int32_t delta ;
  int32_t tang;
  int16_t x ;
  int16_t y ;
  int16_t z; 
  bool status;  
};


