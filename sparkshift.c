#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sparkshift.h"

/*
 *  Environment variables required for configuration
 *
 *  GX_HOST		: IP address of GX device
 *  GX_PORT		: Modbus TCP port of GX device
 *
 *  EVCS_HOST		: IP address of EVCS device
 *  ECVS_PORT		: Modbus TCP port of EVCS device
 *
 *  POWER_EXCESS_MIN	: Minimum excess power to start charging
 *
 *  AVERAGING_SECS	: Seconds to average excess power over
 *  SLEEP_SECS		: Seconds to sleep in control loop
 *
 */

int main(void)
{
     struct config_t config = {0};
     struct system_status current = {0};

     if (config_from_env(&config)) return 1;
     if (modbus_device_connect(config.evcs, &current.evcs_ctx)) return 1;
     if (modbus_device_connect(config.gx, &current.gx_ctx)) return 1;

     if (modbus_set_slave(current.gx_ctx, 100) == -1) { /* com.victronenergy.system */
	  fprintf(stderr, "Error: setting modbus slave for GX failed: %s\n", modbus_strerror(errno));
	  return 1;
     }

     /*

       Control loop

     */
     for(size_t i = 0;; ++i) {

	  /* If gathering of system status fails, ignore and loop again */
	  if (system_status_update(&current)) goto loop;

	  printf("M/%c S/%c C/%d X/%7d G/%7d P/%7d C/%7d E/%7d B/%7d \n",
		 get_charging_mode(current.evcs_charging_mode),
		 get_charger_status(current.evcs_charger_status),
		 current.evcs_charge_start,
		 (current.power_pv - current.power_battery + current.power_grid),
		 current.power_grid, current.power_pv, current.power_consumption,
		 current.power_evcs, current.power_battery);
	  fflush(stdout);

     loop:
	  sleep(config.sleep_secs);
     }
}

int config_from_env(struct config_t *config)
{
     const char *config_debug = getenv("SPARKSHIFT_DEBUG");
     if (config_debug == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "SPARKSHIFT_DEBUG");
	  return -1;
     }

     if (!strcmp("1", config_debug)) {
	  config->debug = 1;
     }

     const char *config_averaging_secs = getenv("AVERAGING_SECS");
     if (config_averaging_secs == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "AVERAGING_SECS");
	  return -1;
     }

     config->averaging_secs = atoi(config_averaging_secs);
     if (config->averaging_secs == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "AVERAGING_SECS");
	  return -1;
     }

     const char *config_sleep_secs = getenv("SLEEP_SECS");
     if (config_sleep_secs == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "SLEEP_SECS");
	  return -1;
     }

     config->sleep_secs = (uint32_t)atoi(config_sleep_secs);
     if (config->sleep_secs == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "SLEEP_SECS");
	  return -1;
     }

     const char *config_power_excess_min = getenv("POWER_EXCESS_MIN");
     if (config_power_excess_min == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "POWER_EXCESS_MIN");
	  return -1;
     }

     config->power_excess_min = atoi(config_power_excess_min);
     if (config->power_excess_min == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "POWER_EXCESS_MIN");
	  return -1;
     }

     config->gx.host = getenv("GX_HOST");
     if (config->gx.host == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "GX_HOST");
	  return -1;
     }

     const char *gx_port_str = getenv("GX_PORT");
     if (gx_port_str == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "GX_PORT");
	  return -1;
     }
     config->gx.port = atoi(gx_port_str);
     if (config->gx.port == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "GX_PORT");
	  return -1;
     }

     config->evcs.host = getenv("EVCS_HOST");
     if (config->evcs.host == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "EVCS_HOST");
	  return -1;
     }

     const char *evcs_port_str = getenv("EVCS_PORT");
     if (evcs_port_str == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "EVCS_PORT");
	  return -1;
     }
     config->evcs.port = atoi(evcs_port_str);
     if (config->evcs.port == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "EVCS_PORT");
	  return -1;
     }

     return 0;
}
