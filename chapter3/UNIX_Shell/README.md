# OSH - Operating Systems Shell

A custom Unix shell implementation written in C, featuring command execution, I/O redirection, piping, background processes, and command history.

## Table of Contents

- [Features](#features)
- [Setup](#setup)
- [Usage](#usage)
- [Examples](#examples)
- [Limitations](#limitations)
- [Project Structure](#project-structure)
- [Technical Details](#technical-details)

## Features

### Core Functionality

- **Command Execution**: Execute any standard Unix command with arguments
- **I/O Redirection**:
  - Input redirection using `<`
  - Output redirection using `>`
- **Piping**: Support for multiple pipes to chain commands
- **Background Processes**: Run commands in the background using `&`
- **Command History**: Recall the last executed command with `!!`
- **Built-in Commands**:
  - `exit` - Exit the shell
  - `clear` - Clear the terminal screen

### Advanced Features

- Multiple pipe support (e.g., `cmd1 | cmd2 | cmd3`)
- Combined I/O redirection with pipes
- Quoted string support (single and double quotes)
- Debug mode for development and troubleshooting

## Setup

### Prerequisites

- GCC compiler
- Make build system
- Linux/Unix operating system
- Standard C library

### Installation

1. **Clone or navigate to the project directory**:

   ```bash
   cd OS_Projects/chapter3/UNIX_Shell
   ```

2. **Build the project**:

   ```bash
   make
   ```

3. **Run the shell**:
   ```bash
   ./bin/shell
   ```

### Build Options

```bash
# Standard build
make

# Build with debug output
make debug

# Build with optimizations
make release

# Build and run
make run

# Build with debug and run
make run-debug

# Clean build artifacts
make clean

# View all available targets
make help
```

## Usage

### Starting the Shell

```bash
./bin/shell
```

You will see the prompt:

```
osh>
```

### Basic Commands

**Simple command execution:**

```bash
osh> ls
osh> pwd
osh> echo hello world
```

**Commands with arguments:**

```bash
osh> ls -la
osh> grep pattern file.txt
osh> cat file1.txt file2.txt
```

**Commands with quoted strings:**

```bash
osh> echo "hello world"
osh> echo "hello world" "another string"
osh> grep "search pattern" file.txt
osh> echo 'single quotes work too'
```

### I/O Redirection

**Output redirection:**

```bash
osh> ls -la > output.txt
osh> echo "Hello" > greeting.txt
```

**Input redirection:**

```bash
osh> sort < unsorted.txt
osh> wc -l < file.txt
```

**Both input and output:**

```bash
osh> sort < input.txt > sorted.txt
```

### Piping

**Single pipe:**

```bash
osh> ls -la | grep shell
osh> cat file.txt | wc -l
```

**Multiple pipes:**

```bash
osh> ls -la | grep c | wc -l
osh> cat file.txt | sort | uniq | wc -l
```

**Pipes with redirection:**

```bash
osh> cat < input.txt | grep pattern | sort > output.txt
```

### Background Processes

**Run command in background:**

```bash
osh> sleep 10 &
osh> find / -name "*.txt" &
```

Note: The shell will continue to accept new commands while background processes run.

### Command History

**Repeat last command:**

```bash
osh> ls -la
osh> !!
Running command: ls -la
```

### Built-in Commands

**Exit the shell:**

```bash
osh> exit
```

**Clear the screen:**

```bash
osh> clear
```

## Examples

### Example Session

```bash
$ ./bin/shell
osh> ls
file1.txt  file2.txt  data.csv

osh> ls -la > listing.txt
osh> cat listing.txt
total 24
-rw-r--r-- 1 user user  156 Jan 22 10:30 file1.txt
-rw-r--r-- 1 user user  892 Jan 22 10:31 file2.txt
-rw-r--r-- 1 user user 1024 Jan 22 10:32 data.csv

osh> cat file1.txt | grep "error" | wc -l
5

osh> sort < data.csv | uniq > unique_data.csv

osh> !!
Running command: sort < data.csv | uniq > unique_data.csv

osh> sleep 30 &
osh> echo "Background process running..."
Background process running...

osh> exit
```

### Data Processing Example

```bash
osh> cat large_file.txt | sort | uniq -c | sort -rn > word_frequency.txt
```

## Limitations

### Current Limitations

1. **Redirect Position**: No validation for redirect position in pipes
   - `ls | cat > file.txt | grep pattern` may not work as expected
   - Best practice: Use `<` at the beginning and `>` at the end only

2. **Command Length**: Maximum command length is 1024 characters (BUFFER_LENGTH)

3. **Argument Count**: Maximum of 80 arguments per command (MAXLINE)

4. **History**: Only the last command is stored (no full history navigation)

5. **No Job Control**:
   - Cannot bring background processes to foreground
   - No `jobs`, `fg`, or `bg` commands

6. **Signal Handling**: Limited signal handling (Ctrl+C will exit the shell)

7. **Environment Variables**:
   - No variable expansion (`$HOME`, `$PATH`, etc.)
   - No variable assignment

8. **Wildcards**: No globbing support (`*.txt` won't expand)

9. **Command Substitution**: No support for `$(command)` or backticks

10. **Logical Operators**: No support for `&&`, `||`, or `;`

11. **Redirection Append**: Only truncate mode (`>`), no append mode (`>>`)

12. **Nested Quotes**: Mixing quote types in complex ways is not supported

- `echo "She said 'hello'"` may not work as expected

### Known Issues

- Empty input (just pressing Enter) shows an error message
- No error recovery for malformed pipe chains
- Background process completion is not reported

## Project Structure

```
OS_Projects/chapter3/UNIX_Shell/
├── bin/              # Compiled executable
│   └── shell
├── obj/              # Object files
│   ├── main.o
│   └── shell.o
├── main.c            # Entry point and main loop
├── shell.c           # Core shell functionality
├── shell.h           # Header file with declarations
├── Makefile          # Build configuration
└── README.md         # This file
```

### File Descriptions

- **main.c**: Contains the main loop, handles user input, and manages the shell prompt
- **shell.c**: Implements tokenization, parsing, and command execution
- **shell.h**: Defines the Command structure and function prototypes
- **Makefile**: Automated build system with multiple targets

## Technical Details

### Command Processing Pipeline

1. **Input Reading**: User input is read via `fgets()`
2. **Tokenization**: Input is split into tokens (space-delimited)
3. **Parsing**: Tokens are analyzed for special characters (`|`, `>`, `<`, `&`)
4. **Execution**: Commands are executed using `fork()` and `execvp()`

### Process Management

- Each command is executed in a child process created with `fork()`
- Parent process waits for child completion (unless background mode)
- Pipes are implemented using `pipe()` system call
- File descriptors are redirected using `dup2()`

### Memory Management

- Commands are stored in a static `Command` structure
- Pointers are used to reference tokens within the input buffer
- Structure is reset after each command execution

### Debugging

Enable debug mode during compilation to see detailed parsing information:

```bash
make debug
./bin/shell
```

Debug output includes:

- Token breakdown
- Argument array contents
- Redirect flags and filenames
- Pipe information
- Command arrays for piped commands

## Contributing

This is an educational project. Improvements and bug fixes are welcome.

### Development Guidelines

1. Maintain the existing code structure
2. Add debug output for new features (under `#ifdef DEBUG`)
3. Update this README with new features or changes
4. Test thoroughly with various command combinations

## License

This project is created for educational purposes as part of an Operating Systems course.

## Author

Created as part of Chapter 3 exercises - UNIX Shell implementation.

## Acknowledgments

- Based on concepts from Operating Systems textbooks
- Implements standard UNIX shell features
- Uses POSIX system calls for process and I/O management
