#!/bin/sh
#
# guest-init.sh - Runs inside the Alpine VM on autologin
#
# This script:
# 1. Mounts the shared folder from host (if available)
# 2. Compiles the kernel module (smart rebuild - only if sources changed)
# 3. Loads the module with insmod
# 4. Shows dmesg output
# 5. Drops to interactive shell
#
# Installation: Copy this to /root/init.sh inside the VM and make executable
#

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

MOUNT_POINT="/mnt/host"
MODULE_NAME=""

info() {
    printf "${GREEN}[*]${NC} %s\n" "$1"
}

warn() {
    printf "${YELLOW}[!]${NC} %s\n" "$1"
}

error() {
    printf "${RED}[!]${NC} %s\n" "$1"
}

# Read module_name from config file in shared directory
get_module_name() {
    # First mount the share to read the config
    if [ ! -d "$MOUNT_POINT" ]; then
        mkdir -p "$MOUNT_POINT"
    fi

    # Try to mount 9p share
    if mount -t 9p -o trans=virtio,version=9p2000.L hostshare "$MOUNT_POINT" 2>/dev/null; then
        if [ -f "$MOUNT_POINT/.module_name" ]; then
            MODULE_NAME="$(cat "$MOUNT_POINT/.module_name")"
        fi
    fi
}

# Check if rebuild is needed
# Returns 0 (true) if rebuild needed, 1 (false) otherwise
needs_rebuild() {
    local ko_file="$1"

    # If .ko doesn't exist, need to build
    if [ ! -f "$ko_file" ]; then
        return 0
    fi

    # Check if .ko was built for a different kernel version
    # modinfo returns the vermagic which includes kernel version
    local ko_version
    ko_version=$(modinfo -F vermagic "$ko_file" 2>/dev/null | awk '{print $1}')
    local running_version
    running_version=$(uname -r)

    if [ "$ko_version" != "$running_version" ]; then
        info "Module built for $ko_version, running $running_version"
        return 0
    fi

    # Check if any .c file is newer than .ko
    for c_file in *.c; do
        if [ -f "$c_file" ] && [ "$c_file" -nt "$ko_file" ]; then
            return 0
        fi
    done

    return 1
}

# Clean only kernel build artifacts, preserving compile_commands.json and other files
clean_build_artifacts() {
    info "Cleaning kernel build artifacts..."
    rm -f *.o *.ko *.mod *.mod.c *.mod.o .*.cmd .*.o.cmd 2>/dev/null
    rm -f Module.symvers modules.order .Module.symvers.cmd .modules.order.cmd 2>/dev/null
    rm -f .module-common.o ..module-common.o.cmd 2>/dev/null
}

# Print banner
print_banner() {
    printf "\n"
    printf "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"
    printf "${CYAN}  Kernel Module Test Environment${NC}\n"
    printf "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"
    printf "\n"
}

# Print help
print_help() {
    printf "\n"
    printf "${CYAN}Useful commands:${NC}\n"
    printf "  dmesg              - View kernel messages\n"
    printf "  dmesg -w           - Watch kernel messages live\n"
    printf "  dmesg | tail -30   - Last 30 kernel messages\n"
    printf "  lsmod              - List loaded modules\n"
    printf "  rmmod %s    - Unload the module\n" "$MODULE_NAME"
    printf "  insmod %s.ko - Reload the module\n" "$MODULE_NAME"
    printf "  make               - Rebuild the module\n"
    printf "  poweroff           - Shutdown VM\n"
    printf "  exit               - Same as poweroff\n"
    printf "\n"
}

# Main
main() {
    print_banner

    get_module_name

    # Check if we have a module to load (run.sh mode vs shell.sh mode)
    if [ -z "$MODULE_NAME" ]; then
        info "Shell mode - no module specified"
        info "Use 'poweroff' or Ctrl+A, X to exit"
        exec /bin/sh
    fi

    info "Module: $MODULE_NAME"

    # Share should already be mounted by get_module_name
    if ! mountpoint -q "$MOUNT_POINT"; then
        error "Shared folder not mounted"
        error "Make sure you're using run.sh, not shell.sh"
        exec /bin/sh
    fi

    cd "$MOUNT_POINT" || {
        error "Failed to cd to $MOUNT_POINT"
        exec /bin/sh
    }

    info "Working directory: $(pwd)"

    # Smart rebuild
    KO_FILE="${MODULE_NAME}.ko"
    if needs_rebuild "$KO_FILE"; then
        info "Rebuild needed, cleaning build artifacts..."
        clean_build_artifacts
        if make; then
            info "Build successful"
        else
            error "Build failed!"
            print_help
            exec /bin/sh
        fi
    else
        info "No rebuild needed (module up to date)"
    fi

    # Check if module file exists
    if [ ! -f "$KO_FILE" ]; then
        error "Module file not found: $KO_FILE"
        error "Check that MODULE_NAME matches the .ko filename"
        print_help
        exec /bin/sh
    fi

    # Load module
    info "Loading module: $KO_FILE"
    if insmod "$KO_FILE"; then
        info "Module loaded successfully"
    else
        error "Failed to load module"
        warn "Check dmesg for details"
    fi

    # Show dmesg output
    printf "\n"
    printf "${CYAN}━━━━━━━━━━━━━━━━━━━ dmesg (last 20 lines) ━━━━━━━━━━━━━━━━━━━${NC}\n"
    dmesg | tail -20
    printf "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"

    print_help

    # Drop to shell
    exec /bin/sh
}

main
