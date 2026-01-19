# Chapter 2 - Operating System Concepts Book (10th edition) - Programming Projects

This directory contains four Linux kernel modules demonstrating basic kernel programming concepts.

## Modules Overview

### 1. hello.c - /proc File System Module

A kernel module that creates a `/proc/hello` entry and demonstrates:

- Creating `/proc` filesystem entries
- Implementing read operations for `/proc` entries
- Using `proc_ops` structure (modern kernel approach)
- User-space to kernel-space data transfer with `copy_to_user()`

**Features:**

- Creates `/proc/hello` entry when loaded
- Returns "Hello World\n" when read via `cat /proc/hello`
- Properly handles multiple reads using completion flag

### 2. simple.c - Basic Kernel Information Module

A simple kernel module that demonstrates:

- Basic module initialization and cleanup
- Accessing kernel constants and variables
- Using kernel timing mechanisms (jiffies)
- Mathematical operations in kernel space

**Features:**

- Displays `GOLDEN_RATIO_PRIME` constant value
- Shows system timer frequency (`HZ`)
- Tracks module loading/unloading time using jiffies
- Calculates GCD of two numbers (3300 and 24)
- Reports total time module was loaded

### 3. jiffies_mod.c - Current Jiffies /proc Module

A kernel module that creates a `/proc/jiffies` entry to display current kernel timing information:

- Real-time access to kernel timer tick counter
- Demonstrates dynamic `/proc` content that changes on each read
- Shows current system state through `/proc` interface

**Features:**

- Creates `/proc/jiffies` entry when loaded
- Returns current jiffies value each time it's read
- Shows real-time kernel timer ticks

### 4. time_elapsed.c - Elapsed Time Tracking Module

A comprehensive timing module that creates a `/proc/seconds` entry showing detailed timing information:

- Tracks module load time and calculates elapsed time
- Demonstrates time conversion (jiffies to seconds/milliseconds)
- Shows multiple time representations in a single read

**Features:**

- Creates `/proc/seconds` entry when loaded
- Shows initial jiffies value when module was loaded
- Displays current jiffies and elapsed jiffies
- Converts elapsed time to both seconds and milliseconds
- Provides comprehensive timing analysis

## Building the Modules

### Prerequisites

- Linux kernel headers: `sudo apt install linux-headers-$(uname -r)`
- Build tools: `sudo apt install build-essential`

### Compilation

To build a specific module, edit the first line of `Makefile`:

```makefile
obj-m += hello.o         # For hello module
obj-m += simple.o        # For simple module
obj-m += jiffies_mod.o   # For jiffies module
obj-m += time_elapsed.o  # For time elapsed module
```

**Note:** Only uncomment one line at a time to build individual modules.

Then run:

```bash
make
```

### Generate compile_commands.json (for LSP support)

```bash
bear -- make -C /lib/modules/$(uname -r)/build M=$PWD
```

## Usage Examples

### hello.c Module

```bash
# Load the module
sudo insmod hello.ko

# Read from /proc entry
cat /proc/hello
# Output: Hello World

# Check if entry exists
ls -la /proc/hello

# Unload the module
sudo rmmod hello

# Verify entry is removed
ls -la /proc/hello  # Should show "No such file or directory"
```

### simple.c Module

```bash
# Load the module
sudo insmod simple.ko

# Check kernel logs for output
dmesg | tail -10

# Unload the module (after some time to see timing difference)
sudo rmmod simple

# Check kernel logs again for exit information
dmesg | tail -10
```

Expected output in kernel logs:

```
Loading Kernel Module
GOLDEN_RATIO_PRIME is: [value]
Value of jiffies in init is: [value]
Value of HZ is: [value]
Removing Kernel Module
GCD of 3300 and 24 is: 12
Value of jiffies in exit is: [value]
Total timer interrupts since loading module: [difference]
Total time elapsed since loading module (in seconds): [time]
```

### jiffies_mod.c Module

```bash
# Load the module
sudo insmod jiffies_mod.ko

# Read current jiffies (will show different values each time)
cat /proc/jiffies
# Output: The current jiffies value is: [current_value]

# Read again to see the value change
cat /proc/jiffies
# Output: The current jiffies value is: [different_value]

# Unload the module
sudo rmmod jiffies_mod
```

### time_elapsed.c Module

```bash
# Load the module
sudo insmod time_elapsed.ko

# Read timing information immediately after loading
cat /proc/seconds
# Output:
# Module loaded at jiffies: [init_value]
# Current jiffies: [current_value]
# Elapsed jiffies: [small_difference]
# Elapsed time: 0 seconds ([milliseconds] ms)

# Wait a few seconds and read again
sleep 5
cat /proc/seconds
# Output:
# Module loaded at jiffies: [init_value]
# Current jiffies: [current_value]
# Elapsed jiffies: [larger_difference]
# Elapsed time: 5 seconds ([milliseconds] ms)

# Unload the module
sudo rmmod time_elapsed
```

## Key Learning Points

### Modern Kernel Development

- Use `struct proc_ops` instead of `struct file_operations` for `/proc` entries
- Proper error handling with `copy_to_user()` return values
- Module metadata with `MODULE_LICENSE`, `MODULE_AUTHOR`, `MODULE_DESCRIPTION`

### Kernel Programming Concepts

- **jiffies**: Kernel's global timer tick counter that increments with each timer interrupt
- **HZ**: System timer frequency (ticks per second) - typically 250, 1000, or 1024 Hz
- **printk()**: Kernel's equivalent to printf() for logging
- **KERN_INFO**: Log level for informational messages
- **snprintf()**: Safe string formatting function for kernel space
- **Dynamic /proc content**: Files that show different content on each read
- **Time conversion**: Converting between jiffies, seconds, and milliseconds

### Memory Management

- Always check return values from `copy_to_user()`
- Handle multiple read attempts properly (completion flags)
- Clean resource allocation/deallocation in init/exit functions

## Troubleshooting

### Common Issues

1. **Empty output from `/proc/hello`, `/proc/jiffies`, or `/proc/seconds`**:
   - Ensure using `struct proc_ops` not `struct file_operations`
   - Check that `.proc_read` is used instead of `.read`
   - Verify completion flag logic is correct

2. **Module won't load**:
   - Check kernel log: `dmesg | tail`
   - Verify kernel headers match running kernel: `uname -r`
   - Ensure no conflicting `/proc` entries exist

3. **Permission denied**:
   - Use `sudo` for insmod/rmmod operations
   - Check if another module with same name is loaded: `lsmod | grep module_name`

4. **Timing values seem incorrect**:
   - Check system's HZ value: `grep 'CONFIG_HZ=' /boot/config-$(uname -r)`
   - Verify jiffies calculations for your system's timer frequency

### Debugging Tips

- Use `printk(KERN_DEBUG "debug message")` for debugging
- Check module status: `lsmod | grep module_name`
- View detailed module info: `modinfo module_name.ko`
- Monitor kernel logs in real-time: `dmesg -w`

## Clean Up

```bash
# Remove compiled files
make clean

# Remove any loaded modules
sudo rmmod hello simple jiffies_mod time_elapsed 2>/dev/null || true

# Verify all proc entries are removed
ls -la /proc/{hello,jiffies,seconds} 2>/dev/null || echo "All proc entries cleaned up"
```
