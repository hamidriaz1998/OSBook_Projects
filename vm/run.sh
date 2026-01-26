#!/bin/bash
#
# run.sh - Launch QEMU VM and test a kernel module
#
# Usage: ./vm/run.sh <module_dir> <module_name>
#
# Examples:
#   ./vm/run.sh chapter2 time_elapsed
#   ./vm/run.sh chapter3/2.KM_Task_info task_info
#   cd chapter2 && ../vm/run.sh . simple
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VM_IMAGE="$SCRIPT_DIR/alpine.img"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

usage() {
    echo "Usage: $0 <module_dir> <module_name>"
    echo ""
    echo "Arguments:"
    echo "  module_dir   Directory containing the kernel module source (relative to project root)"
    echo "  module_name  Name of the module (without .ko extension)"
    echo ""
    echo "Examples:"
    echo "  $0 chapter2 time_elapsed"
    echo "  $0 chapter3/2.KM_Task_info task_info"
    exit 1
}

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

# Check arguments
if [[ $# -ne 2 ]]; then
    usage
fi

MODULE_DIR="$1"
MODULE_NAME="$2"

# Resolve module directory to absolute path
if [[ "$MODULE_DIR" = "." ]]; then
    MODULE_DIR_ABS="$(pwd)"
elif [[ "$MODULE_DIR" = /* ]]; then
    MODULE_DIR_ABS="$MODULE_DIR"
else
    MODULE_DIR_ABS="$PROJECT_ROOT/$MODULE_DIR"
fi

# Validate module directory
if [[ ! -d "$MODULE_DIR_ABS" ]]; then
    error "Module directory not found: $MODULE_DIR_ABS"
fi

if [[ ! -f "$MODULE_DIR_ABS/Makefile" ]]; then
    error "No Makefile found in $MODULE_DIR_ABS"
fi

# Check for source files
if ! ls "$MODULE_DIR_ABS"/*.c &>/dev/null; then
    error "No .c source files found in $MODULE_DIR_ABS"
fi

# Write module config file for guest to read
echo "$MODULE_NAME" > "$MODULE_DIR_ABS/.module_name"

# Cleanup on exit
cleanup() {
    rm -f "$MODULE_DIR_ABS/.module_name"
}
trap cleanup EXIT

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

info "Module directory: $MODULE_DIR_ABS"
info "Module name: $MODULE_NAME"
info "Starting QEMU VM..."
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  VM Console - Press Ctrl+A, X to exit QEMU"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Launch QEMU
# - 9p virtfs shares the module source directory
# - nographic for console-only mode
# - Module name passed via .module_name file in shared directory
qemu-system-x86_64 \
    -m 1G \
    -smp 2 \
    $KVM_FLAG \
    -cpu host \
    -drive file="$VM_IMAGE",format=qcow2 \
    -nographic \
    -virtfs local,path="$MODULE_DIR_ABS",mount_tag=hostshare,security_model=mapped-xattr

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
info "VM session ended"
