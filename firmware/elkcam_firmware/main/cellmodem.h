#ifndef _CELLMODEH_H_
#define _CELLMODEM_H_

#include "configstorage.h"

_Bool init_cellmodem(cell_config_t* cellConfig);
void close_cellmodem();

#endif