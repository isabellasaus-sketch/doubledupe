#!/usr/bin/env bash

# MTRX3760 2026 Project 2: Package Deal
# File: test_all_worlds.sh
# Smoke-test the first stable label in each of the five Gazebo rooms.

# ---Configuration------------------------------------------------------------
set -o pipefail

# An optional first argument selects the built workspace to source.
script_directory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
default_workspace="$(cd "${script_directory}/../../.." && pwd)"
workspace_path="${1:-${default_workspace}}"
launch_pid=""
log_directory="$(mktemp -d)"

# Matching indices pair each launch world with its expected output label.
worlds=(small_square large_square rectangle hallway equilateral_triangle)
expected=(SMALL_SQUARE LARGE_SQUARE RECTANGLE HALLWAY EQUILATERAL_TRIANGLE)

# ---stop_current_launch------------------------------------------------------
# setsid creates a dedicated process group; stop that launch and its children.
stop_current_launch()
{
  if [[ -n "${launch_pid}" ]] && kill -0 "${launch_pid}" 2>/dev/null; then
    kill -INT -- "-${launch_pid}" 2>/dev/null || true
    wait "${launch_pid}" 2>/dev/null || true
  fi
  launch_pid=""
}

# ---cleanup------------------------------------------------------------------
# Preserve logs for diagnosis on success, failure or interruption.
cleanup()
{
  stop_current_launch
  echo "Logs are available in: ${log_directory}"
}

trap cleanup EXIT INT TERM

# ---ROS environment----------------------------------------------------------
if [[ ! -f /opt/ros/jazzy/setup.bash ]]; then
  echo "FAIL: ROS 2 Jazzy was not found."
  exit 1
fi

if [[ ! -f "${workspace_path}/install/setup.bash" ]]; then
  echo "FAIL: build the workspace before running this script."
  echo "Expected: ${workspace_path}/install/setup.bash"
  exit 1
fi

source /opt/ros/jazzy/setup.bash
source "${workspace_path}/install/setup.bash"
export TURTLEBOT3_MODEL=burger

passed_count=0

# ---World smoke tests---------------------------------------------------------
# Check initial stationary detection, not continuous driving accuracy.
for index in "${!worlds[@]}"; do
  world="${worlds[${index}]}"
  expected_result="${expected[${index}]}"
  log_file="${log_directory}/${world}.log"

  echo
  echo "Testing ${world}; expected ${expected_result}"

  setsid ros2 launch roomshape gazebo.launch.py \
    world:="${world}.sdf" >"${log_file}" 2>&1 &
  launch_pid=$!

  # Poll for at most 45 seconds, also stopping if the launch exits early.
  result=""
  for _ in $(seq 1 45); do
    if grep -q "Stable room type:" "${log_file}"; then
      result="$(grep -m1 "Stable room type:" "${log_file}" | sed 's/.*Stable room type: //')"
      break
    fi

    if ! kill -0 "${launch_pid}" 2>/dev/null; then
      break
    fi

    sleep 1
  done

  stop_current_launch
  # Allow simulator resources to settle before starting the next world.
  sleep 3

  if [[ "${result}" == "${expected_result}" ]]; then
    echo "PASS ${world}: ${result}"
    passed_count=$((passed_count + 1))
  else
    echo "FAIL ${world}: expected ${expected_result}, received ${result:-NO_RESULT}"
    echo "See log: ${log_file}"
  fi
done

echo
echo "Result: ${passed_count}/${#worlds[@]} Gazebo worlds passed."

if [[ "${passed_count}" -ne "${#worlds[@]}" ]]; then
  exit 1
fi
