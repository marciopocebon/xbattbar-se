/* See LICENSE file for copyright and license details. */

/* Polling interval in seconds */
static int bi_interval	= 10;

/* Battery indicator thickness in pixels */
static int bi_thick	= 3;

/* Status bar location */
/* BI_Bottom
 * BI_Top
 * BI_Left
 * BI_Right
 */
static int bi_direction = BI_Bottom;

/* Battery indicator default colors */
static char *ONIN_C	= "green";
static char *ONOUT_C	= "olive drab";
static char *OFFIN_C	= "blue";
static char *OFFOUT_C	= "red";

/* Show battery indicator always on top */
static int always_on_top = True;

/* Battery, UPS, AC, DC power supply properties.
 * see: Documentation/power/power_supply_class.txt
 * */
#define BattFull	"/sys/class/power_supply/BAT0/energy_full"
#define BattNow		"/sys/class/power_supply/BAT0/energy_now"
#define PowerState	"/sys/class/power_supply/ADP1/online"

/* vim: set cc=80 : sw=8 : ts=8 : et
 * End of file
 * */
