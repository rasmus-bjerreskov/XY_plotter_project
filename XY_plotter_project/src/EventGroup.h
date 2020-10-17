#ifndef EVENTGROUP_H_
#define EVENTGROUP_H_

//Event group for task synchronisation
extern EventGroupHandle_t eGrp;
#define USB_CDC_b 	1 << 0
#define RX_b 		1 << 1
#define PLOT_b 		1 << 2
#define TX_b		1 << 3
#define TASK_BITS	(USB_CDC_b | RX_b | PLOT_b | TX_b)

#endif
