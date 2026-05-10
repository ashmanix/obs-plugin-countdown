#!/usr/bin/env zsh

BUILD_FILE_LOCATION=./build_macos/RelWithDebInfo/obs-plugin-countdown.plugin
BUILD_DESTINATION_FOLDER="$HOME/Library/Application Support/obs-studio/plugins/"
APP_NAME="OBS"

RED="\033[0;31m"
GREEN="\033[0;32m"
NC="\033[0m" # No Color
CLEAR_LINE="\r\033[K"  # Carriage return with clear line

echo "Starting build and run script..."


# Temporary files to store output
temp_close_output=""
temp_build_output=$(mktemp)
# Cleanup temp files on script exit
trap 'rm -f "$temp_build_output" "$temp_close_output"' EXIT

# Initial cmake getting dependencies
echo -ne "${CLEAR_LINE}Running cmake script..."

cmake --preset macos >"$temp_build_output" 2>&1

if [[ $? -ne 0 ]]; then
    echo "${CLEAR_LINE}${RED}X${NC} Initial cmake script failed! Ending script. Output:" >&2
    cat "$temp_build_output" >&2
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} Initial cmake completed successfully!"
fi

# Initial build getting dependencies
echo -ne "${CLEAR_LINE}Running build script..."

cmake --build --preset macos >"$temp_build_output" 2>&1

if [[ $? -ne 0 ]]; then
    echo "${CLEAR_LINE}${RED}X${NC} Initial Build script failed! Ending script. Output:" >&2
    cat "$temp_build_output" >&2
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} Initial build completed successfully!"
fi

# Copy the plugin to the correct destination folder
echo -ne "Copying build folder to application folder"
cp -r "$BUILD_FILE_LOCATION" "$BUILD_DESTINATION_FOLDER"
if [[ $? -ne 0 ]]; then
    echo "${CLEAR_LINE}${RED}X${NC} Copying plugin to application folder failed! Ending script." >&2
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} Plugin copied to application folder successfully"
fi


# If OBS is open then close it
if pgrep -x "$APP_NAME" >/dev/null; then
    echo -ne "$APP_NAME is currently open. Now closing...."
    # Temporary file to store output
    temp_close_output=$(mktemp)

    osascript <<EOF >"$temp_close_output" 2>&1
            try
                tell application "$APP_NAME" to quit
            on error errorMessage number errorNumber
                -- -128 is "User cancelled", -1711 is "Automatic termination"
                if (errorNumber is not -128) and (errorNumber is not -1711) then
                    error errorMessage number errorNumber
                end if
            end try
EOF

    if [[ $? -ne 0 ]]; then
        echo "${CLEAR_LINE}${RED}X${NC} Error closing $APP_NAME app! Output:" >&2
        cat "$temp_close_output" >&2
        exit 1
    fi

    echo "${CLEAR_LINE}${GREEN}✓${NC} $APP_NAME closed successfully!"
    sleep 1
fi

echo -ne "Opening $APP_NAME app"
open -a "$APP_NAME"
if [[ $? -ne 0 ]]; then
    echo "${CLEAR_LINE}${RED}X${NC} Error opening $APP_NAME app! Ending script." >&2
    exit 1
else
    echo "${CLEAR_LINE}${GREEN}✓${NC} $APP_NAME opened successfully!"
fi

echo "${GREEN}✓${NC} End of build and run script!"
