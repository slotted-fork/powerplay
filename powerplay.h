#ifndef INCLUDE_POWERPLAY_H
#define INCLUDE_POWERPLAY_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <modbus/modbus.h>

/*
 *
 * GX
 *
 */

/* CCGX Modbus TCP register list 3.50 */
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

typedef enum {
     GX_SWITCH_CHARGER_ONLY				= 1,
     GX_SWITCH_INVERTER_ONLY				= 2,
     GX_SWITCH_ON					= 3,
     GX_SWITCH_OFF					= 4,
} gx_switch_t;

struct gx_data_t {
     int32_t battery_power;
     int32_t grid_power_total;
     int16_t grid_power_l1, grid_power_l2, grid_power_l3;
     int32_t pv_power_total;
     int16_t pv_power_l1, pv_power_l2, pv_power_l3;
     int32_t consumption_power_total;
     int16_t consumption_power_l1, consumption_power_l2, consumption_power_l3;
};

bool gx_data_get(modbus_t *ctx, struct gx_data_t *data);

/*
 *
 * EVCS
 *
 */

/* EVCS Modbus TCP register list 3.5 */
typedef enum {
     EVCS_REGISTER_CHARGE_MODE				= 5009, /* uint16_t */
     EVCS_REGISTER_CHARGE_START				= 5010, /* uint16_t */
     EVCS_REGISTER_TOTAL_POWER				= 5014, /* uint16_t */
     EVCS_REGISTER_CHARGER_STATUS			= 5015, /* uint16_t */
     EVCS_REGISTER_CHARGING_CURRENT			= 5016, /* uint16_t */
     EVCS_REGISTER_MAX_CURRENT				= 5017, /* uint16_t */
} evcs_register_t;

typedef enum {
     EVCS_CHARGE_MODE_MANUAL				= 0,
     EVCS_CHARGE_MODE_AUTO				= 1,
     EVCS_CHARGE_MODE_SCHED				= 2,
} evcs_charge_mode_t;

typedef enum {
    EVCS_CHARGER_STATUS_DISCONNECTED			= 0,
    EVCS_CHARGER_STATUS_CONNECTED			= 1,
    EVCS_CHARGER_STATUS_CHARGING			= 2,
    EVCS_CHARGER_STATUS_CHARGED				= 3,
    EVCS_CHARGER_STATUS_WAITING_FOR_SUN			= 4,
    EVCS_CHARGER_STATUS_WAITING_FOR_RFID		= 5,
    EVCS_CHARGER_STATUS_WAITING_FOR_START		= 6,
    EVCS_CHARGER_STATUS_LOW_SOC				= 7,
    EVCS_CHARGER_STATUS_GROUND_TEST_ERROR		= 8,
    EVCS_CHARGER_STATUS_WELDED_CONTACTS_ERROR		= 9,
    EVCS_CHARGER_STATUS_CP_INPUT_ERROR_SHORTED		= 10,
    EVCS_CHARGER_STATUS_RESIDUAL_CURRENT_DETECTED	= 11,
    EVCS_CHARGER_STATUS_UNDERVOLTAGE_DETECTED		= 12,
    EVCS_CHARGER_STATUS_OVERVOLTAGE_DETECTED		= 13,
    EVCS_CHARGER_STATUS_OVERHEATING_DETECTED		= 14,
    EVCS_CHARGER_STATUS_RESERVED_15			= 15,
    EVCS_CHARGER_STATUS_RESERVED_16			= 16,
    EVCS_CHARGER_STATUS_RESERVED_17			= 17,
    EVCS_CHARGER_STATUS_RESERVED_18			= 18,
    EVCS_CHARGER_STATUS_RESERVED_19			= 19,
    EVCS_CHARGER_STATUS_CHARGING_LIMIT			= 20,
    EVCS_CHARGER_STATUS_START_CHARGING			= 21,
    EVCS_CHARGER_STATUS_SWITCHING_TO_3_PHASE		= 22,
    EVCS_CHARGER_STATUS_SWITCHING_TO_1_PHASE		= 23,
    EVCS_CHARGER_STATUS_STOP_CHARGING			= 24,
} evcs_charger_status_t;

struct evcs_data_t {
     int32_t power_total;
     uint16_t charge_start;
     uint16_t charger_status;
     uint16_t charging_mode;
};

const char *get_watchdog_reason(uint16_t code);
const char *get_reset_reason(uint16_t code);
const char *get_charging_mode(uint16_t code);
const char *get_charger_status(uint16_t code);
bool evcs_data_get(modbus_t *ctx, struct evcs_data_t *data);

/*
 *
 * PowerPlay
 *
 */

struct modbus_device_t {
     const char *host;
     int port;
};

struct config_t {
     int32_t power_excess_min;
     time_t averaging_secs;
     uint32_t sleep_secs;
     int debug;
     struct modbus_device_t gx;
     struct modbus_device_t evcs;
};

bool modbus_device_connect(struct modbus_device_t device, modbus_t **ctx);

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

const char *get_charger_status(uint16_t code)
{
     switch (code) {
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

const char *get_charging_mode(uint16_t code)
{
     switch (code) {
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
	  fprintf(stderr, "Error: connection failed: %s\n", modbus_strerror(errno));
	  return 1;
     }

     return 0;
}

#endif

#endif
