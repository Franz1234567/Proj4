#!/bin/bash

# Default values, you need to change them to your raspberry
RASPBERRY_NAME="barnouin"
RASPBERRY_IP="192.168.230.92"
TARGET_FOLDER="/home/barnouin/Documents/Proj4"

# Local folder to cocpy
LOCAL_FOLDER=$1

# Check if the local folder argument is provided
if [ -z "$LOCAL_FOLDER" ]; then
    echo "Error: You must provide the local folder to copy."
    echo "Usage: ./scp_to_raspberry.sh <local_folder>"
    exit 1
fi

# copy command
echo "Copying folder '$LOCAL_FOLDER' to $RASPBERRY_NAME@$RASPBERRY_IP:$TARGET_FOLDER..."
scp -r "$LOCAL_FOLDER" "$RASPBERRY_NAME@$RASPBERRY_IP:$TARGET_FOLDER"

# tell if it worked
if [ $? -eq 0 ]; then
    echo "Folder successfully copied to the Raspberry Pi!"
else
    echo "Failed to copy the folder."
fi
