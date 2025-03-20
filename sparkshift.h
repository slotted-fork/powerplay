#ifndef INCLUDE_SPARKSHIFT_H
#define INCLUDE_SPARKSHIFT_H

#include "powerplay.h"

struct config_t {
     int32_t power_excess_min;
     time_t averaging_secs;
     uint32_t sleep_secs;
     int debug;
     struct modbus_device gx;
     struct modbus_device evcs;
};

int config_from_env(struct config_t *config);

#endif
