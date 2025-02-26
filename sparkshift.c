#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <modbus/modbus.h>

#define POWERPLAY_IMPLEMENTATION
#include "powerplay.h"

bool config_from_env(struct config_t *config);

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


int main()
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

     printf("Charge setpoint: %dW\n"
	    "Averaged over: %lds\n"
	    "Loop sleep: %us\n"
	    "Charge mode: %s\n"
	    "Charge status: %s\n"
	    "Charge start: %u\n",
	    config.power_excess_min,
	    config.averaging_secs,
	    config.sleep_secs,
	    get_charging_mode(evcs_data.charging_mode),
	    get_charger_status(evcs_data.charger_status),
	    evcs_data.charge_start);
     fflush(stdout);

     /* Start control loop */

     /* Default to maintain the current charging setting */
     uint16_t evcs_should_charge = evcs_data.charge_start;
     uint16_t evcs_charger_status_previous = evcs_data.charger_status;

     time_t   start, current;
     int32_t  excess_acc = 0;
     int32_t  n = 0;

     time(&start);
     printf("Starting control loop\n");
     fflush(stdout);

     for (;;) {
	  time(&current);

	  /* Get data and calculate excess and accumulate */
	  if (evcs_data_get(evcs_ctx, &evcs_data) || gx_data_get(gx_ctx, &gx_data)) {
	       sleep(config.sleep_secs);
	       continue;
	  }

	  ++n;

	  int32_t excess = gx_data.battery_power + evcs_data.power_total - gx_data.grid_power_total;
	  excess_acc += excess;

	  /* Only make charging available if in a know good state to do so */
	  int can_charge = 0;
	  if (evcs_data.charging_mode == EVCS_CHARGE_MODE_AUTO
	      && (evcs_data.charger_status == EVCS_CHARGER_STATUS_CONNECTED
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_CHARGING
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_CHARGED
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_WAITING_FOR_SUN
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_WAITING_FOR_START
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_CHARGING_LIMIT
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_START_CHARGING
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_SWITCHING_TO_3_PHASE
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_SWITCHING_TO_1_PHASE
		  || evcs_data.charger_status == EVCS_CHARGER_STATUS_STOP_CHARGING)) {
	       can_charge = 1;
	  }

	  if(config.debug) {
	       printf("DEBUG: n: %3d, grid: %6d, battery: %6d, evcs: %6d, excess: %6d, "
		      "excess_acc: %6d, mean excess: %6d, can charge: %d\n",
		      n, gx_data.grid_power_total, gx_data.battery_power, evcs_data.power_total,
		      excess, excess_acc, excess_acc / n, can_charge);
	  }

	  /* Report change in charger status */
	  if (evcs_charger_status_previous != evcs_data.charger_status) {
	       printf("Charger status changed to: %s\n", get_charger_status(evcs_data.charger_status));
	       evcs_charger_status_previous = evcs_data.charger_status;
	  }

          /* Assess excess only if averaging secs have passed */
	  time_t elapsed_secs = current - start;
	  if (elapsed_secs >= config.averaging_secs) {
	       int32_t power_excess_mean = excess_acc / n;

	       printf("Excess mean (%lds): %6dW, charger status: %s, charging mode: %s, charge start: %u, can charge: %d\n",
		      elapsed_secs,power_excess_mean, get_charger_status(evcs_data.charger_status),
		      get_charging_mode(evcs_data.charging_mode), evcs_data.charge_start, can_charge);

	       if (power_excess_mean < config.power_excess_min) {
		    evcs_should_charge = 0;
	       } else {
		    evcs_should_charge = 1;
	       }

	       /* Start new averaging cycle */
	       n = 0;
	       excess_acc = 0;
	       time(&start);
	  }

	  /* Switch charge control if required */
	  if (can_charge && evcs_should_charge != evcs_data.charge_start) {
	       printf("Set charge start: %u\n", evcs_should_charge);
	       if (modbus_write_register(evcs_ctx, EVCS_REGISTER_CHARGE_START, evcs_should_charge) == -1) {
		    fprintf(stderr, "Error: could not set charging: %s\n", modbus_strerror(errno));
		    fflush(stderr);
	       }
	  }

	  fflush(stdout);

	  /* Pace data collection to protect devices */
	  sleep(config.sleep_secs);
     }
}

bool config_from_env(struct config_t *config)
{
     const char *config_debug = getenv("SPARKSHIFT_DEBUG");
     if (config_debug == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "SPARKSHIFT_DEBUG");
	  return 1;
     }

     if (!strcmp("1", config_debug)) {
	  config->debug = 1;
     }

     const char *config_averaging_secs = getenv("AVERAGING_SECS");
     if (config_averaging_secs == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "AVERAGING_SECS");
	  return 1;
     }

     config->averaging_secs = atoi(config_averaging_secs);
     if (config->averaging_secs == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "AVERAGING_SECS");
	  return 1;
     }

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

     const char *config_power_excess_min = getenv("POWER_EXCESS_MIN");
     if (config_power_excess_min == NULL) {
	  fprintf(stderr, "Error: %s environment variable not set\n", "POWER_EXCESS_MIN");
	  return 1;
     }

     config->power_excess_min = atoi(config_power_excess_min);
     if (config->power_excess_min == 0) {
	  fprintf(stderr, "Error: %s environment variable not an integer\n", "POWER_EXCESS_MIN");
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
