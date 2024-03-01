#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

int main() {
    char json_string[] = "{\"cluster_ip\": \"192.168.110.73\",\"serverNodeKey\": \"740349174670594049\",\"addrs\": [\"192.168.110.53:9444\",\"www.baidu.com\",\"1080::8:800:200C:417A\"]}";
    cJSON *json = NULL, *cluster_ip = NULL, *serverNodeKey = NULL, *addrs = NULL;
    cJSON *addr = NULL;

    // 解析JSON字符串
    json = cJSON_Parse(json_string);
    if (json == NULL) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return 1;
    }

    // 获取cluster_ip字段的值
    cluster_ip = cJSON_GetObjectItemCaseSensitive(json, "cluster_ip");
    if (cJSON_IsString(cluster_ip) && (cluster_ip->valuestring != NULL)) {
        printf("cluster_ip: %s\n", cluster_ip->valuestring);
    }

    // 获取serverNodeKey字段的值
    serverNodeKey = cJSON_GetObjectItemCaseSensitive(json, "serverNodeKey");
    if (cJSON_IsString(serverNodeKey) && (serverNodeKey->valuestring != NULL)) {
        printf("serverNodeKey: %s\n", serverNodeKey->valuestring);
    }

    // 获取addrs字段的值（数组）
    addrs = cJSON_GetObjectItemCaseSensitive(json, "addrs");
    if (cJSON_IsArray(addrs)) {
        cJSON_ArrayForEach(addr, addrs) {
            if (cJSON_IsString(addr) && (addr->valuestring != NULL)) {
                printf("addr: %s\n", addr->valuestring);
            }
        }
    }

    // 释放cJSON对象
    cJSON_Delete(json);

    return 0;
}