#!/usr/bin/env bash

set -Eeuo pipefail

dnsmasq -C ./dnsmasq.conf 
