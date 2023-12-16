# Simple Memory Allocator
This is a basic memory allocator implementation that provides functionalities for dynamic memory allocation (`malloc`), reallocation (`realloc`), and deallocation (`free`). It is designed for educational purposes and simplicity.

## Features
- **malloc :** Allocates a block of memory.
- **realloc:** Changes the size of the memory block.
- **free   :** Releases the allocated memory.

## Thread Safety
The allocator incorporates mutex locks to ensure thread safety. This prevents multiple processes from concurrently allocating or deallocating memory in the same region.
