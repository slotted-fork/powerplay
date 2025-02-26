#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <modbus/modbus.h>

#define POWERPLAY_IMPLEMENTATION
#include "powerplay.h"

bool config_from_env(struct config_t *config);

/*
 * Environment variables required for configuration
 *
 * GX_HOST		: IP address of GX device
 * GX_PORT		: Modbus TCP port of GX device
 *
 * EVCS_HOST		: IP address of EVCS device
 * ECVS_PORT		: Modbus TCP port of EVCS device
 *
 */

int main(void)
{
     modbus_t *gx_ctx, *evcs_ctx;

     struct config_t config = {0};
     struct gx_data_t gx_data = {0};
     struct evcs_data_t evcs_data = {0};

     if (config_from_env(&config)) return 1;
     if (modbus_device_connect(config.evcs, &evcs_ctx)) return 1;
     if (modbus_device_connect(config.gx, &gx_ctx)) return 1;

     if (modbus_set_slave(gx_ctx, 100) == -1) { /* com.victronenergy.system */
	  fprintf(stderr, "Error: setting modbus slave for GX failed: %s\n", modbus_strerror(errno));
	  return 1;
     }

     if(evcs_data_get(evcs_ctx, &evcs_data)) return 1;

     /* Start monitoring loop */
     for (;;) {

	  /* Gather data and log power values */
	  if (evcs_data_get(evcs_ctx, &evcs_data) || gx_data_get(gx_ctx, &gx_data)) {
	       sleep(config.sleep_secs);
	       continue;
	  }

	  printf("Total: grid: %6dW, pv: %6dW, battery: %6dW, consumption: %6dW, evcs: %6dW\n",
		 gx_data.grid_power_total, gx_data.pv_power_total, gx_data.battery_power,
		 gx_data.consumption_power_total, evcs_data.power_total);

	  fflush(stdout);

	  /* Pace data collection to protect devices */
	  sleep(config.sleep_secs);
     }
}

bool config_from_env(struct config_t *config)
{
     const char *config_sleep_secs = getenv("SLEEP_SECS");
     if (config_sleep_secs == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "SLEEP_SECS");
	  return 1;
     }

     config->sleep_secs = (uint32_t)atoi(config_sleep_secs);
     if (config->sleep_secs == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "SLEEP_SECS");
	  return 1;
     }

     config->gx.host = getenv("GX_HOST");
     if (config->gx.host == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "GX_HOST");
	  return 1;
     }

     const char *gx_port_str = getenv("GX_PORT");
     if (gx_port_str == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "GX_PORT");
	  return 1;
     }
     config->gx.port = atoi(gx_port_str);
     if (config->gx.port == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "GX_PORT");
	  return 1;
     }

     config->evcs.host = getenv("EVCS_HOST");
     if (config->evcs.host == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "EVCS_HOST");
	  return 1;
     }

     const char *evcs_port_str = getenv("EVCS_PORT");
     if (evcs_port_str == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "EVCS_PORT");
	  return 1;
     }
     config->evcs.port = atoi(evcs_port_str);
     if (config->evcs.port == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "EVCS_PORT");
	  return 1;
     }

     return 0;
}
