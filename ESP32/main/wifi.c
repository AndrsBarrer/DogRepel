#include "wifi.h"
#include "tcp.h"

#include "config.h"

static char *TAG = "WIFI";

bool storedSSID = false;
bool storedPASS = false;
char got_collar_mac[18] = {0};
char station_mac_addr_str[20] = {0};
int8_t currentRSSI = 0;
char server_ip[IP_ADDR_STR_LEN] = {0};

#ifdef STATION
static esp_err_t root_handler(httpd_req_t *req)
{
    const char *resp =
        "<html>"
        "<head><style>"
        "body{display:flex; flex-direction:column; align-items:center; height:80vh; margin:0; background:#f0f0f0; font-family:Arial,sans-serif;} "
        "form{display:flex; flex-direction:column; align-items:center; width:80%; max-width:500px; padding:20px; background:#fff; box-shadow:0 4px 20px rgba(0,0,0,0.1); border-radius:10px;} "
        "label{font-size:2rem; color:#333;} "
        "input{width:100%; padding:15px; font-size:2rem; border:1px solid #ccc; border-radius:5px; margin:10px 0;} "
        "input:focus{border-color:#007BFF; outline:none;} "
        "input[type=\"submit\"]{background-color:#007BFF; color:#fff; border:none; cursor:pointer; font-weight:bold;} "
        "input[type=\"submit\"]:hover{background-color:#0056b3;}"
        "</style></head>"
        "<body>"
        "<form action=\"/process-info\" method=\"post\">"
        "<label for=\"ssid_input\">Enter WiFi Credentials</label>"
        "<input type=\"text\" id=\"ssid_input\" name=\"ssid\" placeholder=\"SSID\" required />"
        "<input type=\"text\" id=\"password_input\" name=\"password\" placeholder=\"Password\" required />"
        "<input type=\"submit\" value=\"Submit\" />"
        "</form>"
        "</body>"
        "</html>";

    return httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
}

void save_wifi_credentials(char *ssid, char *pass)
{
    nvs_handle_t my_handle;
    int err = nvs_open("wifi_storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("[X] Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = nvs_set_str(my_handle, "storedSSID", ssid);
        printf((err != ESP_OK) ? "[X] Failed to set SSID in storage!\n" : "[✓] Wrote SSID to NVS Storage.\n");

        err = nvs_set_str(my_handle, "storedPASS", pass);
        printf((err != ESP_OK) ? "[X] Failed to set Password in storage!\n" : "[✓] Wrote Password to NVS Storage.\n");
        printf((err != ESP_OK) ? "[X] Failed to commit!\n" : "[✓] Commited to NVS Storage.\n");
        nvs_commit(my_handle);
        nvs_close(my_handle);

        // Set these as true so that the ESP can reboot itself
        storedSSID = true;
        storedPASS = true;
    }
}

esp_err_t process_info_handler(httpd_req_t *req)
{
    char buffer[256];
    int ret = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (ret <= 0)
        return httpd_resp_send_500(req);

    buffer[ret] = '\0';

    ESP_LOGI(TAG, "Received: %s", buffer);

    // Extract SSID and password from buffer
    char ssid[32], password[64];

    // Reads after ssid=, up to 31 characters, stopping at the &
    // then it matches &password= and reads up to 63 characters, stopping at whitespace
    sscanf(buffer, "ssid=%31[^&]&password=%63s", ssid, password);

    ESP_LOGI(TAG, "SSID: %s", ssid);
    ESP_LOGI(TAG, "PASS: %s", password);

    save_wifi_credentials(ssid, password);

    const char *resp = "<html><body><h3>Credentials saved! Rebooting...</h3></body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

httpd_uri_t root_uri =
    {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL};

httpd_uri_t process_info_uri =
    {
        .uri = "/process-info",
        .method = HTTP_POST,
        .handler = process_info_handler,
        .user_ctx = NULL};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 20480;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &process_info_uri);
        return server;
    }

    return NULL;
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config =
        {
            .ap =
                {
                    .ssid = ESP_WIFI_AP_SSID,
                    .ssid_len = strlen(ESP_WIFI_AP_SSID),
                    .channel = ESP_WIFI_CHANNEL,
                    .password = ESP_WIFI_AP_PASSWD,
                    .max_connection = MAX_STA_CONN,
                    .authmode = WIFI_AUTH_WPA_WPA2_PSK},
        };

    if (strlen(ESP_WIFI_AP_PASSWD) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);
    ESP_LOGI(TAG, "AP IP Address: " IPSTR, IP2STR(&ip_info.ip));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization complete.");
    ESP_LOGI(TAG, "SSID: %s", ESP_WIFI_AP_SSID);
    ESP_LOGI(TAG, "Password: %s", ESP_WIFI_AP_PASSWD);
}

void wifi_init_sta(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize networking and Wi-Fi for station mode
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_initiation));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    // Open NVS handle
    nvs_handle_t my_handle;
    err = nvs_open("wifi_storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS handle!");
    }

    // Buffers for SSID and password
    size_t ssid_size = 32;
    size_t pass_size = 64;

    char ssid[ssid_size];
    char pass[pass_size];

    // Get stored SSID from NVS
    err = nvs_get_str(my_handle, "storedSSID", ssid, &ssid_size);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "SSID not found in NVS, using default.");
        // this is where the esp flash memory should be reset
    }

    // Get stored password from NVS
    err = nvs_get_str(my_handle, "storedPASS", pass, &pass_size);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "[X] Password not found in NVS!");
        // this is where the esp flash memory should be reset
    }

    // Close NVS handle
    nvs_close(my_handle);

    // Configure Wi-Fi with the retrieved SSID and password
    wifi_config_t wifi_sta_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Set authentication mode
        }};

    // Copy SSID and password from the buffers
    strcpy((char *)wifi_sta_config.sta.ssid, ssid);
    strcpy((char *)wifi_sta_config.sta.password, pass);

    // Set Wi-Fi mode and configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));

    // Start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "[✓] Initialization of Station finished.");
    ESP_LOGI(TAG, "Connected SSID: %s", ssid);
    ESP_LOGI(TAG, "Using password: %s", pass);
}

#endif

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "[✓] Station started");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        runTCPtask = false;
        ESP_LOGI(TAG, "[!] Disconnected from Wi-Fi, trying to reconnect...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "[✓] Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        runTCPtask = true;

        // Create the TCP client task
        xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
    }
}

#ifdef COLLAR

void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();

    // Initialize Wi-Fi in station mode
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    // Configure Wi-Fi station
    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = WIFI_DOG_REPEL_SSID,
            .password = WIFI_DOG_REPEL_PASS,
            .scan_method = WIFI_FAST_SCAN, // this is used to save battery
            .failure_retry_cnt = ESP_MAXIMUM_RETRY,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    // Apply the Wi-Fi configuration
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config);
    esp_wifi_start();

    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG_STA, "Sending out probe request.");

        esp_wifi_connect();
        esp_deep_sleep_start();
    }
}
#endif