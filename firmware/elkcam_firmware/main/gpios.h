#ifndef _GPIOS_H_
#define _GPIOS_H_

#define CAM_LED_BLUE (0)
#define CAM_LED_RED (1)

void init_gpios();
void switch_led(int led, int on);
void power_cam(int power);
void power_radio(int power);
void powerkey_radio(int on);




#endif