#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import serial
import time

def main():
    print("=== Bluetooth UART Receive Test ===")
    print("Press Ctrl+C to exit.\n")

    try:
        ser = serial.Serial(
            port="/dev/ttyAMA1",
            baudrate=9600,
            timeout=0.1
        )
    except Exception as e:
        print(f"uart open fail: {e}")
        return

    print("? UART Opened (/dev/ttyAMA1 @ 9600)")
    print("Waiting for data...\n")

    try:
        while True:
            data = ser.read(1)  # 1 byte read
            if data:
                ch = data.decode(errors="ignore")
                print(f"[RX] Raw: {data}  Char: '{ch}'")
            else:
                time.sleep(0.01)

    except KeyboardInterrupt:
        print("\nexit (Ctrl+C)")

    finally:
        ser.close()
        print("UART Closed")

if __name__ == "__main__":
    main()
