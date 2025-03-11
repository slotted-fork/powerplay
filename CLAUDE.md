# PowerPlay Development Guidelines

This file serves as a memory for Claude and other AI coding assistants. When asked to "remember" something about code style or project preferences, Claude will add those details to this file.

## Build Commands
- `make` - Build all targets (sparkshift, amptrack)
- `make install` - Install binaries to $(PREFIX)/bin/
- `make clean` - Remove compiled objects and executables

## Dependencies
- libmodbus - Required for Modbus communication

## Code Style Guidelines
- Language: C99 standard
- Indentation: Spaces (not tabs)
- Naming: snake_case for functions/variables, UPPER_CASE for constants
- Typedefs: Use `typedef enum` for parameter definitions
- Error handling: Return boolean values (true=error, false=success)
- Functions: Declare prototypes with descriptive names
- Comments: Use standard C-style comments /* ... */ (not Doxygen-style)
- Function comments: Include parameter and return value descriptions in plain text
- Document environment variables and configuration

## Coding Practices
- Use comprehensive compiler warnings (Wall, Wextra, Wpedantic)
- Validate function inputs and handle errors explicitly
- Flush error messages to stderr
- Maintain single-header library pattern with POWERPLAY_IMPLEMENTATION until complexity requires separation
- Use structs for grouping related data
- Access configuration via environment variables
- Check return codes from all function calls

## Project Structure
- powerplay.h - Core library and data structures
- sparkshift.c - Manages EV charging based on excess power
- amptrack.c - Monitors power consumption and production