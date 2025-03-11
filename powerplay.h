#ifndef POWERPLAY_H
#define POWERPLAY_H

/*
 * powerplay.h - PowerPlay energy management system for Victron Energy ecosystem
 *
 * This library provides interfaces for communication with Victron GX devices
 * and EVCS (Electric Vehicle Charging Station) via Modbus TCP.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <modbus/modbus.h>

/*
 *
 * GX
 *
 */

/*
 * Modbus register addresses for the GX device
 *
 * Based on CCGX Modbus TCP register list version 3.50
 */
typedef enum {
     GX_REGISTER_SWITCH_POSITION			= 33,  /* uint16, com.victronenergy.vebus */
     GX_REGISTER_PV_AC_IN_L1				= 811, /* uint16, com.victronenergy.system */
     GX_REGISTER_PV_AC_IN_L2				= 812, /* uint16, com.victronenergy.system */
     GX_REGISTER_PV_AC_IN_L3				= 813, /* uint16, com.victronenergy.system */
     GX_REGISTER_AC_CONSUMPTION_L1			= 817, /* uint16, com.victronenergy.system */
     GX_REGISTER_AC_CONSUMPTION_L2			= 818, /* uint16, com.victronenergy.system */
     GX_REGISTER_AC_CONSUMPTION_L3			= 819, /* uint16, com.victronenergy.system */
     GX_REGISTER_GRID_L1				= 820, /* int16,  com.victronenergy.system */
     GX_REGISTER_GRID_L2				= 821, /* int16,  com.victronenergy.system */
     GX_REGISTER_GRID_L3				= 822, /* int16,  com.victronenergy.system */
     GX_REGISTER_BATTERY_POWER				= 842, /* int16,  com.victronenergy.system */
} gx_register_t;

/*
 * GX switch position control values
 *
 * Values used to control the operating mode of the GX device
 */
typedef enum {
     GX_SWITCH_CHARGER_ONLY				= 1, /* Device operates in charger-only mode */
     GX_SWITCH_INVERTER_ONLY				= 2, /* Device operates in inverter-only mode */
     GX_SWITCH_ON					= 3, /* Device is fully on (both charging and inverting) */
     GX_SWITCH_OFF					= 4, /* Device is turned off */
} gx_switch_t;

/*
 * Data structure for GX device power information
 *
 * Contains power data from various sources including grid, PV, and consumption
 * values across three phases, as well as battery power and calculated totals.
 */
struct gx_data_t {
     int32_t battery_power;              /* Battery power in watts (positive = charging) */
     int32_t grid_power_total;           /* Total grid power across all phases in watts */
     int16_t grid_power_l1;              /* Grid power phase 1 in watts (positive = import) */
     int16_t grid_power_l2;              /* Grid power phase 2 in watts (positive = import) */
     int16_t grid_power_l3;              /* Grid power phase 3 in watts (positive = import) */
     int32_t pv_power_total;             /* Total PV (solar) power across all phases in watts */
     int16_t pv_power_l1;                /* PV power phase 1 in watts */
     int16_t pv_power_l2;                /* PV power phase 2 in watts */
     int16_t pv_power_l3;                /* PV power phase 3 in watts */
     int32_t consumption_power_total;    /* Total power consumption across all phases in watts */
     int16_t consumption_power_l1;       /* Power consumption phase 1 in watts */
     int16_t consumption_power_l2;       /* Power consumption phase 2 in watts */
     int16_t consumption_power_l3;       /* Power consumption phase 3 in watts */
};

/*
 * Retrieves data from a GX device via Modbus
 *
 * Reads multiple registers from the GX device to gather power information
 * including grid power, PV (solar) power, battery status, and consumption data.
 *
 * ctx: Initialized Modbus context connected to a GX device
 * data: Pointer to a gx_data_t structure to populate with the retrieved data
 * return: true on error, false on success
 */
bool gx_data_get(modbus_t *ctx, struct gx_data_t *data);

/*
 *
 * EVCS
 *
 */

/*
 * Modbus register addresses for the EVCS device
 *
 * Based on EVCS Modbus TCP register list version 3.5
 */
typedef enum {
     EVCS_REGISTER_CHARGE_MODE				= 5009, /* Charging mode (manual/auto/scheduled) */
     EVCS_REGISTER_CHARGE_START				= 5010, /* Start/stop charging flag */
     EVCS_REGISTER_TOTAL_POWER				= 5014, /* Total power consumption in watts */
     EVCS_REGISTER_CHARGER_STATUS			= 5015, /* Current status of the charger */
     EVCS_REGISTER_CHARGING_CURRENT			= 5016, /* Current charging current in 0.1A */
     EVCS_REGISTER_MAX_CURRENT				= 5017, /* Maximum allowed charging current in 0.1A */
} evcs_register_t;

/*
 * EVCS charging mode options
 *
 * Defines the different modes for controlling the EV charging process
 */
typedef enum {
     EVCS_CHARGE_MODE_MANUAL				= 0, /* Manual mode - user controls charging */
     EVCS_CHARGE_MODE_AUTO				= 1, /* Automatic mode - system controls charging */
     EVCS_CHARGE_MODE_SCHED				= 2, /* Scheduled mode - charging follows a time schedule */
} evcs_charge_mode_t;

/*
 * EVCS charger status codes
 *
 * Defines all possible states of the EV charging station
 * including normal operation, waiting conditions, and error states
 */
typedef enum {
    EVCS_CHARGER_STATUS_DISCONNECTED			= 0,  /* No vehicle connected */
    EVCS_CHARGER_STATUS_CONNECTED			= 1,  /* Vehicle connected but not charging */
    EVCS_CHARGER_STATUS_CHARGING			= 2,  /* Vehicle actively charging */
    EVCS_CHARGER_STATUS_CHARGED				= 3,  /* Vehicle fully charged */
    EVCS_CHARGER_STATUS_WAITING_FOR_SUN			= 4,  /* Waiting for solar power availability */
    EVCS_CHARGER_STATUS_WAITING_FOR_RFID		= 5,  /* Waiting for RFID authentication */
    EVCS_CHARGER_STATUS_WAITING_FOR_START		= 6,  /* Waiting for start command */
    EVCS_CHARGER_STATUS_LOW_SOC				= 7,  /* Vehicle battery state of charge too low */
    EVCS_CHARGER_STATUS_GROUND_TEST_ERROR		= 8,  /* Error in ground connection test */
    EVCS_CHARGER_STATUS_WELDED_CONTACTS_ERROR		= 9,  /* Welded contacts detected */
    EVCS_CHARGER_STATUS_CP_INPUT_ERROR_SHORTED		= 10, /* Control Pilot input error (shorted) */
    EVCS_CHARGER_STATUS_RESIDUAL_CURRENT_DETECTED	= 11, /* Residual current detected (safety) */
    EVCS_CHARGER_STATUS_UNDERVOLTAGE_DETECTED		= 12, /* Input voltage too low */
    EVCS_CHARGER_STATUS_OVERVOLTAGE_DETECTED		= 13, /* Input voltage too high */
    EVCS_CHARGER_STATUS_OVERHEATING_DETECTED		= 14, /* Charger temperature too high */
    EVCS_CHARGER_STATUS_RESERVED_15			= 15, /* Reserved for future use */
    EVCS_CHARGER_STATUS_RESERVED_16			= 16, /* Reserved for future use */
    EVCS_CHARGER_STATUS_RESERVED_17			= 17, /* Reserved for future use */
    EVCS_CHARGER_STATUS_RESERVED_18			= 18, /* Reserved for future use */
    EVCS_CHARGER_STATUS_RESERVED_19			= 19, /* Reserved for future use */
    EVCS_CHARGER_STATUS_CHARGING_LIMIT			= 20, /* Charging at power limit */
    EVCS_CHARGER_STATUS_START_CHARGING			= 21, /* Starting the charging process */
    EVCS_CHARGER_STATUS_SWITCHING_TO_3_PHASE		= 22, /* Switching to 3-phase charging */
    EVCS_CHARGER_STATUS_SWITCHING_TO_1_PHASE		= 23, /* Switching to 1-phase charging */
    EVCS_CHARGER_STATUS_STOP_CHARGING			= 24, /* Stopping the charging process */
} evcs_charger_status_t;

/*
 * Data structure for EVCS (Electric Vehicle Charging Station) information
 *
 * Contains charging status, mode, and power usage data from the EVCS device.
 */
struct evcs_data_t {
     int32_t power_total;     /* Total power consumption of the charger in watts */
     uint16_t charge_start;   /* Charge start flag (1 = start charging, 0 = stop) */
     uint16_t charger_status; /* Current status of the charger (see evcs_charger_status_t) */
     uint16_t charging_mode;  /* Current charging mode (see evcs_charge_mode_t) */
};

/*
 * Converts EVCS charger status code to a human-readable string
 *
 * status: Status code from the EVCS device
 * return: String representation of the charger status
 */
const char *get_charger_status(evcs_charger_status_t status);

/*
 * Converts EVCS charging mode code to a human-readable string
 *
 * mode: Charging mode code from the EVCS device
 * return: String representation of the charging mode
 */
const char *get_charging_mode(evcs_charge_mode_t mode);

/*
 * Converts watchdog reason code to a human-readable string
 *
 * code: Watchdog reason code from the device
 * return: String representation of the watchdog reason
 */
const char *get_watchdog_reason(uint16_t code);

/*
 * Converts reset reason code to a human-readable string
 *
 * code: Reset reason code from the device
 * return: String representation of the reset reason
 */
const char *get_reset_reason(uint16_t code);

/*
 * Retrieves data from an EVCS device via Modbus
 *
 * Reads power, charging status, and mode information from the EVCS.
 *
 * ctx: Initialized Modbus context connected to an EVCS device
 * data: Pointer to an evcs_data_t structure to populate with retrieved data
 * return: true on error, false on success
 */
bool evcs_data_get(modbus_t *ctx, struct evcs_data_t *data);

/*
 *
 * PowerPlay
 *
 */

/*
 * Connection details for a Modbus TCP device
 */
struct modbus_device_t {
     const char *host;  /* Hostname or IP address of the Modbus device */
     int port;          /* TCP port number for the Modbus connection */
};

/*
 * Configuration for PowerPlay operation
 *
 * Contains parameters for controlling the charging behavior,
 * connection details for devices, and debug options.
 */
struct config_t {
     int32_t power_excess_min;     /* Minimum excess power (watts) to start charging */
     time_t averaging_secs;        /* Seconds to average excess power over */
     uint32_t sleep_secs;          /* Seconds to sleep in control loop */
     int debug;                    /* Debug level (0 = off, higher values = more verbose) */
     struct modbus_device_t gx;    /* Connection details for the GX device */
     struct modbus_device_t evcs;  /* Connection details for the EVCS device */
};

/*
 * Establishes a Modbus TCP connection to a device
 *
 * Creates a new Modbus context, configures timeouts and error recovery,
 * and connects to the specified host and port.
 *
 * device: Connection details including host and port
 * ctx: Pointer to a modbus_t pointer that will be set to the new context
 * return: true on error, false on success
 */
bool modbus_device_connect(struct modbus_device_t device, modbus_t **ctx);

/*
 * POWERPLAY_IMPLEMENTATION
 *
 * This macro implements a single-header library pattern.
 * Define POWERPLAY_IMPLEMENTATION in exactly one source file
 * before including this header to include the implementation.
 * Other files should include the header without defining the macro
 * to only include the declarations.
 */
#ifdef POWERPLAY_IMPLEMENTATION

/*
 *
 * GX
 *
 */

bool gx_data_get(modbus_t *ctx, struct gx_data_t *data) {
     int16_t battery_power;

     if (   (modbus_read_registers(ctx, GX_REGISTER_GRID_L1, 1, (uint16_t *)&data->grid_power_l1) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_GRID_L2, 1, (uint16_t *)&data->grid_power_l2) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_GRID_L3, 1, (uint16_t *)&data->grid_power_l3) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_PV_AC_IN_L1, 1, (uint16_t *)&data->pv_power_l1) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_PV_AC_IN_L2, 1, (uint16_t *)&data->pv_power_l2) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_PV_AC_IN_L3, 1, (uint16_t *)&data->pv_power_l3) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_AC_CONSUMPTION_L1, 1, (uint16_t *)&data->consumption_power_l1) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_AC_CONSUMPTION_L2, 1, (uint16_t *)&data->consumption_power_l2) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_AC_CONSUMPTION_L3, 1, (uint16_t *)&data->consumption_power_l3) == -1)
	 || (modbus_read_registers(ctx, GX_REGISTER_BATTERY_POWER, 1, (uint16_t *)&battery_power) == -1)) {
	  fprintf(stderr, "Error: could not read GX value: %s\n", modbus_strerror(errno));
	  fflush(stderr);

	  return 1;
     }

     data->battery_power = (int32_t)battery_power;
     data->grid_power_total =
	  (int32_t)data->grid_power_l1
	  + (int32_t)data->grid_power_l2
	  + (int32_t)data->grid_power_l3;
     data->pv_power_total =
	  (int32_t)data->pv_power_l1
	  + (int32_t)data->pv_power_l2
	  + (int32_t)data->pv_power_l3;
     data->consumption_power_total =
	  (int32_t)data->consumption_power_l1
	  + (int32_t)data->consumption_power_l2
	  + (int32_t)data->consumption_power_l3;

     return 0;
}

/*
 *
 * EVCS
 *
 */

const char *get_charger_status(evcs_charger_status_t status)
{
     switch (status) {
     case 0:
	  return "Disconnected";
     case 1:
	  return "Connected";
     case 2:
	  return "Charging";
     case 3:
	  return "Charged";
     case 4:
	  return "Waiting for sun";
     case 5:
	  return "Waiting for RFID";
     case 6:
	  return "Waiting for start";
     case 7:
	  return "Low SOC";
     case 8:
	  return "Ground test error";
     case 9:
	  return "Welded contacts test error";
     case 10:
	  return "CP input test error (shorted)";
     case 11:
	  return "Residual current detected";
     case 12:
	  return "Undervoltage detected";
     case 13:
	  return "Overvoltage detected";
     case 14:
	  return "Overheating detected";
     case 15:
     case 16:
     case 17:
     case 18:
     case 19:
	  return "Reserved";
     case 20:
	  return "Charging limit";
     case 21:
	  return "Start charging";
     case 22:
	  return "Switching to 3 phase";
     case 23:
	  return "Switching to 1 phase";
     case 24:
	  return "Stop charging";
     default:
	  return "Unknown state";
     }
}

const char *get_charging_mode(evcs_charge_mode_t mode)
{
     switch (mode) {
     case 0:
	  return "Manual";
     case 1:
	  return "Auto";
     case 2:
	  return "Scheduled";
     default:
	  return "Unknown mode";
     }
}

const char *get_reset_reason(uint16_t code)
{
     switch (code) {
     case 0:
	  return "Reset reason can not be determined";
     case 1:
	  return "Reset due to power-on event";
     case 2:
	  return "Reset by external pin";
     case 3:
	  return "Software reset via reboot function";
     case 4:
	  return "Software reset due to exception/panic";
     case 5:
	  return "Reset (software or hardware) due to interrupt watchdog";
     case 6:
	  return "Reset due to task watchdog";
     case 7:
	  return "Reset due to other watchdogs";
     case 8:
	  return "Reset after exiting deep sleep mode";
     case 9:
	  return "Brownout reset (software or hardware)";
     case 10:
	  return "Reset over SDIO";
     default:
	  return "Unknown reset reason";
     }
}

const char *get_watchdog_reason(uint16_t code)
{
     switch (code) {
     case 0:
	  return "Reset reason can not be determined";
     case 1:
	  return "Can't connect to desired WiFi access point";
     case 2:
	  return "Critically low memory amount";
     case 3:
	  return "Critical memory fragmentation";
     case 4:
	  return "Memory allocation failed";
     case 5:
	  return "Ping failed";
     default:
	  return "Unknown watchdog reason";
     }
}

bool evcs_data_get(modbus_t *ctx, struct evcs_data_t *data) {
     uint16_t power_total;
     if ((modbus_read_registers(ctx, EVCS_REGISTER_TOTAL_POWER, 1, &power_total) == -1)
	 || (modbus_read_registers(ctx, EVCS_REGISTER_CHARGE_START, 1, &data->charge_start) == -1)
	 || (modbus_read_registers(ctx, EVCS_REGISTER_CHARGER_STATUS, 1, &data->charger_status) == -1)
	 || (modbus_read_registers(ctx, EVCS_REGISTER_CHARGE_MODE, 1, &data->charging_mode) == -1)) {
	  fprintf(stderr, "Error: could not read EVCS value: %s\n", modbus_strerror(errno));
	  fflush(stderr);

	  return 1;
     }

     data->power_total = (int32_t)power_total;

     return 0;
}

/*
 *
 * Powerplay
 *
 */

bool modbus_device_connect(struct modbus_device_t device, modbus_t **ctx) {
     *ctx = modbus_new_tcp(device.host, device.port);
     if (*ctx == NULL) {
	  fprintf(stderr, "Error: Failed to create modbus context: %s\n", modbus_strerror(errno));
	  return 1;
     }

     if (modbus_set_response_timeout(*ctx, 5, 0) == -1) {
	  fprintf(stderr, "Error: failed to set timeout: %s\n", modbus_strerror(errno));
	  return 1;
     }

     if (modbus_set_error_recovery(*ctx, MODBUS_ERROR_RECOVERY_LINK|MODBUS_ERROR_RECOVERY_PROTOCOL) == -1) {
	  fprintf(stderr, "Error: failed to set error recovery: %s\n", modbus_strerror(errno));
	  return 1;
     }

     if (modbus_connect(*ctx) == -1) {
	  fprintf(stderr, "Error: connection failed to %s:%d: %s\n",
		  device.host, device.port, modbus_strerror(errno));
	  return 1;
     }

     return 0;
}

#endif /* POWERPLAY_IMPLEMENTATION */

#endif /* POWERPLAY_H */
