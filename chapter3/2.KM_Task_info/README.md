# Task Info Kernel Module

A Linux kernel module that exposes process information via the `/proc` filesystem.

## Overview

This module creates a `/proc/pid` entry that allows you to:

1. **Write** a PID to query
2. **Read** information about that process (command name, PID, state)

This is an exercise from Chapter 3 of "Operating System Concepts" (the dinosaur book).

## Usage

### Load the module

```bash
insmod task_info.ko
```

### Query a process

```bash
# Write a PID to the proc entry
echo 1 > /proc/pid

# Read the process information
cat /proc/pid
```

**Example output:**

```
command = [systemd]
pid = [1]
state = [1]
```

### Query any running process

```bash
# Get your shell's PID
echo $$ > /proc/pid
cat /proc/pid

# Query a specific process
pidof sshd > /proc/pid
cat /proc/pid
```

### Unload the module

```bash
rmmod task_info
```

## Process States

The `state` field corresponds to Linux task states:

| Value | State                |
| ----- | -------------------- |
| 0     | TASK_RUNNING         |
| 1     | TASK_INTERRUPTIBLE   |
| 2     | TASK_UNINTERRUPTIBLE |
| 4     | \_\_TASK_STOPPED     |
| 8     | \_\_TASK_TRACED      |

## Implementation Details

- Creates `/proc/pid` with read/write permissions (0666)
- Uses `find_vpid()` and `pid_task()` to look up the task struct
- Reads `task->comm` (command name), `task->pid`, and `task->__state`
- Memory for user input is allocated with `kmalloc()` and freed after parsing

## Building

```bash
# In the VM
make

# On host (for LSP support only, won't run on host)
bear -- make -C /lib/modules/$(uname -r)/build M=$PWD
```

## Files

| File          | Description                       |
| ------------- | --------------------------------- |
| `task_info.c` | Module source code                |
| `Makefile`    | Kernel module build configuration |
