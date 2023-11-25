#!/usr/bin/env python3

# A little script I made to learn/test the ESP8266 AT commands for this project

import time
import os
from functools import partial
from typing import NoReturn, Callable

import serial  # type: ignore


class ESP8266:
    ok_resps = ["OK", "ready", "no change"]
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


def wifi_test(esp: ESP8266) -> bool:
    """
    Test connecting to WIFI
    """
    # Get some secrets from the environment
    ssid = os.getenv("ESP_WIFI_SSID")
    passwd = os.getenv("ESP_WIFI_PASSWD")
    assert ssid is not None, "Failed to get SSID from the environment!"
    assert passwd is not None, "Failed to get WIFI password from the environment!"

    # fmt: off
    wifi_commands = [
        "AT+CWMODE=3",                        # Set wifi mode to both
        "AT+CWMODE?",                         # Check current WIFI mode: client (1), access point (2) or both (3)
        "AT+CWDHCP_CUR=2,0",                  # Disable DHCP in all modes
        "AT+CWDHCP_CUR=1,1",                  # Enable DHCP in client mode
        "AT+CWDHCP_CUR?",                     # Check that DHCP is enabled in client mode
        "AT+CWMODE=1",                        # Set wifi mode to client
        "AT+CWMODE?",                         # Check current WIFI mode
        "AT+CWLAP",                           # List access points
        "AT+CWJAP_CUR?",                      # Check if we are already connected to an acces point
        f'AT+CWJAP_CUR="{ssid}","{passwd}"',  # Connect to WIFI
        "AT+CWQAP",                           # Disconnect
    ]
    # fmt: on

    return all(esp.send_cmd(c) for c in wifi_commands)


def main() -> NoReturn:
    serial_dev = "/dev/ttyUSB0"

    print(f"Testing ESP8266 connected to {serial_dev}...")
    print()

    esp = ESP8266(serial_dev)

    tests: list[Callable[[], bool]] = [
        partial(restart, esp),
        partial(echo_mode, esp, True),
        partial(echo_mode, esp, False),
        partial(smoke_test, esp),
        partial(wifi_test, esp),
    ]
    if not all(t() for t in tests):
        exit(1)

    print("All tests OK!")
    exit(0)


if __name__ == "__main__":
    main()
