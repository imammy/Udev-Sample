#pragma once
#include <sys/types.h>

typedef void (*ReceiveDataCallback_t)(u_int8_t *p_data, u_int32_t len, void *p_context);

void Uart_Init(ReceiveDataCallback_t callback);
void Uart_Device_Load(u_int8_t *special_file, u_int32_t len);
void Uart_Device_Unload(void);