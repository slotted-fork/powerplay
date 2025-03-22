#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "powerplay.h"

/*
  Environment variables required for configuration

  GX_HOST		: IP address of GX device
  GX_PORT		: Modbus TCP port of GX device

  EVCS_HOST		: IP address of EVCS device
  ECVS_PORT		: Modbus TCP port of EVCS device

  POWER_EXCESS_MIN	: Minimum excess power to start charging

  AVERAGING_SECS	: Seconds to average excess power over
  SLEEP_SECS		: Seconds to sleep in control loop

 */


int main(void)
{
     struct system_status current = {0};
     if (config_from_env(&current.config)) return 1;

     if (modbus_device_connect(current.config.evcs, &current.evcs_ctx)) return 1;
     if (modbus_device_connect(current.config.gx, &current.gx_ctx)) return 1;

     /*

       Control loop

     */
     int64_t power_excess_accum = 0;
     int64_t rounds = 0;

     for(size_t i = 0;; ++i) {
	  int64_t power_excess_mean;

	  if (system_status_update(&current)) {
	       fprintf(stderr, "Error: failed to get system status\n");
	       goto error;
	  };

	  rounds += 1;
	  power_excess_accum += current.power_excess;
	  power_excess_mean = power_excess_accum / rounds;

	  printf("M/%c S/%c C/%d R/%4ld A/%7ld X/%7d G/%7d B/%7d P/%7d C/%7d E/%7d BS/%3d ES/%3d\n",
		 get_charging_mode_char(current.evcs_charging_mode),
		 get_charger_status_char(current.evcs_charger_status),
		 current.evcs_charge_start,
		 rounds,
		 power_excess_mean,
		 current.power_excess, current.power_grid, current.power_battery,
		 current.power_pv, current.power_consumption, current.power_evcs,
		 current.soc_battery, current.soc_ev);

	  if ((rounds * current.config.sleep_secs) >= current.config.averaging_secs) {
	       if (power_excess_mean > current.config.power_excess_min) {
		    if (evcs_charge_start_set(1)) {
			 fprintf(stderr, "Error: failed to set charge start to 1\n");
			 goto error;
		    };
	       } else {
		    if (evcs_charge_start_set(0)) {
			 fprintf(stderr, "Error: failed to set charge start to 0\n");
			 goto error;
		    };
	       }
	       power_excess_accum = 0;
	       rounds = 0;
	  }

     loop:
	  fflush(stdout);
	  sleep(current.config.sleep_secs);
	  continue;

     error:
	  fflush(stderr);
	  goto loop;
     }
}
