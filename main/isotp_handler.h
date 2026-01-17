#ifndef ISOTP_HANDLER_H
#define ISOTP_HANDLER_H

#include <stdint.h>

/* Callback: Gửi một khung CAN */
int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t* data, const uint8_t size);

/* Callback: Lấy thời gian hệ thống (miligiây) */
uint32_t isotp_user_get_ms(void);

/* Callback: Debug */
void isotp_user_debug(const char* message, ...);

#endif // ISOTP_HANDLER_H
