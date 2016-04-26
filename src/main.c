#include <pebble.h>
#include "main.h"
#include "common.h"
#include "turn.h"
#include "settings.h"
#define PERSIST_KEY_LAUNCHES 10
#define TAP_NOT_DATA false
#define NUM_SAMPLES 1

#define INIT_DESC_THRESHOLD 1000
#define INIT_DELTA_LIMIT 200
#define INIT_ELAPSED_TIME 1000

static 	struct tm * timeinfo;	
static bool   toggle_reader = false;
static bool isReading = false;
static AppTimer* update_timer = NULL;
static double elapsed_time = 0;
static TextLayer *text_layer_start;
static DataLoggingSessionRef  s_log_data, s_log_ref ;
int lap_count = 0;
static BUFFER * s_Buffer;
static char buffer_data [13];
static AccelSamplingRate accel_sample_rate = ACCEL_SAMPLING_25HZ;


// Actually keeping track of time
static bool started = false;

static double start_time = 0;
static double pause_time = 0;
static TextLayer* big_time_layer;
static TextLayer* seconds_time_layer;


/**
 * Elementos da interface.
 */
static Window *s_main_window;

static TextLayer *s_intro_layer;
static time_t rawtime;
static uint64_t startTime;
int contador = 0;

void handle_timer(void* data);
void update_stopwatch();
void stop_stopwatch();
void start_stopwatch();



/**
 * Adquire dados do acelerômetro.
 */
static void data_handler(AccelData *data, uint32_t num_samples) {

  
  if(!toggle_reader)
  {
     accel_data_service_unsubscribe();
     isReading = false;
     APP_LOG(APP_LOG_LEVEL_INFO, "################## STOPPED #######################");    
    return;
  }else
  {
    if(!isReading)
    {
      if(startTime == 0)
      startTime = data[0].timestamp;
      isReading = true;
      APP_LOG(APP_LOG_LEVEL_INFO, "is reading %d", isReading);
    }    
  }  
  
  static char s_buffer_layer[11];

  if (startTime==0) {
    startTime = data[0].timestamp;
  } 
  
  int internalTime = (int)(data[0].timestamp - startTime);
  
  data[0].timestamp = internalTime;
  


  if (!data[0].did_vibrate ) {  
      
    contaViradas(data[0].x, data[0].y, data[0].z, data[0].timestamp, s_Buffer); 
     
     lap_count = s_Buffer->count * 25  ;
    
      //  APP_LOG(APP_LOG_LEVEL_INFO, "lap count after %i",lap_count ); 

  snprintf(s_buffer_layer, sizeof(s_buffer_layer),            
  " %d\n", 
   lap_count
  );
  text_layer_set_text(s_intro_layer, s_buffer_layer);  
   
  
  }
 // free(compassData);
}
static  void start_accel_reading()
{
  
  start_stopwatch();
    toggle_reader = true;    
 
    accel_data_service_subscribe(NUM_SAMPLES, data_handler) ;
    APP_LOG(APP_LOG_LEVEL_INFO, "starting accel sample %i ", (int)accel_sample_rate );   
    accel_service_set_sampling_rate(accel_sample_rate); 
  
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  	layer_set_hidden((Layer*)text_layer_start , true  ) ;    
 
   toggle_reader = toggle_reader ? false: true;

  
  if(toggle_reader)
    {
    if(strlen(buffer_data) == 0){
      
      time (&rawtime);
	  	timeinfo = localtime(&rawtime);
	  	strftime (buffer_data,13,"%d%m%y%H%M%S\n",timeinfo);   
    
      s_log_data = data_logging_create(101, DATA_LOGGING_BYTE_ARRAY, sizeof(buffer_data), false);    
      data_logging_log(s_log_data , buffer_data , 1);           
           
    }
   start_accel_reading();
    APP_LOG(APP_LOG_LEVEL_INFO, "################## STARTED #######################");    
    }else{
    stop_stopwatch();
  }
  
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  stop_stopwatch();
 
  inicializaBuffers(s_Buffer, s_log_ref);
   text_layer_set_text(s_intro_layer, " 0");  
 
    
start_time = 0;
 pause_time = 0; 

  if(toggle_reader){
  start_stopwatch();
  }
    text_layer_set_text(big_time_layer, "00:00");
   text_layer_set_text(seconds_time_layer, ".0");
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers 
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  
   window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
}

static void main_window_load(Window *window) {
 
	Layer *window_layer = window_get_root_layer(window);
  // Create TextLayer
  s_intro_layer = text_layer_create(GRect(4, 55, 140, 140));
    text_layer_set_font(s_intro_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_intro_layer));
  
  text_layer_set_text_alignment(s_intro_layer, GTextAlignmentCenter);
  GRect window_bounds = layer_get_bounds(window_layer);  	
  big_time_layer = text_layer_create(GRect(0, 5, 96, 35));  
    text_layer_set_text(big_time_layer, "00:00");
  text_layer_set_font(big_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));  
    text_layer_set_text_alignment(big_time_layer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), (Layer*)big_time_layer);
  

    seconds_time_layer = text_layer_create(GRect(96, 6, 49, 35));  
  text_layer_set_font(seconds_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));  
    text_layer_set_text(seconds_time_layer, ".0");
    layer_add_child(window_get_root_layer(window), (Layer*)seconds_time_layer);

	text_layer_start = text_layer_create(GRect(0, 50, window_bounds.size.w, window_bounds.size.h - 80));
  
	text_layer_set_text_color(text_layer_start, GColorBlack ); 
	text_layer_set_background_color(text_layer_start,GColorWhite);
	text_layer_set_text_alignment(text_layer_start,GTextAlignmentCenter);   
	text_layer_set_font(text_layer_start,  fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	layer_add_child(window_get_root_layer(window),text_layer_get_layer(text_layer_start));
  text_layer_set_text(text_layer_start,    "START"  );  
  
}
//Limpar a Tela
static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_intro_layer);
  accel_data_service_unsubscribe();
  
  
}

static void init() {

   //APP_LOG(APP_LOG_LEVEL_INFO, "size of buffer %i", (int)sizeof(s_Buffer));    
 // APP_LOG(APP_LOG_LEVEL_INFO, "size of data_log: %i", (int)sizeof(LOG)); 
  s_log_ref = data_logging_create(51, DATA_LOGGING_BYTE_ARRAY, sizeof(LOG), true);
  
  startTime = 0;
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
 
  
 window_set_click_config_provider(s_main_window, click_config_provider);
  
s_Buffer =  (BUFFER *)  malloc(sizeof(BUFFER)) ;
  
inicializaBuffers(s_Buffer, s_log_ref);
  
  
 //init_counter();
}
void start_stopwatch() {
    started = true;  
  
	if(start_time == 0) {
		start_time = float_time_ms();
	} else if(pause_time != 0) {
		double interval = float_time_ms() - pause_time;
		start_time += interval;
	}
    update_timer = app_timer_register(100, handle_timer, NULL);
}

void stop_stopwatch() {
    started = false;
	pause_time = float_time_ms();
    if(update_timer != NULL) {
        app_timer_cancel(update_timer);
        update_timer = NULL;
    }

   
}

void update_stopwatch() {
    static char big_time[] = "00:00";
    static char deciseconds_time[] = ".0";
    static char seconds_time[] = ":00";

    // Now convert to hours/minutes/seconds.
    int tenths = (int)(elapsed_time * 10) % 10;
    int seconds = (int)elapsed_time % 60;
    int minutes = (int)elapsed_time / 60 % 60;
    int hours = (int)elapsed_time / 3600;

    // We can't fit three digit hours, so stop timing here.
    if(hours > 99) {
        stop_stopwatch();
        return;
    }
	
	if(hours < 1) {
		snprintf(big_time, 6, "%02d:%02d", minutes, seconds);
		snprintf(deciseconds_time, 3, ".%d", tenths);
	} else {
		snprintf(big_time, 6, "%02d:%02d", hours, minutes);
		snprintf(seconds_time, 4, ":%02d", seconds);
	}

    // Now draw the strings.
    text_layer_set_text(big_time_layer, big_time);
    text_layer_set_text(seconds_time_layer, hours < 1 ? deciseconds_time : seconds_time);
}
void handle_timer(void* data) {
	if(started) {
		double now = float_time_ms();
		elapsed_time = now - start_time;
		update_timer = app_timer_register(100, handle_timer, NULL);
	}
	update_stopwatch();
}

static void deinit() {
    data_logging_finish(s_log_data);
   data_logging_finish(s_log_ref);
  free(s_Buffer);

  
text_layer_destroy(seconds_time_layer);
	text_layer_destroy(big_time_layer);
  	text_layer_destroy(text_layer_start);
  // Destroy Window
  window_destroy(s_main_window);  
//  deinit_counter();
}

int main(void) {
  init();
  init_menu_settings()  ;  
  app_event_loop();
  deinit();
}


void  set_sampling_rate(AccelSamplingRate rate){
  accel_sample_rate = rate;
  
}
