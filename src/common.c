#include <pebble.h>
#include "common.h"

void format_lap(double lap_time, char* buffer) {
  int tenths = (int)(lap_time * 10) % 10;
  int seconds = (int)lap_time % 60;
  int minutes = (int)lap_time / 60 % 60;
  int hours = (int)lap_time / 3600;

	snprintf(buffer, 11, "%02d:%02d:%02d.%d", hours, minutes, seconds, tenths);
}

double float_time_ms() {
	time_t seconds;
	uint16_t milliseconds;
	time_ms(&seconds, &milliseconds);
	return (double)seconds + ((double)milliseconds / 1000.0);
}


