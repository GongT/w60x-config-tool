#!/usr/bin/env bash

set -Eeuo pipefail

cd /data/internal

echo "$WIFI_USER" >wifi.name
echo "$WIFI_PASS" >wifi.pass
echo "$MQTT_HOST" >mqtt.server
echo "$MQTT_USER" >mqtt.user
echo "$MQTT_PASS" >mqtt.pass

sleep 3
exec nginx -c /etc/nginx/nginx.conf "$@"
