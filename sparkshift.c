#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "powerplay.h"

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
     struct system_status current = {0};
     if (config_from_env(&current.config)) return 1;

     if (modbus_device_connect(current.config.evcs, &current.evcs_ctx)) return 1;
     if (modbus_device_connect(current.config.gx, &current.gx_ctx)) return 1;

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
		 current.power_excess,
		 current.power_grid, current.power_pv, current.power_consumption,
		 current.power_evcs, current.power_battery);
	  fflush(stdout);

     loop:
	  sleep(current.config.sleep_secs);
     }
}
