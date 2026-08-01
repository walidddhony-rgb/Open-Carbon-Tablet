"""
OSICT - Python Receiver Application
Receives data from USB, draws on screen, and saves drawings.

Requirements:
- pyserial (only for real serial use)
- matplotlib
- numpy

Install: pip install -r requirements.txt
"""

import argparse
import threading
import time
import random
import collections
import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np
from datetime import datetime

# ==================== CONFIGURATION ====================

DEFAULT_SERIAL_PORT = 'COM3'  # Change to your port (e.g., /dev/ttyUSB0 on Linux)
BAUDRATE = 115200
MAX_POINTS = 5000

# ==================== GLOBAL VARIABLES ====================

x_data = []
y_data = []
ser = None
is_drawing = False
running = True

# ==================== SERIAL SIMULATOR (for testing without hardware) ====

class FakeSerial:
    """A minimal serial-like object that generates ASCII and binary packets."""
    def __init__(self, baudrate=115200, timeout=1, interval=0.01):
        self.baudrate = baudrate
        self.timeout = timeout
        self.interval = interval
        self._buffer = collections.deque()
        self._lock = threading.Lock()
        self._thread = threading.Thread(target=self._producer, daemon=True)
        self._stop = threading.Event()
        self._thread.start()

    @property
    def in_waiting(self):
        with self._lock:
            return len(self._buffer)

    def _producer(self):
        # Produce mixed ASCII and binary packets
        proto = 1
        while not self._stop.is_set():
            # choose to send binary (70%) or ASCII (30%)
            if random.random() < 0.7:
                # binary packet format: 0xAA PROTO X_H X_L Y_H Y_L SEG PEN CHK
                seg = random.randint(0, 3)
                x = random.randint(0, 1023)
                y = random.randint(0, 1023)
                pen = 1
                x_h = (x >> 8) & 0xFF
                x_l = x & 0xFF
                y_h = (y >> 8) & 0xFF
                y_l = y & 0xFF
                s = (proto + x_h + x_l + y_h + y_l + seg + pen) & 0xFF
                packet = bytes([0xAA, proto, x_h, x_l, y_h, y_l, seg, pen, s])
            else:
                # ASCII: SEG:X:Y\n
                seg = random.randint(0, 3)
                x = random.randint(0, 1023)
                y = random.randint(0, 1023)
                line = f"{seg}:{x}:{y}\n".encode('utf-8')
                packet = line

            with self._lock:
                for b in packet:
                    self._buffer.append(b)

            time.sleep(self.interval)

    def read(self, size=1):
        out = bytearray()
        start = time.time()
        while len(out) < size:
            with self._lock:
                if self._buffer:
                    out.append(self._buffer.popleft())
                else:
                    # no data available
                    pass
            if len(out) >= size:
                break
            # respect timeout
            if (time.time() - start) > self.timeout:
                break
            time.sleep(0.001)
        return bytes(out)

    def readline(self):
        out = bytearray()
        start = time.time()
        while True:
            with self._lock:
                if self._buffer:
                    b = self._buffer.popleft()
                    out.append(b)
                    if b == 0x0A:  # newline
                        break
                else:
                    pass
            if (time.time() - start) > self.timeout:
                break
            time.sleep(0.001)
        return bytes(out)

    def close(self):
        self._stop.set()
        if self._thread.is_alive():
            self._thread.join(timeout=0.5)

# ==================== SETUP SERIAL ====================

def setup_serial(port, baudrate, simulate=False):
    """Initialize serial connection or simulator"""
    global ser
    if simulate:
        ser = FakeSerial(baudrate=baudrate)
        print("Using Serial Simulator")
        return True

    try:
        ser = serial.Serial(port, baudrate, timeout=0.5)
        print("="*50)
        print("OSICT - Interactive Carbon Tablet")
        print("Connected to real serial device")
        print("="*50)
        print(f"\nConnected to {port}")
        return True
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return False

# ==================== DATA PARSING ====================

def parse_ascii_line(line):
    """Parse incoming ASCII data format: SEG:X:Y"""
    try:
        parts = line.strip().split(':')
        if len(parts) == 3:
            segment = int(parts[0])
            x = int(parts[1])
            y = int(parts[2])

            global_x = x + (segment % 2) * 1024
            global_y = y + (segment // 2) * 1024

            return global_x, global_y
    except Exception:
        pass
    return None, None


def read_binary_packet(ser):
    """Read a binary packet from ser if available. Returns (x,y) or (None,None)."""
    # We assume the caller already read the 0xAA start byte.
    payload = ser.read(8)  # PROTO + X_H + X_L + Y_H + Y_L + SEG + PEN + CHK
    if len(payload) != 8:
        return None, None
    proto = payload[0]
    x = (payload[1] << 8) | payload[2]
    y = (payload[3] << 8) | payload[4]
    seg = payload[5]
    pen = payload[6]
    chk = payload[7]
    s = (proto + payload[1] + payload[2] + payload[3] + payload[4] + payload[5] + payload[6]) & 0xFF
    if s != chk:
        # checksum failed
        return None, None
    global_x = x + (seg % 2) * 1024
    global_y = y + (seg // 2) * 1024
    return global_x, global_y

# ==================== DRAWING FUNCTIONS ====================

def update_plot(frame):
    """Update the matplotlib plot with new data"""
    global x_data, y_data, is_drawing

    if ser and ser.in_waiting > 0:
        # Try to read one byte to decide packet type
        b = ser.read(1)
        if not b:
            is_drawing = False
        else:
            first = b[0]
            if first == 0xAA:
                x, y = read_binary_packet(ser)
            else:
                # Read the rest of the line
                rest = ser.readline()
                try:
                    line = (bytes([first]) + rest).decode('utf-8', errors='ignore').strip()
                except Exception:
                    line = ''
                x, y = parse_ascii_line(line)

            if x is not None and y is not None:
                if not is_drawing:
                    if x_data:
                        x_data.append(np.nan)
                        y_data.append(np.nan)
                    is_drawing = True

                x_data.append(x)
                y_data.append(y)

                if len(x_data) > MAX_POINTS:
                    x_data.pop(0)
                    y_data.pop(0)
    else:
        is_drawing = False

    line_plot.set_data(x_data, y_data)

    if x_data:
        ax.set_xlim(min(x_data) - 50, max(x_data) + 50)
        ax.set_ylim(min(y_data) - 50, max(y_data) + 50)

    return line_plot,


def on_key(event):
    """Handle keyboard shortcuts"""
    if event.key == 'c' or event.key == 'C':
        x_data.clear()
        y_data.clear()
        print("Drawing cleared.")
    elif event.key == 's' or event.key == 'S':
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"drawing_{timestamp}.png"
        plt.savefig(filename, dpi=300)
        print(f"Drawing saved as {filename}")
    elif event.key == 'q' or event.key == 'Q':
        plt.close()

# ==================== MAIN ====================

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='OSICT Python Receiver')
    parser.add_argument('--port', '-p', default=DEFAULT_SERIAL_PORT, help='Serial port (e.g., COM3 or /dev/ttyUSB0)')
    parser.add_argument('--baud', '-b', type=int, default=BAUDRATE, help='Baud rate')
    parser.add_argument('--simulate', '-s', action='store_true', help='Use serial simulator instead of real device')
    args = parser.parse_args()

    if not setup_serial(args.port, args.baud, simulate=args.simulate):
        exit()

    fig, ax = plt.subplots(figsize=(12, 10))
    ax.set_title("OSICT - Real-Time Drawing", fontsize=16)
    ax.set_xlabel("X Coordinate", fontsize=12)
    ax.set_ylabel("Y Coordinate", fontsize=12)
    ax.grid(True, alpha=0.3)
    ax.set_aspect('equal')

    line_plot, = ax.plot([], [], 'b-', lw=2, markersize=1)

    ani = FuncAnimation(fig, update_plot, interval=10, blit=True)
    fig.canvas.mpl_connect('key_press_event', on_key)

    print("\nControls:")
    print("  'C' - Clear drawing")
    print("  'S' - Save drawing as PNG")
    print("  'Q' - Quit")
    print("\nStart drawing on the tablet!\n")

    plt.show()

    if ser:
        try:
            ser.close()
        except Exception:
            pass
