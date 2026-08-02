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
STATUS_INTERVAL = 5.0  # seconds

# ==================== GLOBAL VARIABLES / STATS ====================

x_data = []
y_data = []
ser = None
is_drawing = False
running = True

# Packet stats
recv_count = 0
checksum_errors = 0
lost_count = 0
last_seq = None
start_time = time.time()
last_latency_ms = None
log_packets = False
accept_ascii = True
accept_binary = True

# ==================== SERIAL SIMULATOR (for testing without hardware) ====

class FakeSerial:
    """A minimal serial-like object that generates ASCII and binary packets (proto v2)."""
    def __init__(self, baudrate=115200, timeout=1, interval=0.01, proto=2):
        self.baudrate = baudrate
        self.timeout = timeout
        self.interval = interval
        self.proto = proto
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
        seq = 0
        while not self._stop.is_set():
            # choose to send binary (70%) or ASCII (30%)
            if random.random() < 0.7:
                # Extended proto v2 packet: 0xAA PROTO SEQ_H SEQ_L TS3 TS2 TS1 TS0 X_H X_L Y_H Y_L SEG PEN CHK
                seg = random.randint(0, 3)
                x = random.randint(0, 1023)
                y = random.randint(0, 1023)
                pen = 1
                proto = self.proto
                seq = (seq + 1) & 0xFFFF
                ts = int((time.time() - start_time) * 1000) & 0xFFFFFFFF
                seq_h = (seq >> 8) & 0xFF
                seq_l = seq & 0xFF
                ts_b3 = (ts >> 24) & 0xFF
                ts_b2 = (ts >> 16) & 0xFF
                ts_b1 = (ts >> 8) & 0xFF
                ts_b0 = ts & 0xFF
                x_h = (x >> 8) & 0xFF
                x_l = x & 0xFF
                y_h = (y >> 8) & 0xFF
                y_l = y & 0xFF
                payload = [proto, seq_h, seq_l, ts_b3, ts_b2, ts_b1, ts_b0, x_h, x_l, y_h, y_l, seg, pen]
                chk = sum(payload) & 0xFF
                packet = bytes([0xAA] + payload + [chk])
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
            if len(out) >= size:
                break
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
            if (time.time() - start) > self.timeout:
                break
            time.sleep(0.001)
        return bytes(out)

    def close(self):
        self._stop.set()
        if self._thread.is_alive():
            self._thread.join(timeout=0.5)

# ==================== SETUP SERIAL ====================

def setup_serial(port, baudrate, simulate=False, sim_proto=2):
    """Initialize serial connection or simulator"""
    global ser
    if simulate:
        ser = FakeSerial(baudrate=baudrate, proto=sim_proto)
        print("Using Serial Simulator (proto", sim_proto, ")")
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

            return {'proto': 0, 'x': global_x, 'y': global_y, 'seg': segment, 'pen': 1}
    except Exception:
        pass
    return None


def read_binary_frame(ser):
    """Read binary frame starting after 0xAA. Returns dict or None on error."""
    # Read proto
    b = ser.read(1)
    if not b or len(b) != 1:
        return None
    proto = b[0]

    if proto == 1:
        # Remaining 7 bytes
        rem = ser.read(7)
        if len(rem) != 7:
            return None
        payload = bytes([proto]) + rem
        x = (payload[1] << 8) | payload[2]
        y = (payload[3] << 8) | payload[4]
        seg = payload[5]
        pen = payload[6]
        chk = payload[7]
        s = sum(payload[0:7]) & 0xFF
        if s != chk:
            return {'error': 'checksum'}
        global_x = x + (seg % 2) * 1024
        global_y = y + (seg // 2) * 1024
        return {'proto': 1, 'x': global_x, 'y': global_y, 'seg': seg, 'pen': pen}

    elif proto == 2:
        # Remaining 13 bytes: SEQ(2) TS(4) X(2) Y(2) SEG(1) PEN(1) CHK(1)
        rem = ser.read(13)
        if len(rem) != 13:
            return None
        payload = bytes([proto]) + rem
        seq = (payload[1] << 8) | payload[2]
        ts = (payload[3] << 24) | (payload[4] << 16) | (payload[5] << 8) | payload[6]
        x = (payload[7] << 8) | payload[8]
        y = (payload[9] << 8) | payload[10]
        seg = payload[11]
        pen = payload[12]
        chk = payload[13]
        s = sum(payload[0:13]) & 0xFF
        if s != chk:
            return {'error': 'checksum'}
        global_x = x + (seg % 2) * 1024
        global_y = y + (seg // 2) * 1024
        return {'proto': 2, 'seq': seq, 'ts': ts, 'x': global_x, 'y': global_y, 'seg': seg, 'pen': pen}

    else:
        # Unknown proto: try to resync (read until newline?)
        return {'error': 'unknown_proto', 'proto': proto}

# ==================== STATS REPORTER ====================

def stats_reporter():
    global running, recv_count, checksum_errors, lost_count, last_latency_ms
    while running:
        elapsed = time.time() - start_time
        rate = recv_count / elapsed if elapsed > 0 else 0
        print("[STATS] recv=", recv_count, "lost=", lost_count, "chk_err=", checksum_errors,
              f"rate={rate:.2f}/s", f"last_lat_ms={last_latency_ms}")
        time.sleep(STATUS_INTERVAL)

# ==================== DRAWING FUNCTIONS ====================

def update_plot(frame):
    """Update the matplotlib plot with new data"""
    global x_data, y_data, is_drawing, recv_count, checksum_errors, last_seq, lost_count, last_latency_ms

    if ser and ser.in_waiting > 0:
        # Try to read one byte to decide packet type
        b = ser.read(1)
        if not b:
            is_drawing = False
        else:
            first = b[0]
            if first == 0xAA and accept_binary:
                pkt = read_binary_frame(ser)
                if pkt is None:
                    # incomplete
                    pass
                elif 'error' in pkt and pkt['error'] == 'checksum':
                    checksum_errors += 1
                elif 'error' in pkt:
                    # ignore unknown proto
                    pass
                else:
                    # valid pkt
                    recv_count += 1
                    if pkt['proto'] == 2:
                        seq = pkt['seq']
                        # detect lost packets
                        if last_seq is not None:
                            expected = (last_seq + 1) & 0xFFFF
                            if seq != expected:
                                # handle wrap-around
                                diff = (seq - expected) & 0xFFFF
                                lost_count += diff
                        last_seq = seq
                        # compute latency using timestamp
                        # ts is milliseconds since boot (start_time base)
                        now_ms = int((time.time() - start_time) * 1000) & 0xFFFFFFFF
                        last_latency_ms = (now_ms - pkt['ts']) if pkt.get('ts') is not None else None
                    x = pkt['x']
                    y = pkt['y']

                    if log_packets:
                        print(f"PACKET proto={pkt['proto']} x={x} y={y} seq={pkt.get('seq')} ts={pkt.get('ts')}")

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
                # treat as ASCII if allowed
                if accept_ascii:
                    rest = ser.readline()
                    try:
                        line = (bytes([first]) + rest).decode('utf-8', errors='ignore').strip()
                    except Exception:
                        line = ''
                    pkt = parse_ascii_line(line)
                    if pkt:
                        recv_count += 1
                        if log_packets:
                            print(f"PACKET ascii x={pkt['x']} y={pkt['y']}")
                        x = pkt['x']
                        y = pkt['y']

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
                    # ascii disabled: consume line
                    ser.readline()
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
    parser.add_argument('--sim-proto', type=int, default=2, help='Simulator proto version (1 or 2)')
    parser.add_argument('--no-ascii', action='store_true', help='Disable ASCII parsing')
    parser.add_argument('--no-binary', action='store_true', help='Disable binary parsing')
    parser.add_argument('--log', action='store_true', help='Log packets to console')
    args = parser.parse_args()

    log_packets = args.log
    accept_ascii = not args.no_ascii
    accept_binary = not args.no_binary

    if not setup_serial(args.port, args.baud, simulate=args.simulate, sim_proto=args.sim_proto):
        exit()

    # start stats thread
    stats_thread = threading.Thread(target=stats_reporter, daemon=True)
    stats_thread.start()

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

    try:
        plt.show()
    finally:
        running = False
        time.sleep(0.1)
        try:
            if ser:
                ser.close()
        except Exception:
            pass
