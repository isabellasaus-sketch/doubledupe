#!/usr/bin/env bash
set -euo pipefail

workspace_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
src_dir="$workspace_dir/src"

repositories=(
    "https://github.com/ROBOTIS-GIT/DynamixelSDK.git DynamixelSDK"
    "https://github.com/ROBOTIS-GIT/turtlebot3_msgs.git turtlebot3_msgs"
    "https://github.com/ROBOTIS-GIT/turtlebot3.git turtlebot3"
    "https://github.com/ROBOTIS-GIT/turtlebot3_simulations.git turtlebot3_simulations"
)

mkdir -p "$src_dir"

for repository in "${repositories[@]}"; do
    read -r url directory <<< "$repository"
    destination="$src_dir/$directory"

    if [[ -e "$destination" ]]; then
        printf 'Skipping %s: destination already exists\n' "$directory"
        continue
    fi

    git clone --branch jazzy "$url" "$destination"
done