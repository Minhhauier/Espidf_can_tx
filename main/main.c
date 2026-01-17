#include <stdio.h>
#include "driver/twai.h"
#include "string.h"

// Cấu trúc để định nghĩa thông tin gói tin CAN
typedef struct {
    uint32_t id;              // ID của tin nhắn CAN
    uint8_t data[8];          // Dữ liệu (tối đa 8 byte)
    uint8_t data_length;      // Số byte dữ liệu
    bool is_extended_id;      // Sử dụng ID mở rộng (29-bit)
} can_message_t;

/**
 * Khởi tạo bộ điều khiển CAN
 * @return ESP_OK nếu thành công, ngược lại trả về mã lỗi
 */
esp_err_t can_init(void)
{
    // Cấu hình pins CAN (TX: GPIO21, RX: GPIO22 cho ESP32)
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(27, 36, TWAI_MODE_NORMAL);
    
    // Cấu hình timing - 500kbps baud rate
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    
    // Cấu hình bộ lọc để nhận tất cả các gói tin
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Cài đặt driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        printf("Lỗi: Không thể cài đặt driver CAN\n");
        return ESP_FAIL;
    }

    // Khởi động driver
    if (twai_start() != ESP_OK) {
        printf("Lỗi: Không thể khởi động CAN\n");
        return ESP_FAIL;
    }

    printf("CAN được khởi tạo thành công\n");
    return ESP_OK;
}

/**
 * Hàm truyền gói tin CAN
 * @param msg Con trỏ đến cấu trúc chứa thông tin gói tin
 * @return ESP_OK nếu truyền thành công, ngược lại trả về mã lỗi
 */
esp_err_t can_send_message(can_message_t *msg)
{
    if (msg == NULL) {
        printf("Lỗi: Thông tin gói tin không hợp lệ\n");
        return ESP_ERR_INVALID_ARG;
    }

    // Xây dựng frame CAN
    twai_message_t tx_frame = (twai_message_t){0};
    tx_frame.identifier = msg->id;
    tx_frame.extd = msg->is_extended_id;   // Đặt loại ID
    tx_frame.rtr = false;                   // Không phải Remote Frame
    tx_frame.ss = false;                    // Single shot mode
    tx_frame.data_length_code = msg->data_length;  // Số byte dữ liệu

    // Copy dữ liệu vào frame
    for (int i = 0; i < msg->data_length && i < 8; i++) {
        tx_frame.data[i] = msg->data[i];
    }

    // Truyền frame
    esp_err_t result = twai_transmit(&tx_frame, pdMS_TO_TICKS(1000));
    
    // if (result == ESP_OK) {
    //     printf("✓ Gói tin CAN được truyền thành công (ID: 0x%lX, DLC: %d)\n", 
    //            msg->id, msg->data_length);
    // } else if (result == ESP_ERR_TIMEOUT) {
    //     printf("✗ Timeout: Không thể truyền gói tin CAN\n");
    // } else {
    //     printf("✗ Lỗi khi truyền gói tin CAN\n");
    // }

    return result;
}

/**
 * Hàm truyền nhiều bytes qua CAN (tự động chia thành nhiều gói tin nếu cần)
 * @param id CAN ID
 * @param data Con trỏ đến dữ liệu cần truyền
 * @param length Số bytes cần truyền
 * @param is_extended_id Sử dụng ID mở rộng hay không
 */
void can_write_bytes(uint32_t id, uint8_t *data, uint8_t length, bool is_extended_id)
{
    int index = 0;
    int remaining = length;
    while(remaining > 0)
    {
        int len = (remaining > 8) ? 8 : remaining;   
        can_message_t msg = {
            .id = id,
            .data_length = len,
            .is_extended_id = is_extended_id
        };
        memcpy(msg.data, &data[index], len);
        can_send_message(&msg);
        index += len;
        remaining -= len;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void app_main(void)
{
    // Khởi tạo CAN
    if (can_init() != ESP_OK) {
        printf("Lỗi khởi tạo CAN\n");
        return;
    }
    // Ví dụ: Tạo và truyền gói tin CAN
    can_message_t msg = {
        .id = 0x123,              // ID CAN (11-bit)
        .data_length = strlen("okee"),         // 8 bytes
        .is_extended_id = false   // Sử dụng ID tiêu chuẩn (11-bit)
    };
    memcpy(msg.data, "okee", msg.data_length);
    // Gửi gói tin
    can_send_message(&msg);

    // Có thể gửi thêm các gói tin khác
    vTaskDelay(pdMS_TO_TICKS(100));

    can_message_t msg2 = {
        .id = 0x456,
        .data_length = strlen("Test can"),
        .is_extended_id = false
    };
    memcpy(msg2.data, "Test can", msg2.data_length);

    can_send_message(&msg2);
    vTaskDelay(pdMS_TO_TICKS(1000));
    can_message_t msg3 = {
        .id = 0x1ABCDE,          // ID CAN (29-bit)
        .data_length = strlen("Hello"),
        .is_extended_id = true    // Sử dụng ID mở rộng (29-bit)
    };
    memcpy(msg3.data, "Hello", msg3.data_length);
    can_send_message(&msg3);
    vTaskDelay(pdMS_TO_TICKS(1000));
    //== Test gửi nhiều bytes
//    / printf("Test gửi nhiều bytes qua CAN\n");
  //  can_write_bytes(0x789, (uint8_t *)"This is a longer string to test sending multiple CAN frames.", 76, false);
   // printf("Đã gửi\n");
    int dem=0;
    uint8_t test_data[20];
    while(1){
        dem++;
        sprintf((char *)test_data, "%d", dem);
        can_write_bytes(0x789, test_data, strlen((char *)test_data), false);
        printf("Gửi: %s\n", test_data);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
