# VM Test Environment for Kernel Modules

Minimal QEMU setup for safely testing kernel modules without risking host system.

## Quick Start (After Setup)

```bash
# Test a module
./vm/run.sh chapter2 time_elapsed
./vm/run.sh chapter3/2.KM_Task_info task_info

# Open VM shell (no module loading)
./vm/shell.sh
```

---

## Initial Setup (From Scratch)

### Prerequisites

Install QEMU on your host system:

```bash
# Arch Linux
sudo pacman -S qemu-full

# Ubuntu/Debian
sudo apt install qemu-system-x86 qemu-utils

# Fedora
sudo dnf install qemu-kvm qemu-img
```

Ensure KVM is available (for hardware acceleration):

```bash
# Check if KVM is supported
lsmod | grep kvm

# Your user should have access to /dev/kvm
ls -la /dev/kvm
# If not, add yourself to the kvm group:
# sudo usermod -aG kvm $USER
```

### Step 1: Download Alpine Linux ISO

Download the **Virtual** edition (optimized for VMs):

```bash
cd vm/
wget https://dl-cdn.alpinelinux.org/alpine/v3.21/releases/x86_64/alpine-virt-3.21.3-x86_64.iso
```

Or visit https://alpinelinux.org/downloads/ and download the "Virtual" x86_64 ISO.

### Step 2: Create the VM Disk Image

```bash
# Create a 2GB qcow2 disk image (grows dynamically, starts small)
qemu-img create -f qcow2 alpine.img 2G
```

### Step 3: Install Alpine Linux

Boot from the ISO to install:

```bash
qemu-system-x86_64 \
  -m 1G \
  -smp 2 \
  -enable-kvm \
  -cpu host \
  -drive file=alpine.img,format=qcow2 \
  -cdrom alpine-virt-*.iso \
  -boot d \
  -nographic
```

Once booted, login as `root` (no password) and run:

```bash
setup-alpine
```

Follow the prompts:

- Keyboard layout: `us` (or your preference)
- Hostname: `alpine` (or anything)
- Network: `eth0`, DHCP
- Root password: **set a password** (you'll need it)
- Timezone: your timezone
- Proxy: `none`
- Mirror: pick one (or just press Enter for default)
- SSH server: `none` (not needed)
- Disk: `sda`, `sys` mode
- Confirm erase: `y`

After installation completes:

```bash
poweroff
```

### Step 4: Install Build Dependencies

Boot the installed system (without the ISO):

```bash
qemu-system-x86_64 \
  -m 1G \
  -smp 2 \
  -enable-kvm \
  -cpu host \
  -drive file=alpine.img,format=qcow2 \
  -nographic
```

Login as root and install required packages:

```bash
# Update package index
apk update

# Install kernel headers and build tools
apk add linux-virt-dev build-base

# Verify installation
ls /lib/modules/$(uname -r)/build
# Should show the kernel build directory
```

Then shutdown:

```bash
poweroff
```

### Step 5: Configure Autologin and Init Script

Boot the VM with the `vm/` folder shared:

```bash
# Run from the project root directory
qemu-system-x86_64 \
  -m 1G -smp 2 -enable-kvm -cpu host \
  -drive file=vm/alpine.img,format=qcow2 \
  -nographic \
  -virtfs local,path=vm,mount_tag=hostshare,security_model=mapped-xattr
```

Login as root, then run:

```bash
# Mount the shared folder
mkdir -p /mnt/host
mount -t 9p -o trans=virtio,version=9p2000.L hostshare /mnt/host

# Run the setup script
sh /mnt/host/setup-guest.sh

# Shutdown
poweroff
```

### Step 6: Done!

Your VM is now configured. Test it:

```bash
./vm/run.sh chapter2 time_elapsed
```

---

## Usage

### Testing a Module

```bash
./vm/run.sh <module_directory> <module_name>
```

**Examples:**

```bash
./vm/run.sh chapter2 time_elapsed
./vm/run.sh chapter3/2.KM_Task_info task_info

# From within a chapter directory
cd chapter2
../vm/run.sh . simple
```

**What happens:**

1. Launches the QEMU VM
2. Shares the module directory with the VM via 9p virtfs
3. Auto-logins as root
4. **Smart rebuild**: Only recompiles if `.c` files are newer than `.ko`
5. Loads the module with `insmod`
6. Displays `dmesg` output
7. Drops to an interactive shell

### Shell Mode (No Module)

```bash
./vm/shell.sh
```

Boots the VM without loading any module. Useful for:

- Debugging
- Installing additional packages
- Manual testing

### Inside the VM

| Command              | Description             |
| -------------------- | ----------------------- |
| `dmesg`              | View kernel messages    |
| `dmesg \| tail -30`  | Last 30 kernel messages |
| `lsmod`              | List loaded modules     |
| `rmmod <module>`     | Unload a module         |
| `insmod <module>.ko` | Load a module           |
| `make`               | Rebuild the module      |
| `poweroff`           | Shutdown VM             |
| `Ctrl+A, X`          | Force quit QEMU         |

---

## Files

```
vm/
├── alpine.img         # Alpine Linux qcow2 image (you create this)
├── alpine-virt-*.iso  # Alpine installer ISO (download this)
├── run.sh             # Main launcher (builds + loads module)
├── shell.sh           # Boot VM without loading module
├── guest-init.sh      # Runs inside VM on autologin
├── setup-guest.sh     # One-time VM setup script
└── README.md          # This file
```

---

## Troubleshooting

### "KVM not available, VM will be slower"

Your user doesn't have access to `/dev/kvm`. Fix:

```bash
sudo usermod -aG kvm $USER
# Then logout and login again
```

### Module fails to load with "Invalid module format"

The module was compiled for a different kernel version. Make sure:

1. You're compiling **inside** the VM (not on the host)
2. The `linux-virt-dev` package matches your running kernel

Check versions:

```bash
uname -r                              # Running kernel
apk info linux-virt-dev | head -1     # Installed headers
```

### "Failed to mount shared folder"

The 9p filesystem isn't available. This happens if:

- You're using `shell.sh` instead of `run.sh`
- The VM kernel doesn't have 9p support (Alpine virt kernel has it by default)

### Build fails with "kernel configuration is invalid"

Run inside the VM:

```bash
cd /lib/modules/$(uname -r)/build
make oldconfig && make prepare
```

### Autologin not working

Re-run the setup:

```bash
./vm/shell.sh
# Login manually, then:
mount -t 9p -o trans=virtio,version=9p2000.L hostshare /mnt/host
cp /mnt/host/guest-init.sh /root/init.sh
chmod +x /root/init.sh
# Verify /etc/inittab has the autologin line:
grep ttyS0 /etc/inittab
# Should show: ttyS0::respawn:/sbin/getty -n -l /root/init.sh ttyS0 115200 vt100
poweroff
```
