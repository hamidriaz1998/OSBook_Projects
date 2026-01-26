#!/bin/sh
#
# Run this script INSIDE the Alpine VM to complete setup
#
# It will:
# 1. Copy guest-init.sh to /root/init.sh
# 2. Configure autologin on serial console
# 3. Set up the shell profile
#

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

info() {
    printf "${GREEN}[*]${NC} %s\n" "$1"
}

error() {
    printf "${RED}[!]${NC} %s\n" "$1"
    exit 1
}

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    error "This script must be run as root"
fi

# Check if we have the guest-init.sh available (mounted via 9p)
if [ -f "/mnt/host/guest-init.sh" ]; then
    INIT_SRC="/mnt/host/guest-init.sh"
elif [ -f "/tmp/guest-init.sh" ]; then
    INIT_SRC="/tmp/guest-init.sh"
else
    error "guest-init.sh not found. Mount the vm folder first or copy guest-init.sh to /tmp/"
fi

info "Installing init script from $INIT_SRC..."
cp "$INIT_SRC" /root/init.sh
chmod +x /root/init.sh

info "Configuring autologin on ttyS0..."

# Backup original inittab
cp /etc/inittab /etc/inittab.bak

# Check current serial console configuration
if grep -q "ttyS0::respawn:/sbin/getty" /etc/inittab; then
    # Replace the existing ttyS0 line with autologin version
    sed -i 's|ttyS0::respawn:/sbin/getty.*|ttyS0::respawn:/sbin/getty -n -l /root/init.sh ttyS0 115200 vt100|' /etc/inittab
    info "Updated existing ttyS0 configuration"
else
    # Add new line for ttyS0 autologin
    echo "ttyS0::respawn:/sbin/getty -n -l /root/init.sh ttyS0 115200 vt100" >> /etc/inittab
    info "Added ttyS0 autologin configuration"
fi

info "Setup complete!"
info ""
info "You can now shutdown the VM with 'poweroff'"
info "Next time you boot with ./vm/run.sh, it will auto-login and load your module"
