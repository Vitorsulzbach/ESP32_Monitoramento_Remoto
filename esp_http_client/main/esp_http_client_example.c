/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "driver/gpio.h"
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs_flash.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "sdkconfig.h"
#include "dht11.h"

#define MAX_HTTP_RECV_BUFFER 512
static const char *TAG = "HTTP_CLIENT";

/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

static void http_test_task(void *pvParameters)
{
	while(1){
		esp_http_client_config_t config = {
			.host = "http://34.95.165.121:8000",
			.path = "/",
			.transport_type = HTTP_TRANSPORT_OVER_TCP,
			.event_handler = _http_event_handler,
		};
		esp_http_client_handle_t client = esp_http_client_init(&config);
		struct read r;
		int humidity[25];
		int temperature[25];
		for(int i = 0;i<25;i++){
			getData(&r);
			while(r.temperature==-274||r.humidity<0||r.humidity>100){
				getData(&r);
		}
			humidity[i] = r.humidity;
			temperature[i] = r.temperature;
		}
		const char *post_data = malloc(1000);
		sprintf(post_data, "x0=%d&x1=%d&x2=%d&x3=%d&x4=%d&x5=%d&x6=%d&x7=%d&x8=%d&x9=%d&x10=%d&x11=%d&x12=%d&x13=%d&x14=%d&x15=%d&x16=%d&x17=%d&x18=%d&x19=%d&x20=%d&x21=%d&x22=%d&x23=%d&x24=%d",temperature[0],temperature[1],temperature[2],temperature[3],temperature[4],temperature[5],temperature[6],temperature[7],temperature[8],temperature[9],temperature[0],temperature[11],temperature[12],temperature[13],temperature[14],temperature[15],temperature[16],temperature[17],temperature[18],temperature[19],temperature[20],temperature[21],temperature[22],temperature[23],temperature[24]);
		esp_http_client_set_url(client, "http://34.95.165.121:8000/");
		esp_http_client_set_method(client, HTTP_METHOD_POST);
		esp_http_client_set_post_field(client, post_data, strlen(post_data));
		esp_err_t err = esp_http_client_perform(client);
		if (err == ESP_OK) {
			ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
					esp_http_client_get_status_code(client),
					esp_http_client_get_content_length(client));
		} else {
			ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
		}
		printf("Pausando por agora! De volta em 5 min.\n");
		vTaskDelay(1800000 / portTICK_RATE_MS); // 1800000/0.001=1800 s => 1800 s = 30 min
		esp_http_client_cleanup(client);
	}
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
     
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to AP, begin http example");

    
    xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL);
}
