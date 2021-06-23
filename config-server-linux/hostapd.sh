#!/usr/bin/env bash

set -Eeuo pipefail

ip link set dev wlp58s0 up
ip addr replace 192.168.1.1/24 dev wlp58s0
hostapd ./hostapd.conf -P /var/run/config-hostapd.pid "$@"
