#!/usr/bin/env python3

# A little to learn/test the ESP8266 AT commands for this project

import time
import os
import itertools
from functools import partial
from typing import NoReturn, Callable
import concurrent.futures
import multiprocessing

import serial  # type: ignore

import udp_server


class ESP8266:
    ok_resps = ["OK", "ready", "no change", "SEND OK"]

    err_resps = ["ERROR"]

    def __init__(
        self, serial_dev: str, baudrate: int = 115200, response_timeout: int = 10
    ):
        self.serial = serial.Serial(
            port=serial_dev,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.5,
            exclusive=True,
        )
        self.response_timeout = response_timeout

    def _check_response_timeout(self, start: float) -> bool:
        return (time.time() - start) > self.response_timeout

    def send_cmd(self, cmd: str) -> bool:
        print(f"Sending command: {cmd}...")
        self.serial.flushInput()
        self.serial.write(f"{cmd}\r\n".encode("utf-8"))

        start_ts = time.time()
        timed_out = False
        error = False
        while not timed_out:
            resp = self.serial.readline().decode("utf-8").rstrip()
            if resp:
                print(resp)
            if resp in self.ok_resps:
                break
            if resp in self.err_resps:
                error = True
                break
            timed_out = self._check_response_timeout(start_ts)

        print()

        return (not timed_out) and (not error)


def restart(esp: ESP8266) -> bool:
    res = esp.send_cmd("AT+RST")
    time.sleep(1)  # Wait for the board to reboot
    return res


def echo_mode(esp: ESP8266, enable: bool = False) -> bool:
    """
    Enable/disable echoing of AT commands
    """
    res = False
    if enable:
        res = esp.send_cmd("ATE1")
    else:
        res = esp.send_cmd("ATE0")

    if not res:
        print("Echo mode setting failed!")
    return res


def smoke_test(esp: ESP8266) -> bool:
    """
    Test that the module responds to some basic commands
    """

    # fmt: off
    basic_commands = [
        "AT",            # AT test
        "AT+GMR",        # Version information
        "AT+UART_CUR?",  # Current USART settings
        "AT+UART_DEF?",  # Default USART settings, seems to return 0s if you have never set this
        "AT+SYSRAM?",    # Remaining RAM size in bytes
    ]
    # fmt: on

    return all(esp.send_cmd(c) for c in basic_commands)


def udp_test(esp: ESP8266) -> bool:
    """
    Test connecting to WIFI
    """
    # Get some variables from the environment
    ssid = os.getenv("ESP_WIFI_SSID")
    passwd = os.getenv("ESP_WIFI_PASSWD")
    server_ip = os.getenv("UDP_SERVER_IP")
    server_port = os.getenv("UDP_SERVER_PORT")
    for x in [ssid, passwd, server_ip, server_port]:
        assert x is not None, f"Failed to get variable from the environment!"

    # Make an list of alternating AT send command and message
    test_msg = "Hello World"
    test_again = test_msg + " again"
    test_and_again = test_again + " and again"
    test_and_again_ang_again = test_and_again + " and again"
    test_msgs = [test_msg, test_again, test_and_again, test_and_again_ang_again]
    alternating_cmd_msg = [
        cmd
        for cmd in itertools.chain.from_iterable(
            zip(
                [f"AT+CIPSEND={len(msg)}" for msg in test_msgs],
                test_msgs,
            )
        )
    ]

    # fmt: off
    wifi_commands = [
        "AT+CWMODE=3",                                         # Set wifi mode to both
        "AT+CWMODE?",                                          # Check current WIFI mode: client (1), access point (2) or both (3)
        "AT+CWDHCP_CUR=2,0",                                   # Disable DHCP in all modes
        "AT+CWDHCP_CUR=1,1",                                   # Enable DHCP in client mode
        "AT+CWDHCP_CUR?",                                      # Check that DHCP is enabled in client mode
        "AT+CWMODE=1",                                         # Set wifi mode to client
        "AT+CWMODE?",                                          # Check current WIFI mode
        "AT+CWLAP",                                            # List access points
        "AT+CWJAP_CUR?",                                       # Check if we are already connected to an acces point
        f'AT+CWJAP_CUR="{ssid}","{passwd}"',                   # Connect to WIFI
        "AT+CIPMUX=1",                                         # Multi TCP/UDP/SSL connection mode
        "AT+CIPCLOSE=5",                                       # Close all connections
        "AT+CIPMUX=0",                                         # Single connection mode
        f"AT+CIPSTART=\"UDP\",\"{server_ip}\",{server_port}",  # Establish UDP connection
    ] + alternating_cmd_msg                                    # Send the test messages
    wifi_commands += [
        "AT+CIPCLOSE",                                         # Close the UDP connection
        "AT+CWQAP",                                            # Disconnect from access point
    ]
    # fmt: on

    return all(esp.send_cmd(c) for c in wifi_commands)


def tests() -> bool:
    serial_dev = "/dev/ttyUSB0"
    esp = ESP8266(serial_dev)
    print(f"Testing ESP8266 connected to {serial_dev}...")
    print()
    tests: list[Callable[[], bool]] = [
        partial(restart, esp),
        partial(echo_mode, esp, True),
        partial(echo_mode, esp, False),
        partial(smoke_test, esp),
        partial(udp_test, esp),
    ]
    return all(t() for t in tests)


def main() -> NoReturn:
    # Run the UDP server and tests in parallel parallel separate processes
    res: bool
    with multiprocessing.Manager() as manager:
        stop_server_event = manager.Event()
        with concurrent.futures.ProcessPoolExecutor() as executor:
            server_future = executor.submit(udp_server.server, stop_server_event)
            # Wait for the server to start
            while not server_future.running():
                pass

            # Run the tests
            test_future = executor.submit(tests)
            res = test_future.result()

            # The tests have completed -> close the server
            stop_server_event.set()
            server_future.result()

    exit(not res)


if __name__ == "__main__":
    main()
