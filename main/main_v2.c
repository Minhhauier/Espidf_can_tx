#include "isotp.h"
#include "isotp_handler.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static const char *TAG = "ISO-TP-TX";

// Global ISO-TP link and buffers
static volatile IsoTpLink g_link;
static volatile uint8_t g_isotpSendBuf[1024];
static volatile uint8_t g_isotpRecvBuf[1024];

// Configure and start TWAI (CAN) at 500kbps using GPIO27 (TX) / GPIO36 (RX)
static esp_err_t twai_init(void) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(27, 36, TWAI_MODE_NORMAL);
    twai_timing_config_t  t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_LOGI(TAG, "Installing TWAI driver on GPIO27(TX)/GPIO36(RX)...");
    esp_err_t r = twai_driver_install(&g_config, &t_config, &f_config);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI: %s", esp_err_to_name(r));
        return r;
    }

    ESP_LOGI(TAG, "Starting TWAI...");
    r = twai_start();
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI: %s", esp_err_to_name(r));
        return r;
    }
    
    ESP_LOGI(TAG, "✓ TWAI initialized successfully");
    return r;
}

// Task xử lý CAN messages và poll ISO-TP
void isotp_can_handler_task(void *pvParameter) {
    twai_message_t rx_msg;
    int frame_count = 0;
    
    while (1) {
        // Đọc CAN messages từ bus
        esp_err_t r = twai_receive(&rx_msg, pdMS_TO_TICKS(10));
        if (r == ESP_OK) {
            frame_count++;
            ESP_LOGI(TAG, "[#%d] RX CAN: ID=0x%lX, DLC=%d, Data=[%02X %02X %02X %02X %02X %02X %02X %02X]", 
                     frame_count, rx_msg.identifier, rx_msg.data_length_code,
                     rx_msg.data[0], rx_msg.data[1], rx_msg.data[2], rx_msg.data[3],
                     rx_msg.data[4], rx_msg.data[5], rx_msg.data[6], rx_msg.data[7]);
            
            // Feed vào ISO-TP stack
            isotp_on_can_message(&g_link, rx_msg.data, rx_msg.data_length_code);
           // vTaskDelay(pdMS_TO_TICKS(1));
        }
        
        // Poll ISO-TP stack (gửi Consecutive Frames, xử lý timeouts)
        isotp_poll(&g_link);
        
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// Function to send ISO-TP message (non-blocking)
static esp_err_t isotp_send_message(const char *message) {
    uint16_t len = strlen(message);
   // printf("GỬI CHUỖI (%u bytes)\n", len);
    printf("Send: %s\n", message);
    
    int ret = isotp_send(&g_link, (const uint8_t *)message, len);
    if (ret != ISOTP_RET_OK) {
        ESP_LOGE(TAG, "✗ isotp_send failed: %d", ret);
        return ESP_FAIL;
    }
    
//    ESP_LOGI(TAG, "✓ Message queued for transmission");
    return ESP_OK;
}

void app_main(void) {
    // Init TWAI
 //   esp_log_level_set("*", ESP_LOG_NONE);
    if (twai_init() != ESP_OK) {
        ESP_LOGE(TAG, "TWAI init failed");
        return;
    }
    ESP_LOGI(TAG, "=== INIT ISO-TP ===");
    isotp_init_link(&g_link, 0x789, 
                    g_isotpSendBuf, sizeof(g_isotpSendBuf),
                    g_isotpRecvBuf, sizeof(g_isotpRecvBuf));
    g_link.receive_arbitration_id = 0x787;
   xTaskCreate(isotp_can_handler_task, "isotp_handler", 4096, NULL, 10, NULL);

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Example messages to send
    const char *messages = "Hello, this is a long ISO-TP message sent over CAN bus using ESP32. "
                           "It should be split into multiple frames automatically by the ISO-TP stack. "
                           "This is useful for transmitting data larger than 8 bytes over CAN.";

    int msg_index = 0;
    // int num_messages = sizeof(messages) / sizeof(messages[0]);
    char buffer[256];
    ESP_LOGI(TAG, "=== START SENDING ISO-TP MESSAGES ===");
    
    while (1) {
        snprintf(buffer, sizeof(buffer), "Message #%d: %s", msg_index++, messages);
        while(g_link.send_status != ISOTP_SEND_STATUS_IDLE) {
            isotp_poll(&g_link);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        isotp_send_message(buffer);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}