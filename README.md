
# File System Documentation

## Overview
This file system is a client-server-based implementation that allows for file management operations such as creating files or directories, reading files, streaming audio files, and more. The system is distributed across a **Naming Server**, **Storage Servers**, and a **Client**. It supports functions of File operations: Read, write, create, delete, copy, and stream files,
Scalability: Dynamically add storage servers to the system,
Error handling with detailed error codes,
Asynchronous and synchronous file operations,
Redundant data replication across multiple storage servers,
optimized file search using Tries and LRU caching,
streaming audio support for clients AND  error handling through descriptive error codes, making it robust and user-friendly.

---

## Components

### 1. **Client**
The client interacts with the Naming Server and Storage Servers to perform file system operations. It sends requests, handles responses, and processes user commands via a terminal interface.

### 2. **Naming Server**
The Naming Server acts as the central coordinator. It manages metadata about the file structure, assigns Storage Servers for file operations, and resolves paths. It also handles error cases and ensures clients are redirected appropriately.

### 3. **Storage Server**
The Storage Server is responsible for actual file storage, streaming, and read/write operations. It communicates with the client for file data exchange.

---

## Supported Operations

### **CREATE**
- **Syntax**: `CREATE FILE <PATH> <NAME>` or `CREATE DIR <PATH> <NAME>`
- Creates a file or directory at the specified path.
- **Error Codes**:
  - `1001`: Cannot create a directory inside a file.
  - `1002`: Invalid path.
  - `1004`: Resource already exists.

### **READ**
- **Syntax**: `READ <PATH>`
- Reads the content of a file.
- **Error Codes**:
  - `1002`: Invalid path.
  - `3`: File not found or invalid.

### **STREAM**
- **Syntax**: `STREAM <PATH>`
- Streams an audio file from the Storage Server to the client using `mpv`.

### **COPY**
- **Syntax**: `COPY <SOURCE_PATH> <DESTINATION_PATH>`
- Copies a file or directory to the specified destination.

### **DELETE**
- **Syntax**: `DELETE <PATH>`
- Deletes a file or directory.

### **LIST**
- **Syntax**: `LIST <PATH>`
- Lists the contents of a directory.

---

## Error Codes
- **`1001`**: Cannot create a directory inside a file.
- **`1002`**: Invalid path or name.
- **`1004`**: Resource already exists.
- **`1005`**:No servers available for file storage.
- **`400`**: Invalid command.
- **`3`**: File not found or invalid.
- **`404`**:- File/Path Not Found:

Used when a file or path does not exist in the system.
- **`409`**: - Duplicate Resource:

Used when attempting to create a file or directory that already exists.
- **`500`**: - Internal Server Error:

---


## Naming Server Workflow

1. **Command Handling**:
   - Processes commands like `CREATE`, `READ`, `STREAM`, etc.
   - Routes valid requests to the appropriate Storage Server.

2. **Error Handling**:
   - Invalid commands or requests generate error codes, which are sent back to the client.
   - Example: Creating a directory inside a file returns `1001: Cannot create directory inside a file`.

3. **Server Assignments**:
   - Assigns main and backup Storage Servers for new files.
   - Uses a MinHeap to manage server load.

4. **Trie-Based Metadata**:
   - Maintains a trie data structure to store the file system hierarchy.
   - Validates paths before processing commands.

---
## HOW OUR DIRECTORY AND FILE ARE  STORED

1. **TRIE IMPLEMENTATION**:
   - Directories once created are only made to be prsent on the trie, are actually not there at the server. When i try to put a file inside my folder, it just finds the location using my trie, but there is no actual physical directory, just illussion for the client. Through the metadata of the file we are able to figure out if its a directory or file, and hence creating files is never creating a problem.

## Storage Server Workflow

1. **File Operations**:
   - Reads, writes, streams, and validates files.
   - Supports chunked file transmission for large files.

2. **Streaming**:
   - Sends audio files in chunks to the client.
   - Ends gracefully with a `STOP` message.

3. **Error Handling**:
   - Returns descriptive error messages to the Naming Server or client.
   - Examples:
     - `stat failed`: File not found or inaccessible.
     - `fopen failed`: Unable to open the file.

4. **Concurrency**:
   - Handles multiple client requests using threads.
   - Ensures synchronization using mutexes.

---

## Client Workflow

1. **Command Input**:
   - Accepts user commands via terminal.
   - Redirects back to the command prompt on invalid inputs or errors.

2. **Error Handling**:
   - Redirects to the command prompt for errors like `1001` or `1004`.
   - Example:
     - **Input**: `CREATE DIR /invalid/path name`
     - **Output**: `1002: Invalid path. Please try again.`

3. **Interactions**:
   - Communicates with the Naming Server for metadata and server assignments.
   - Communicates with Storage Servers for actual file operations.

---

## Example Usage

1. **Creating a File**:
   ```
   Client Input: CREATE FILE /home/docs report.txt
   NM Response: 127.0.0.1 35000 1
   ```

2. **Streaming a File**:
   ```
   Client Input: STREAM /home/music/track.mp3
   NM Response: 127.0.0.1 35000
   Streaming...
   ```

3. **Error Handling**:
   ```
   Client Input: CREATE DIR /home/music track.mp3
   NM Response: 1001: Cannot create directory inside a file
   ```

---

## Technologies and Features

- **Concurrency**:
  - Uses pthreads for handling multiple client requests.
  - Mutexes ensure synchronization.

- **Error Handling**:
  - Robust error codes with descriptive messages.

- **Trie Data Structure**:
  - Efficient path validation and file system hierarchy management.

- **Chunked File Transmission**:
  - Handles large files seamlessly.

- **Streaming**:
  - Real-time streaming with `mpv`.

---

## Future Improvements

- **Authentication**:
  - Add user authentication for secure access.
  
- **File Locking**:
  - Implement file locking mechanisms to handle concurrent writes.

- **Scalability**:
  - Optimize server assignment for large-scale systems.

- **Monitoring**:
  - Add monitoring tools for server health and performance.

---

This README provides a comprehensive overview of the file system, ensuring seamless understanding for developers and maintainers.
