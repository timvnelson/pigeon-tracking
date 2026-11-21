// Stand-in for loft/config.h when testing on a computer: the location is a
// variable, so one test run can try many places. Only what SolarSchedule.cpp
// needs is here.
#pragma once
#include <stdint.h>

extern double g_lat, g_lon;
#define LOFT_LATITUDE  g_lat
#define LOFT_LONGITUDE g_lon

static const int16_t OPEN_OFFSET_MIN  = 0;
static const int16_t CLOSE_OFFSET_MIN = 30;
