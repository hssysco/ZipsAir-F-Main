#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_console.h"

#include "lwip/err.h"
#include "lwip/apps/sntp.h"
#include "sntp.h"

#define DEFAULT_YEAR	1900
#define INIT_YEAR		70
#define LOOP_CNT		20
#define TIME_SERVER		"time.google.com"

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;

static void InitSNTP(void) 
{
	setenv("TZ", "UTC-9", 1);
	tzset();

	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, TIME_SERVER);

	sntp_init();

	return;
}

static void ObtainTime(void) 
{
	time_t now = 0;
	struct tm timeinfo = { 0 };
	int retry = 0;
	const int retry_count = LOOP_CNT;
	
	while(timeinfo.tm_year <= INIT_YEAR && ++retry < retry_count) 
	{
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		time(&now);
		localtime_r(&now, &timeinfo);

		printf("Time : %04d-%02d-%02d %02d:%02d:%02d\n", timeinfo.tm_year + DEFAULT_YEAR, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    }
}

int GetTimeInfo(int argc, char **argv) 
{
	time_t now = 0;
	struct tm timeinfo = { 0 };

	time(&now);
	localtime_r(&now, &timeinfo);

	printf("Time : %04d-%02d-%02d %02d:%02d:%02d\n", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	return 0;
}

void InitRTC(void) 
{
	time_t now;
	struct tm timeinfo;

	++boot_count;

	InitSNTP();

	printf("#### rtc_initialize (%d) \r\n", boot_count);

	time(&now);
	localtime_r(&now, &timeinfo);
	// Is time set? If not, tm_year will be (1970 - 1900).
	if (timeinfo.tm_year < (2019 - DEFAULT_YEAR)) {
//		ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
		printf(" ### Time is not set yet. Connecting to WiFi and getting time over NTP. \r\n");
		ObtainTime();
		// update 'now' variable with current time

		time(&now);
	}

	return;
}

void Register_TimeInfo() 
{
    const esp_console_cmd_t cmd = 
	{
        .command = "timeinfo",
        .help = "Get version of time info",
        .hint = NULL,
        .func = &GetTimeInfo,
		.argtable = NULL
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}


