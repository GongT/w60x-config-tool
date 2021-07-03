#!/usr/bin/env bash

set -Eeuo pipefail
cd "$(dirname "$(realpath "${BASH_SOURCE[0]}")")"

if [[ "${CI:-}" ]] && ! command -v podman &>/dev/null; then
	sudp apt install podman
fi

JSON=$(gpg --quiet --batch --yes --passphrase "$SECRET_PASSWORD" --decrypt build-secrets.json.gpg)

function query() {
	jq --exit-status --compact-output --monochrome-output --raw-output "$@"
}

function JQ() {
	echo "$JSON" | query "$@"
}

PRIMARY=$(JQ '.publish[0]')
INDEX_ARRAY=$(JQ '.publish | keys | .[1:]')

echo "::set-output name=INDEX_ARRAY::$INDEX_ARRAY"

podman push "$LAST_COMMITED_IMAGE" "docker://$PRIMARY/$PROJECT_NAME:latest"
