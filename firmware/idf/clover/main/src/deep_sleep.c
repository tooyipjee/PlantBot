#include "deep_sleep.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#if SOC_RTC_FAST_MEM_SUPPORTED
static RTC_DATA_ATTR struct timeval sleep_enter_time;
#else
static struct timeval sleep_enter_time;
#endif

static const char *TAG = "deep_sleep";

static void configure_gpio()
{
    esp_sleep_config_gpio_isolate();
    gpio_set_direction(GPIO_NUM_3, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_3, 0);
    gpio_hold_en(GPIO_NUM_3);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2, 1);
    gpio_hold_en(GPIO_NUM_2);
}

static void deep_sleep_task(void *args)
{
    double duration = *((int *)args);

    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "Wake up from timer. Time spent in deep sleep: %dms", sleep_time_ms);
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            ESP_LOGI(TAG, "Not a deep sleep reset");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);

#if CONFIG_IDF_TARGET_ESP32
    rtc_gpio_isolate(GPIO_NUM_12);
#endif

    ESP_LOGI(TAG, "Entering deep sleep for %d seconds", duration);

    gettimeofday(&sleep_enter_time, NULL);

    configure_gpio();

    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(duration * 1000000));
    esp_deep_sleep_start();
}

void deep_sleep_duration(double duration)
{
    ESP_LOGI(TAG, "Creating deep sleep task with duration: %d seconds", duration);
    xTaskCreate(deep_sleep_task, "deep_sleep_task", 4096, &duration, 6, NULL);
}

void deep_sleep_interrupt(int gpio)
{
    const gpio_config_t config = {
        .pin_bit_mask = BIT64(gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_LOW_LEVEL,
    };

    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup());

    printf("Enabling GPIO wakeup on pin GPIO%d\n", gpio);

    ESP_LOGI(TAG, "Entering deep sleep, waiting for GPIO%d to trigger wakeup", gpio);
    esp_deep_sleep_start();
}

void deep_sleep_forever()
{
    ESP_LOGI(TAG, "Entering deep sleep forever");

    configure_gpio();
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
}