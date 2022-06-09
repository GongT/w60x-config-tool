#!/usr/bin/env bash

set -Eeuo pipefail

PROJECT_NAME="iot-devcfg"

cd "$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
source ./common/functions-install.sh

echo "LOAD: $MONO_ROOT_DIR/environment $PROJECT_NAME"
echo "LOAD: ~/environment $PROJECT_NAME"

NET_NAMESPACE="wireless"
arg_string NET_NAMESPACE networkns "network namespace to create"
arg_string _INTERFACE_NAME interfacename "fiber interface name"
arg_string + WIFI wifi "wifi ap-name:password"
arg_string + MQTT mqtt "MQTT username:password@server:port"
arg_finish "$@"

INTERFACE_NAME="${_INTERFACE_NAME:-}"
if [[ ! $INTERFACE_NAME ]]; then
	info_log "detect first wireless interface..."
	INTERFACE_NAME=$(iw dev | grep Interface | awk '{print $2}' | head -1 || true)
	if [[ $INTERFACE_NAME ]]; then
		ALT_NAME=$(ip link show "$INTERFACE_NAME" | grep altname | awk '{print $2}' | head -1 || true)
		if [[ $ALT_NAME ]]; then
			info_log "    altname: $ALT_NAME"
			INTERFACE_NAME="$ALT_NAME"
		fi
	fi
fi
if [[ ! $INTERFACE_NAME ]]; then
	die "failed find any wireless interface"
fi

info_warn "using $INTERFACE_NAME"

MQTT_CRED=${MQTT%@*}
ENV_PASS=$(
	safe_environment \
		"WIFI_USER=${WIFI%:*}" \
		"WIFI_PASS=${WIFI##*:}" \
		"MQTT_HOST=${MQTT##*@}" \
		"MQTT_USER=${MQTT_CRED%:*}" \
		"MQTT_PASS=${MQTT_CRED##*:}"
)

auto_create_pod_service_unit
unit_podman_image gongt/iot-config-server
unit_podman_image_pull never
unit_podman_arguments "$ENV_PASS"
unit_unit Description "GongT's IoT device configure service"
unit_depend network-online.target
unit_start_notify output 'AP-ENABLED'

unit_fs_bind data/iot /data/ota
unit_fs_bind config/iot /data/configs

# unit_body Restart always
export NET_NAMESPACE
network_use_interface "$INTERFACE_NAME" wlan0

unit_finish
