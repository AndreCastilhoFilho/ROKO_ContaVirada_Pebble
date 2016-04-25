#include <pebble.h>
#include "turn.h"
#include "main.h"

static int32_t table[] = {
     0,    16,  22,  27,  32,  35,  39,  42,  45,  48,  50,  53,  55,  57,
     59,   61,  64,  65,  67,  69,  71,  73,  75,  76,  78,  80,  81,  83,
     84,   86,  87,  89,  90,  91,  93,  94,  96,  97,  98,  99, 101, 102,
     103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 116, 117, 118,
     119, 120, 121, 122, 123, 124, 125, 126, 128, 128, 129, 130, 131, 132,
     133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 144, 145,
     146, 147, 148, 149, 150, 150, 151, 152, 153, 154, 155, 155, 156, 157,
     158, 159, 160, 160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168,
     169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178,
     179, 180, 181, 181, 182, 183, 183, 184, 185, 185, 186, 187, 187, 188,
     189, 189, 190, 191, 192, 192, 193, 193, 194, 195, 195, 196, 197, 197,
     198, 199, 199, 200, 201, 201, 202, 203, 203, 204, 204, 205, 206, 206,
     207, 208, 208, 209, 209, 210, 211, 211, 212, 212, 213, 214, 214, 215,
     215, 216, 217, 217, 218, 218, 219, 219, 220, 221, 221, 222, 222, 223,
     224, 224, 225, 225, 226, 226, 227, 227, 228, 229, 229, 230, 230, 231,
     231, 232, 232, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238,
     239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246,
     246, 247, 247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253,
     253, 254, 254, 255
  };

static int32_t adjustment(int x, int xn) {
  // Added by Jeff Lawson:
  // need to test:
  //   if  |xn * xn - x|  >  |x - (xn-1) * (xn-1)|  then xn-1 is more accurate
  //   if  |xn * xn - x|  >  |(xn+1) * (xn+1) - x|  then xn+1 is more accurate
  // or, for all cases except x == 0:
  //    if  |xn * xn - x|  >  x - xn * xn + 2 * xn - 1 then xn-1 is more accurate
  //    if  |xn * xn - x|  >  xn * xn + 2 * xn + 1 - x then xn+1 is more accurate
  int xn2 = xn * xn;

  // |xn * xn - x|
  int comparitor0 = xn2 - x;
  if (comparitor0 < 0) {
    comparitor0 = -comparitor0;
  }

  int twice_xn = xn << 1;

  // |x - (xn-1) * (xn-1)|
  int comparitor1 = x - xn2 + twice_xn - 1;
  if (comparitor1 < 0) { // need to correct for x == 0 case?
    comparitor1 = -comparitor1; // only gets here when x == 0
  }

  // |(xn+1) * (xn+1) - x|
  int comparitor2 = xn2 + twice_xn + 1 - x;

  if (comparitor0 > comparitor1) {
    return (comparitor1 > comparitor2) ? ++xn : --xn;
  }

  return (comparitor0 > comparitor2) ? ++xn : xn;
}
static int32_t f32_sqrt(int32_t x) {
    int xn;

    if (x >= 0x10000) {
      if (x >= 0x1000000) {
        if (x >= 0x10000000) {
          if (x >= 0x40000000) {
            xn = table[x >> 24] << 8;
          } else {
            xn = table[x >> 22] << 7;
          }
        } else {
          if (x >= 0x4000000) {
            xn = table[x >> 20] << 6;
          } else {
            xn = table[x >> 18] << 5;
          }
        }

        xn = (xn + 1 + (x / xn)) >> 1;
        xn = (xn + 1 + (x / xn)) >> 1;
        return adjustment(x, xn);
      } else {
        if (x >= 0x100000) {
          if (x >= 0x400000) {
            xn = table[x >> 16] << 4;
          } else {
            xn = table[x >> 14] << 3;
          }
        } else {
          if (x >= 0x40000) {
            xn = table[x >> 12] << 2;
          } else {
            xn = table[x >> 10] << 1;
          }
        }

        xn = (xn + 1 + (x / xn)) >> 1;

         return adjustment(x, xn);
      }
    } else {
      if (x >= 0x100) {
        if (x >= 0x1000) {
          if (x >= 0x4000) {
            xn = (table[x >> 8]) + 1;
          } else {
            xn = (table[x >> 6] >> 1) + 1;
          }
        } else {
          if (x >= 0x400) {
            xn = (table[x >> 4] >> 2) + 1;
          } else {
            xn = (table[x >> 2] >> 3) + 1;
          }
        }

        return adjustment(x, xn);
      } else {
        if (x >= 0) {
          return adjustment(x, table[x] >> 4);
        }
      }
    }
    
    return -1;
}



int16_t QUEUEremoveData(struct queue * q) {
  int d = q->data[q->front];
  q->count -= 1;
  q->sum -= d;
  q->front = (q->front + 1)%(q->size);
  return d;
}
void QUEUEaddData(struct queue * q, int32_t d) {
  if(q->count >= q->size ) 
    QUEUEremoveData(q);
  q->data[q->rear] = d;
  q->count += 1;
  q->sum += d;
  q->rear = (q->rear + 1)%(q->size);
}

int16_t QUEUEnext(struct queue *q) {
  return  q->data[q->front];

}

void QUEUEinitialize(struct queue * q, int32_t size) {
  q->size = size;
  q->front = 0;
  q->data[0] = 0;
  q->rear = 0;
  q->sum= 0;
  q->count = 0;
}

void inicializaBuffers (BUFFER * buf ) {
  QUEUEinitialize(&(buf->pitches),175);
  buf->lastTurnTime = 0;
  buf->d2 = 0;
  buf->status = 0;
  buf->count = 0;
  buf->p = 0;
  APP_LOG(APP_LOG_LEVEL_INFO, "buffer inicializado");   
}

int32_t tangent_pitch (int32_t xacc, int32_t yacc, int32_t zacc) {
  if(yacc == 0 && zacc==0) {
    return 0;
  }
  return (5656 * xacc) / f32_sqrt(yacc*yacc + zacc*zacc);
}

void contaViradas (int32_t xacc, int32_t yacc, int32_t zacc, int32_t time, BUFFER * buf) {
  int32_t p = tangent_pitch(xacc,yacc, zacc);
bool virada = false;
  QUEUEaddData(&(buf->pitches), p);

  if((buf->pitches).count >= (buf->pitches).size) 
    buf->delta = (-(buf->pitches).sum/(buf->pitches).count + 10020)/130;
  
  if(buf->delta  > 25)  {
    if(((time - buf->lastTurnTime) > 12500) && (buf->status == 0))  { /* VIRADA RECONHECIDA */
      buf->lastTurnTime = time;
      buf->d2 = time;
      buf->status = 1;
      buf->count ++;
      virada = true;
    
    }
  } 
  if((time - buf->d2) > 3000 && (buf->status == 1)){
    buf->status = 0;
    buf->lastTurnTime = time;
  }
  
  
    
   LOG *s_data = malloc(sizeof(*s_data));
  
      s_data-> time = time;
       s_data-> x = xacc;
       s_data-> y =  yacc;
       s_data-> z = zacc;
       s_data->delta =buf->delta;
       s_data->tang = p;
       s_data-> status = virada;
      


    DataLoggingResult logggg = data_logging_log(getDataLog(), s_data , 1);
    
    // APP_LOG(APP_LOG_LEVEL_INFO, "t: %i\n x: %i\n y: %i\n z: %i\ndelta: %i\ntang: %i\nstatus: %i", (int)s_data-> time,(int)s_data-> x ,(int) s_data-> y,(int) s_data-> z, (int)s_data-> delta, (int)s_data-> tang, (int)s_data-> status);
  
    free(s_data);
    
    if (logggg != DATA_LOGGING_SUCCESS) {    
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error datalogging: %d",(int)logggg);
    }       

//     APP_LOG(APP_LOG_LEVEL_INFO, "free: %d",(int) heap_bytes_free());
  
 
  
}






