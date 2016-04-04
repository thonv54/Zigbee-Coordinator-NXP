/*
 * UART.h
 *
 *  Created on: Apr 1, 2016
 *      Author: hp8460p
 */

#ifndef UART_H_
#define UART_H_

#include <os.h>

OS_ISR(Uart0GetData);
void InitUart(void);

#endif /* UART_H_ */
