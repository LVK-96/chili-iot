#!/usr/bin/env python3

# Little UDP server to receive messages from the ESP8266 client
# Only receives UDP packets, does not send any responses
# Used in conjunction with the esp8266_test.py script

import socket
import os
from typing import Optional
import threading


def server(stop_event: Optional[threading.Event] = None) -> None:
    ip = os.getenv("UDP_SERVER_IP")
    port = os.getenv("UDP_SERVER_PORT")
    assert ip is not None, "Server: Failed to get IP from environment!"
    assert port is not None, "Server: Failed to get port from environment!"

    print(f"Server: Starting UDP server @ {ip}:{port}...")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((ip, int(port)))
    sock.setblocking(False)
    while True:
        try:
            data, _ = sock.recvfrom(1024)
        except socket.error as err:
            if err.args[0] != socket.EAGAIN and err.args[0] != socket.EWOULDBLOCK:
                raise err
        else:
            try:
                msg = data.decode('utf-8')
            except UnicodeDecodeError:
                msg = "0x" + data.hex()
            finally:
                print(f"Server: Received message: {msg}")
        # Check for the stop event -> when it is set we should close the server
        if (stop_event is not None) and stop_event.is_set():
            sock.close()
            return


def main() -> None:
    server()


if __name__ == "__main__":
    main()
