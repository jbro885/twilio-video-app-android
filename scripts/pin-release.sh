#!/bin/bash

export LANG=en_US.UTF-8;
export PATH=/usr/local/bin:$PATH
export WORKSPACE_ROOT_DIR=`pwd`
export SCRIPTS_DIR="$WORKSPACE_ROOT_DIR/scripts"
export SDK_PACKAGE_PATH="$WORKSPACE_ROOT_DIR/package"

PRODUCT_NAME="video"

if [ ! -z "$RELEASE_VERSION" ]; then
    export SDK_RELEASE_VERSION=${RELEASE_VERSION}
else
    exit 0;
fi

echo "Prepping sdk-release-tool..."
if [ ! -d $SDK_RELEASE_TOOL_HOME ]; then
    echo "Error: SDK_RELEASE_TOOL_HOME does not exist"
    exit 1
fi

if [ ! -d "$SDK_RELEASE_TOOL_HOME/venv" ]; then
    echo "Error: venv not found. Please run \"make\""
    exit 1
fi

rm -rf ${SDK_PACKAGE_PATH}
mkdir -p ${SDK_PACKAGE_PATH}

pushd "$SDK_RELEASE_TOOL_HOME"

rm -rf ${SCRIPTS_DIR}/downloads
mkdir ${SCRIPTS_DIR}/downloads

MAJORMINOR=$(echo $SDK_RELEASE_VERSION| cut -d'.' -f-2)
DIRECT_URL="https://media.twiliocdn.com/sdk/android/${PRODUCT_NAME}/releases/${SDK_RELEASE_VERSION}/twilio-${PRODUCT_NAME}-android-${SDK_RELEASE_VERSION}.aar"
MAJORMINOR_URL="https://media.twiliocdn.com/sdk/android/${PRODUCT_NAME}/v${MAJORMINOR}/twilio-${PRODUCT_NAME}-android.aar"
LATEST_URL="https://media.twiliocdn.com/sdk/android/${PRODUCT_NAME}/latest/twilio-${PRODUCT_NAME}-android.aar"

curl $DIRECT_URL --output ${SCRIPTS_DIR}/downloads/twilio-${PRODUCT_NAME}-direct.aar

echo "sdk-release-tool: updating..."
./sdk-release-tool pin -f --prod twilio-${PRODUCT_NAME}-android ${SDK_RELEASE_VERSION}
if [ "$?" -ne "0" ]; then
    echo "Error: failed to execute sdk-release-tool update"
    exit 1
fi

echo "sdk-release-tool: pin latest..."
./sdk-release-tool pin-latest -f --prod twilio-${PRODUCT_NAME}-android ${SDK_RELEASE_VERSION}
if [ "$?" -ne "0" ]; then
    echo "Error: failed to execute sdk-release-tool pin latest"
    exit 1
fi

echo "Testing whether versions are same..."
curl -L $MAJORMINOR_URL --output ${SCRIPTS_DIR}/downloads/twilio-${PRODUCT_NAME}-majorminor.aar
curl -L $LATEST_URL --output ${SCRIPTS_DIR}/downloads/twilio-${PRODUCT_NAME}-latest.aar

md5sum ${SCRIPTS_DIR}/downloads/twilio-${PRODUCT_NAME}-direct.aar ${SCRIPTS_DIR}/downloads/twilio-${PRODUCT_NAME}-majorminor.aar ${SCRIPTS_DIR}/downloads/twilio-${PRODUCT_NAME}-latest.aar
popd

echo "Major minor version: $MAJORMINOR"
echo "Direct URL: $DIRECT_URL"
echo "Major Minor URL: $MAJORMINOR_URL"
echo "Latest URL: $LATEST_URL"

