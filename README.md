# DOMO Port Forwarding Server
[![Version](https://img.shields.io/badge/version-1.1-blue.svg)](https://github.com/g91/DOMO-Port-Forwarding-Server.git)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

DOMO Port Forwarding Server is a robust, lightweight C++ application designed to forward TCP and UDP ports from local addresses to broadcast addresses on Linux systems. Built with performance and reliability in mind, it's perfect for network administrators and developers who need flexible port forwarding capabilities.

## Features

- **Dual Protocol Support**: Handles both TCP and UDP port forwarding
- **Multiple Port Forwarding**: Forward multiple ports simultaneously with multi-threading
- **Configuration Based**: Easy-to-use configuration file for managing forwarding rules
- **System Service Integration**: Runs as a systemd service with automatic startup
- **Comprehensive Logging**: Built-in syslog integration for monitoring and debugging
- **Broadcast Support**: Forward traffic to all broadcast addresses
- **High Performance**: Efficient C++ implementation with minimal resource usage
- **Error Handling**: Robust error handling and cleanup mechanisms

## Installation

```bash
# Clone the repository
git clone https://github.com/the1Domo/port-forwarder.git
cd port-forwarder

# Compile the application
g++ -o portforwarder portforwarder.cpp -std=c++11 -pthread

# Create configuration directory
sudo mkdir /etc/portforwarder

# Copy files to system directories
sudo cp portforwarder /usr/local/bin/
sudo cp config.conf /etc/portforwarder/
sudo cp portforwarder.service /etc/systemd/system/

# Set proper permissions
sudo chmod +x /usr/local/bin/portforwarder

# Enable and start the service
sudo systemctl enable portforwarder
sudo systemctl start portforwarder
```

## Configuration

The configuration file (`/etc/portforwarder/config.conf`) uses a simple format:

```plaintext
# LocalIP LocalPort RemotePort Protocol
192.168.1.100 8080 9090 TCP
192.168.1.100 5353 5353 UDP
```

Each line represents a forwarding rule with four components:
- **LocalIP**: The local IP address to listen on
- **LocalPort**: The local port to listen on
- **RemotePort**: The destination port for forwarding
- **Protocol**: Either TCP or UDP

## Usage

### Starting the Service
```bash
sudo systemctl start portforwarder
```

### Stopping the Service
```bash
sudo systemctl stop portforwarder
```

### Checking Status
```bash
sudo systemctl status portforwarder
```

### Viewing Logs
```bash
journalctl -u portforwarder
```

### Manual Execution
```bash
sudo portforwarder /path/to/config.conf
```

## Monitoring

The application logs all activities to syslog, which can be monitored using standard Linux tools:

- View all logs: `journalctl -u portforwarder`
- View real-time logs: `journalctl -u portforwarder -f`
- View errors only: `journalctl -u portforwarder -p err`

## Requirements

- Linux operating system
- G++ compiler with C++11 support
- systemd (for service functionality)
- Root privileges (for port forwarding)

## Building from Source

```bash
g++ -o portforwarder portforwarder.cpp -std=c++11 -pthread
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

Created by the1Domo

## Support

For support, please open an issue in the GitHub repository or contact the maintainers.

## Acknowledgments

- Thanks to all contributors who have helped shape this project
- Special thanks to the C++ community for their invaluable resources

## Security Considerations

- This application requires root privileges to function properly
- Always review forwarding rules before implementation
- Regularly monitor logs for any suspicious activity
- Keep the application updated with the latest security patches

## Disclaimer

This software is provided "as is", without warranty of any kind. Use at your own risk.

---

Made with ♥ by the1Domo