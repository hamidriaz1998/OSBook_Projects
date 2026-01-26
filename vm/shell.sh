#!/bin/bash
#
# shell.sh - Boot the VM without loading any module
#
# Usage: ./vm/shell.sh
#
# Useful for:
#   - Manual VM setup/configuration
#   - Debugging
#   - Installing packages
#   - General VM maintenance
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VM_IMAGE="$SCRIPT_DIR/alpine.img"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

error() {
    echo -e "${RED}Error: $1${NC}" >&2
    exit 1
}

info() {
    echo -e "${GREEN}[*]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# Check VM image exists
if [[ ! -f "$VM_IMAGE" ]]; then
    error "VM image not found: $VM_IMAGE"
fi

# Check if KVM is available
KVM_FLAG=""
if [[ -r /dev/kvm ]]; then
    KVM_FLAG="-enable-kvm"
    info "KVM acceleration enabled"
else
    warn "KVM not available, VM will be slower"
fi

info "Starting QEMU VM (shell mode)..."
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  VM Console - Press Ctrl+A, X to exit QEMU"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Launch QEMU without 9p share (plain shell mode)
qemu-system-x86_64 \
    -m 1G \
    -smp 2 \
    $KVM_FLAG \
    -cpu host \
    -drive file="$VM_IMAGE",format=qcow2 \
    -nographic

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
info "VM session ended"
