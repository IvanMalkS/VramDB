# VramDB: Your NoSQL Key-Value Database Based on Video Card Memory 🚀

Welcome to VramDB! If you're passionate about databases, graphics programming, or just want to explore the exciting world of GPU-based data storage, you’re in the right place!
## What is VramDB? 🤔

VramDB is a NoSQL database that leverages the power of your video card's memory to store data as key-value pairs. Why settle for traditional storage when you can harness the raw power of your GPU?

This database is perfect for developers looking to speed up data processing and enhance their applications while having a blast doing it!
## Features 🌟

  - Key-Value Storage: Store and retrieve data effortlessly using simple keys.
  - GPU Acceleration: Maximize performance by utilizing your graphics card’s memory.
  - Cross-Platform Support: Built with Vulkan, making it compatible with various operating systems.
  - Easy Integration: Access your data via HTTP or TCP for seamless communication with your applications.

## Requirements 📦

To get started, you'll need the following:

  - Vulkan SDK: Download it from the Vulkan SDK Website.
  - GCC: Ensure you have the GNU Compiler Collection installed.
  - CMake: Required for building the project.
  - ninja (for windows)
  - Node.js 16+ (Optional): Use this if you want to integrate with Node.js applications.

## Installation 🛠️

For the best experience, build the kernel and use it in your own application. Here’s how to get started:

### Building only kernel
1. Clone the repository:

    `git clone https://github.com/IvanMalkS/VramDB
    cd VramDB`

2. Build the Kernel:

For linux:
    `cd kernel
    mkdir build
    cd build
    cmake ..
    make`
For windows: 
    `cd kernel
    mkdir build
    cd build
    cmake -G "Ninja" ..
    ninja
    .\VramDB.exe
    `

Run the Application:

For linux:
    `./VramDB`
    
For windows:
    `.\VramDB.exe`

### By api
1. Clone the repository:

    `git clone https://github.com/IvanMalkS/VramDB
    cd VramDB`

2. Install node modules

    `npm install`

3. Start application

    `npm start prod`

## Usage 🎮

Using VramDB is as simple as pie! Once your application is up and running, you can interact with it via HTTP requests or TCP commands.
Example Commands:

- Create: Store a new key-value pair.
- Update: Change the value associated with a key.
- Delete: Remove a key-value pair.
- Show: Retrieve the value associated with a key.

### What for? For fun! 🫠

This project is licensed under the MIT License
Happy coding! 🎉
