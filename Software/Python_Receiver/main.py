"""
OSICT - Python Receiver Application
Receives data from USB, draws on screen, and saves drawings.

Requirements:
- pyserial
- matplotlib
- numpy

Install: pip install pyserial matplotlib numpy
"""

import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np
from datetime import datetime

# ==================== CONFIGURATION ====================

SERIAL_PORT = 'COM3'  # Change to your port (e.g., /dev/ttyUSB0 on Linux)
BAUDRATE = 115200
MAX_POINTS = 5000

# ==================== GLOBAL VARIABLES ====================

x_data = []
y_data = []
ser = None
is_drawing = False

# ==================== SETUP ====================

def setup_serial():
    """Initialize serial connection"""
    global ser
    try:
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
        print("="*50)
        print("OSICT - Interactive Carbon Tablet")
        print("Developed in collaboration with AI tools (Gemini & DeepSeek).")
        print("For details, see: https://github.com/yourusername/Open-Carbon-Tablet")
        print("="*50)
        print(f"\nConnected to {SERIAL_PORT}")
        return True
    except Exception as e:
        print(f"Error: {e}")
        print("Make sure the device is connected and the port is correct.")
        return False

# ==================== DATA PARSING ====================

def parse_data(line):
    """Parse incoming data format: SEG:X:Y"""
    try:
        parts = line.strip().split(':')
        if len(parts) == 3:
            segment = int(parts[0])
            x = int(parts[1])
            y = int(parts[2])
            
            # Convert to global coordinates
            global_x = x + (segment % 2) * 1024
            global_y = y + (segment // 2) * 1024
            
            return global_x, global_y
    except:
        pass
    return None, None

# ==================== DRAWING FUNCTIONS ====================

def update_plot(frame):
    """Update the matplotlib plot with new data"""
    global x_data, y_data, is_drawing
    
    if ser and ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        if line:
            x, y = parse_data(line)
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
    if not setup_serial():
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
        ser.close()