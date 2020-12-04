/*
 * Copyright (c) 2020, CATIE
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed.h"
#include <nsapi_dns.h>
#include <MQTTClientMbedOs.h>

namespace {
#define MQTT_TOPIC "/estia/group1"
}

NetworkInterface *network;
static DigitalOut led(LED1);

// main() runs in its own thread in the OS
// (note the calls to ThisThread::sleep_for below for delays)
int main()
{
    nsapi_size_or_error_t retcode;

	printf("Connecting to border router...\n");

    /* Get Network configuration */
    network = NetworkInterface::get_default_instance();

    if (!network) {
        printf("Error! No network interface found.\n");
        return 0;
    }

    /* Add DNS */
    nsapi_addr_t new_dns = {
        NSAPI_IPv6,
        { 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 }
    };
    nsapi_dns_add_server(new_dns, "LOWPAN");

    /* Border Router connection */
    retcode = network->connect();
    if (retcode != 0) {
        printf("Error! net->connect() returned: %d\n", retcode);
        return retcode;
    }

    /* Print IP address */
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

    /* Open TCP Socket */
    TCPSocket socket;
    const char* hostname = "fd9f:590a:b158::1";
    int port = 1883;

    /* MQTT Connection */
    MQTTClient client(&socket);
    socket.open(network);
    int ret = socket.connect(hostname, port);
    if(ret != 0){
        printf("Connection to MQTT broker Failed");
        return 0;
    }

    /* MQTT Publish */
    MQTTPacket_connectData data;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char *)"6LoWPAN_Node";
    client.connect(data);

    /* Create MQTT Message */
    char *mqttPayload = "Hello from 6TRON";

    MQTT::Message message;
    message.qos = MQTT::QOS1;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)mqttPayload;
    message.payloadlen = strlen(mqttPayload);

    int8_t rc = 0;

    while(1){
        printf("Send: %s to MQTT Broker: %s\n", mqttPayload, hostname);
        rc = client.publish(MQTT_TOPIC, message); 
        ThisThread::sleep_for(5000);
    }
}