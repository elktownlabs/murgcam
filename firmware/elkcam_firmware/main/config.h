#ifndef _CONFIG_H_
#define _CONFIG_H_

#define ELKCAM_SW_VERSION "0.0.4"

/* wifi */
#define CAM_CONFIG_SSID "elkcam"

/* gsm net access */
#define CAM_GSM_PIN "7190"


/* default picture settings */
#define CAM_DEFAULT_QUALITY (10)  /* 0 - 63 */
#define CAM_DEFAULT_AUTO_EXPOSURE (2)
#define CAM_DEFAULT_LIGHT_MODE (0)
#define CAM_DEFAULT_COLOR_SATURATION (0)
#define CAM_DEFAULT_BRIGHTNESS (0)
#define CAM_DEFAULT_CONTRAST (0)
#define CAM_DEFAULT_HUE (0)
#define CAM_DEFAULT_SHARPNESS (0)

/* default cell service settings */
#define CAM_CELL_PIN "6400"
#define CAM_CELL_APN "internet.telekom"
#define CAM_CELL_APN_USER "congstar"
#define CAM_CELL_APN_PASS "cs"
#define CAM_CELL_APN_AUTH 0x01 /*PAP*/
#define CAM_CELL_REMOTE_ADDRESS "wwv-schwarzwald.de"
#define CAM_CELL_REMOTE_URL "https://wwv-schwarzwald.de/webcam/api/upload"

/* default system settings */
#define SYS_DEFAULT_SECS_BETWEEN_PHOTOS (30*60)


#endif