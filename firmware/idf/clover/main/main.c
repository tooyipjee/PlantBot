/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/* Standard includes */
#include <stdio.h>
#include <string.h>

/* ESP-IDF includes */
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Application includes */
#include "common.h"
#include "gap.h"
#include "gatt_svc.h"
#include "heart_rate.h"
#include "led.h"
#include "deep_sleep.h"

/* Duty cycle configuration */
#define DUTY_CYCLE_PERCENTAGE    0.1f     // Set your desired duty cycle here (0.1% to 100%)
#define MIN_ACTIVE_TIME_MS       500      // Minimum active time due to startup requirements

/* Task configuration */
#define NIMBLE_TASK_STACK_SIZE    (4 * 1024)
#define HEART_RATE_TASK_STACK_SIZE (4 * 1024)
#define TASK_PRIORITY_NORMAL      5

/* Global variables */
static uint32_t g_active_time_ms;
static uint32_t g_sleep_time_ms;

/* Function declarations */
static void on_stack_reset(int reason);
static void on_stack_sync(void);
static void nimble_host_config_init(void);
static void nimble_host_task(void *param);
static void heart_rate_task(void *param);

/* Calculate sleep time based on duty cycle */
static void calculate_timing(void) {
    g_active_time_ms = MIN_ACTIVE_TIME_MS;
    g_sleep_time_ms = (uint32_t)((MIN_ACTIVE_TIME_MS / DUTY_CYCLE_PERCENTAGE * 100.0f) - MIN_ACTIVE_TIME_MS);
    
    ESP_LOGI(TAG, "Duty Cycle Configuration:");
    ESP_LOGI(TAG, "  Target: %.3f%%", DUTY_CYCLE_PERCENTAGE);
    ESP_LOGI(TAG, "  Active Time: %d ms", g_active_time_ms);
    ESP_LOGI(TAG, "  Sleep Time: %d ms", g_sleep_time_ms);
}

/* BLE stack callbacks */
static void on_stack_reset(int reason) {
    ESP_LOGI(TAG, "NimBLE stack reset, reason: %d", reason);
}

static void on_stack_sync(void) {
    ESP_LOGI(TAG, "NimBLE stack synchronized");
    adv_init();
}

/* NimBLE host configuration */
static void nimble_host_config_init(void) {
    ble_hs_cfg.reset_cb = on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
}

/* Task implementations */
static void nimble_host_task(void *param) {
    ESP_LOGI(TAG, "NimBLE host task started");
    nimble_port_run();
    nimble_port_deinit();
    vTaskDelete(NULL);
}

static void heart_rate_task(void *param) {
    ESP_LOGI(TAG, "Heart rate task started");
    
    while (1) {
        update_heart_rate();
        ESP_LOGI(TAG, "Heart rate updated to %d", get_heart_rate());
        send_heart_rate_indication();
        vTaskDelay(HEART_RATE_TASK_PERIOD);
    }
}

/* Main application entry point */
void app_main(void) {
    esp_err_t ret;
    
    /* Calculate timing based on duty cycle */
    calculate_timing();
    
    /* Initialize LED */
    led_init();
    
    /* Initialize NVS flash */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    /* Initialize BLE stack */
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        return;
    }
    
    gap_init();
    gatt_svc_init();
    nimble_host_config_init();
    
    /* Create tasks */
    xTaskCreate(nimble_host_task, "NimBLE Host", NIMBLE_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_NORMAL, NULL);
    xTaskCreate(heart_rate_task, "Heart Rate", HEART_RATE_TASK_STACK_SIZE,
                NULL, TASK_PRIORITY_NORMAL, NULL);
    
    /* Stay active for the minimum required time */
    vTaskDelay(pdMS_TO_TICKS(g_active_time_ms));
    
    /* Configure sleep time and enter deep sleep */
    ESP_LOGI(TAG, "Entering deep sleep for %d ms", g_sleep_time_ms);
    deep_sleep_duration(g_sleep_time_ms * 1000ULL);
    
    // deep_sleep_forever();
}