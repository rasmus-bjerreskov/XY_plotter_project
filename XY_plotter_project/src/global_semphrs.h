#ifndef GLOBAL_SEMPHRS_H_
#define GLOBAL_SEMPHRS_H_
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"

//signalling to pen task to update value
extern SemaphoreHandle_t binPen;
//Event group for task synchronisation
extern EventGroupHandle_t eGrp;
#define USB_CDC_b 	1 << 0
#define RX_b 		1 << 1
#define PLOT_b 		1 << 2
#define PEN_b		1 << 3
#define TX_b		1 << 4
#define TASK_BITS	(USB_CDC_b | RX_b | PLOT_b | PEN_b | TX_b)

#endif
