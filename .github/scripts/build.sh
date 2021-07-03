#!/usr/bin/env bash

set -Eeuo pipefail
export TMPDIR="$RUNNER_TEMP"

cd "$(dirname "$(realpath "${BASH_SOURCE[0]}")")/../../config-server-linux"
# shellcheck source=../../config-server-linux/common/functions-build-host.sh
source "./common/functions-build-host.sh"

if [[ ${CI+found} != found ]]; then
	die "This script is only for CI"
fi

export REWRITE_IMAGE_NAME="build.local/dist/${PROJECT_NAME}"

bash "./build.sh" || die "Build failed"
