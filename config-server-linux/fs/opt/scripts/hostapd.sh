#!/usr/bin/env bash

set -Eeuo pipefail

ip link set dev wlan0 up
ip addr replace 192.168.1.1/24 dev wlan0
exec hostapd -P /tmp/hostapd.pid /etc/hostapd.conf
