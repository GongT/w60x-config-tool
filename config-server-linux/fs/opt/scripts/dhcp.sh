#!/usr/bin/env bash

set -Eeuo pipefail

sleep 2
exec dnsmasq -C /etc/dnsmasq.conf
