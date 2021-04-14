#ifndef _CONFIGSTORAGE_CONST_H_
#define _CONFIGSTORAGE_CONST_H_

/* jpeg storage quality, integer 0 - 63 */
#define CONFIGSTORAGE_KEY_CAM_QUALITY "cam_quality"

/* camera auto exposure setting, integer 0 - 4 */
#define CONFIGSTORAGE_KEY_CAM_AUTO_EXPOSURE "cam_exposure"

/* camera light mode, integer 0 - 4 */
/* 0: auto, 1: sunny, 2: cloudy, 3: office, 4: home */
#define CONFIGSTORAGE_KEY_CAM_LIGHT_MODE "cam_light"

/* camera color saturation, integer 0 - 4 */
/* 0: -2, 1: -1, 2:  0, 3: +1, 4: +2 */
#define CONFIGSTORAGE_KEY_CAM_COLOR_SATURATION "cam_col_sat"

/* camera brightness, integer 0 - 4 */
/* 0: -2, 1: -1, 2:  0, 3: +1, 4: +2 */
#define CONFIGSTORAGE_KEY_CAM_BRIGHTNESS "cam_brightnss"

/* camera contrast, integer 0 - 4 */
/* 0: -2, 1: -1, 2:  0, 3: +1, 4: +2 */
#define CONFIGSTORAGE_KEY_CAM_CONTRAST "cam_contrast"

/* cell phone pin, string */
/* can be set to an arbitary string if no pin is required by sim */
#define CONFIGSTORAGE_KEY_CELL_PIN "cell_pin"

/* cell phone data access apn, string */
#define CONFIGSTORAGE_KEY_CELL_APN "cell_apn"

/* cell phone apn user name, string */
/* can be set to an arbitary value if no user name is required and */
/* CONFIGSTORAGE_KEY_CELL_APN_AUTH is 0 */
#define CONFIGSTORAGE_KEY_CELL_APN_USER "cell_apn_user"

/* cell phone apn password, string */
/* can be set to an arbitary value if no password is required and */
/* CONFIGSTORAGE_KEY_CELL_APN_AUTH is 0 */
#define CONFIGSTORAGE_KEY_CELL_APN_PASS "cell_apn_pass"

/* apn authentication required by operator, integer 0 - 1 */
#define CONFIGSTORAGE_KEY_CELL_APN_AUTH "cell_apn_auth"

/* remote host name, sent in http host reader, string */
#define CONFIGSTORAGE_KEY_CELL_REMOTE_ADDRESS "cell_rem_adr"

/* complete remote url, used to access the upload facility, string */
#define CONFIGSTORAGE_KEY_CELL_REMOTE_URL "cell_rem_url"

/* default time between photos in seconds. Can be temporarily overwritten */
/* by server, integer */
#define CONFIGSTORAGE_KEY_SYS_SECS_BETWEEN_PHOTOS "sys_t_btw_phts"

/* photos taken. This cannot be reset by firmware, integer */
#define CONFIGSTORAGE_KEY_SYS_PHOTO_COUNTER "sys_p_counter"

#endif