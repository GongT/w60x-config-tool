#!/usr/bin/env bash

set -Eeuo pipefail

cd "$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
source ./common/functions-build.sh
arg_finish

info "starting..."

RESULT=$(create_if_not iot-config-server-result gongt/alpine-init)

STEP="创建基础镜像"
make_base_image_by_apk "gongt/alpine-init" "iot-config-server-result" bash dnsmasq hostapd nginx

### 复制文件
STEP="复制文件"
hash_fs_files() {
	hash_path fs
}
copy_fs_files() {
	buildah copy "$1" fs /
}
buildah_cache2 iot-config-server-result hash_fs_files copy_fs_files
### 复制文件 END

buildah_config iot-config-server-result \
	"--author=GongT <admin@gongt.me>" \
	"--created-by=#MAGIC!" \
	"--label=name=gongt/iot-config-server" \
	"--volume=/data/leases"

RESULT=$(create_if_not "iot-config-server-commit" "$BUILDAH_LAST_IMAGE")
buildah commit "$RESULT" gongt/iot-config-server
