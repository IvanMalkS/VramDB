const express = require('express');
const net = require("net");
const { spawn, exec } = require('child_process');
const os = require('os');
const path = require('path');
const fs = require('fs');
const readline = require('readline');

// Initializing kernel
const platform = os.platform();
const runCommand = (command, cwd, callback) => {
    exec(command, { cwd, shell: true }, (error, stdout, stderr) => {
        if (error) {
            console.log(`exec error: ${error}`);
            callback(error);
        } else {
            console.log(`stdout: ${stdout}`);
            console.log(`stderr: ${stderr}`);
            callback(null);
        }
    });
};

const kernelPath = path.resolve(__dirname, '..', '..', 'kernel');
const buildPath = path.join(kernelPath, 'build');
if (!fs.existsSync(buildPath)) {
    fs.mkdirSync(buildPath);
}

let vramDBProcess

if (platform === 'win32' || platform === 'win64') {
    runCommand('cmake -G "Ninja" ..', buildPath, (error) => {
        if (error) return;
        runCommand('ninja', buildPath, (error) => {
            if (error) return;
            vramDBProcess = spawn('.\\VramDB.exe', { cwd: buildPath, shell: true });

        });
    });
} else if (platform === 'linux' || platform === 'darwin') {
    runCommand('cmake ..', buildPath, (error) => {
        if (error) return;
        runCommand('make', buildPath, (error) => {
            if (error) return;
            // Spawn the process instead of exec
            vramDBProcess = spawn('./VramDB', { cwd: buildPath });

        });
    });
}

// Setting up the TCP server
const tcpServer = net.createServer((socket) => {
    console.log('Client connected');

    // Handle data received from the VramDB process
    if (vramDBProcess) {
        vramDBProcess.stdout.on('data', (data) => {
            console.log(`VramDB Output: ${data}`);
            socket.write(data);
        });

        vramDBProcess.stderr.on('data', (data) => {
            console.error(`VramDB Error: ${data}`);
            socket.write(data);
        });

        vramDBProcess.on('close', (code) => {
            console.log(`VramDB process exited with code ${code}`);
            socket.end();
        });
    }

    // Function to send commands to the VramDB process
    const sendCommand = (command) => {
        if (vramDBProcess) {
            vramDBProcess.stdin.write(`${command}\n`);
        } else {
            console.log('VramDB process is not running');
        }
    };

    // Handle socket data for TCP commands
    socket.on('data', (data) => {
        const command = data.toString().trim();
        sendCommand(command);
    });

    socket.on('end', () => {
        console.log('Client disconnected');
    });

    socket.on('error', (err) => {
        console.error('Socket error:', err.message);
    });
});

// Set up HTTP server with Express
const app = express();
app.use(express.json());

// API endpoint to handle commands
app.post('/command', (req, res) => {
    const { action, key, value } = req.body;

    let command;
    switch (action) {
        case 'create':
            command = `create ${key} ${value}`;
            break;
        case 'update':
            command = `update ${key} ${value}`;
            break;
        case 'delete':
            command = `delete ${key}`;
            break;
        case 'show':
            command = `show ${key}`;
            break;
        default:
            return res.status(400).send('Invalid action. Use create/update/delete/show.');
    }

    // Send command to the VramDB process via TCP
    const client = new net.Socket();
    client.connect(8080, 'localhost', () => {
        client.write(command);
    });

    client.on('data', (data) => {
        res.send(data.toString());
        client.destroy();
    });

    client.on('error', (err) => {
        console.error('TCP Client Error:', err);
        res.status(500).send('Error communicating with VramDB.');
    });

    client.on('end', () => {
        console.log('TCP connection ended');
    });
});

// Start the TCP server
tcpServer.listen(8080, () => {
    console.log('TCP server listening on port 8080');
});

// Start the HTTP server
const PORT = 3000;
app.listen(PORT, () => {
    console.log(`HTTP server listening on port ${PORT}`);
});
