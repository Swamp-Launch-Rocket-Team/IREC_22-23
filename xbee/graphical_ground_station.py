# -*- coding: utf-8 -*-
import threading
import time
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import askokcancel, showerror, showwarning
import serial
import os
import struct
import re
import random

# User-defined constants
lat_bounds = [28.083, 28.084]
lon_bounds = [-80.600, -80.601]

ports = ["COM3", "COM4"]  # Tests ports in order
directory = "logs"

heading_font = ("Arial", 18, "bold")
log_font = ("Arial", 12, "bold")
text_font = ("Arial", 14, "bold")

states = ['ARMED', 'LAUNCH', 'EJECTION', 'CONTAIN_REL', 'DEPLOYED',
          'CHUTE_REL', 'CHUTE_AVOID', 'AUTONOMOUS', 'DESCENT', 'MANUAL', 'LANDED']

# Mock radio for testing without an Xbee connected
mock_radio = True

if not mock_radio:
    try:
        serialPort = serial.Serial(
            port=ports[0], baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
        )
    except serial.SerialException:
        serialPort = serial.Serial(
            port=ports[1], baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE
        )


def mock_read_radio():
    global warning_message_shown
    if not warning_message_shown:
        warning_message_shown = True
        # Show warning if radio is being mocked
        showwarning(title="Warning", message='The "mock_radio" debug flag is enabled. '
                                                'The radio will NOT be initialized. All data is synthetic. '
                                                'Set this flag to "False" to enable radio communication features.')
    time.sleep(0.5)
    with lock:
        if command_queue and random.random() < 0.25:
            mock_id = f"{command_queue[0]['id']:03d}"
        else:
            mock_id = "XXX"
    return (f"{mock_id}, 00001, ARMED, 28.083000, -80.601000, 18.8305, 0.0475154, -0.0779214\n")


# Global flag for go status
go_status_target = False

# Command ID globals
command_id = 1
command_queue = []
lock = threading.Lock()

# Open file with current date and time
directory = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), directory)
os.makedirs(directory, exist_ok=True)
f = open(os.path.join(
    directory, f"{time.strftime('%Y-%m-%dT%H%M%S', time.localtime())}.txt"), "w")


# Function to send a message over radio
def send_radio(msg):
    print(msg, end="")
    if mock_radio:
        return
    serialPort.write(msg.encode('utf-8'))


# Function for thread to handle command queue
def send_outgoing():
    while window_open:
        # Send command at front of queue
        with lock:
            if command_queue:
                try:
                    print_outgoing_log(
                        f"Sending command {command_queue[0]['id']:03d}: {command_queue[0]['text']}")
                    send_radio(f"{command_queue[0]['radio']}")
                except IndexError:
                    pass
        time.sleep(1)


warning_message_shown = False


# Function to read from the radio
def read_radio():
    if mock_radio:
        return mock_read_radio()

    serialString = ""  # Used to hold data coming over UART
    # Read data out of the buffer until a carraige return / new line is found
    serialString = serialPort.readline()
    # Print and log the contents of the serial data
    msg = ""
    try:
        msg = serialString.decode("Ascii")
    except:
        pass
    return msg


# Function for thread to continuously read incoming data
def read_incoming():
    while window_open:
        msg = read_radio()
        if msg:
            global go_status_received
            if msg[0] == "1":
                go_status_received = True
            elif msg[0] == "0":
                go_status_received = False
            # Check for command acknowledgements
            try:
                dequeue_command(int(msg[0:3]))
            except ValueError:
                pass
            current_time = time.strftime("%H:%M:%S", time.localtime())
            f.write(current_time + " " + msg)
            print_incoming(msg)
        time.sleep(0.5)


# Function to print incoming data with timestamps
def print_incoming(msg):
    autoscroll = False
    if incoming_log_scrollbar.get()[1] == 1:
        autoscroll = True
    current_time = time.strftime("%H:%M:%S", time.localtime())
    incoming_log.config(state=tk.NORMAL)
    incoming_log.insert(tk.END, f"{current_time}  ", "timestamp")
    incoming_log.insert(tk.END, msg, "text")
    incoming_log.config(state=tk.DISABLED)
    if autoscroll:
        incoming_log.see(tk.END)


# Function to print outgoing data with timestamps
def print_outgoing_log(msg):
    autoscroll = False
    if outgoing_log_scrollbar.get()[1] == 1:
        autoscroll = True
    current_time = time.strftime("%H:%M:%S", time.localtime())
    outgoing_log.config(state=tk.NORMAL)
    outgoing_log.insert(tk.END, f"{current_time}  ", "timestamp")

    # Color format the words "Sending" and "acknowledged"
    msg += "\n"
    msg_list = re.findall(r"\b\w+\b|\s+|[^\s\w]+", msg)
    for word in msg_list:
        style = "text"
        if word.lower() == "sending":
            style = "send"
        elif word.lower() == "acknowledged":
            style = "ack"
        outgoing_log.insert(tk.END, f"{word}", style)
    outgoing_log.config(state=tk.DISABLED)
    if autoscroll:
        outgoing_log.see(tk.END)


# Function to add a command to the command queue
def enqueue_command(command_opcode: str, command_operands: str, command_text: str):
    command_text = command_text.capitalize()
    global command_id
    with lock:
        command_queue.append({
            "id": command_id,
            # NOTE: Might need to end with "\r\n"
            "radio": f"{command_opcode}{command_id:03d}{command_operands}\n",
            "text": command_text
        })
    # Update command queue
    autoscroll = False
    if queue_text_scrollbar.get()[1] == 1:
        autoscroll = True
    queue_text.config(state=tk.NORMAL)
    queue_text.insert(tk.END, f"{command_id:03d} ", "id")
    queue_text.insert(tk.END, command_text + "\n", "text")
    queue_text.config(state=tk.DISABLED)
    if autoscroll:
        queue_text.see(tk.END)
    queue_cancel_button.config(state=tk.NORMAL)
    command_id += 1


# Function to remove a command from the command queue
def dequeue_command(command_id: int):
    # Find place in queue of the command being removed
    line_no = 1
    found = False
    with lock:
        for command in command_queue:
            if command["id"] == command_id:
                found = True
                break
            line_no += 1
        if not found:
            return
        # Remove command from queue
        print_outgoing_log(
            f"Command {command['id']:03d} acknowledged: {command['text']}")
        if command['radio'][0] == 'G':
            global go_status_target
            if go_status_target:
                go_button.config(fg="white", bg="green", state=tk.NORMAL)
            else:
                go_button.config(fg="white", bg="#B81D13", state=tk.NORMAL)
        command_queue.remove(command)
        if not command_queue:
            queue_cancel_button.config(state=tk.DISABLED)
    queue_text.config(state=tk.NORMAL)
    queue_text.delete(f"{line_no}.0", f"{line_no + 1}.0")
    queue_text.config(state=tk.DISABLED)


# Function to cancel a command from the command queue
def cancel_command():
    # Remove last command from queue
    with lock:
        command = command_queue.pop()
        if not command_queue:
            queue_cancel_button.config(state=tk.DISABLED)
    if command['radio'][0] == 'G':
        global go_status_target
        if go_status_target:
            go_button.config(fg="white", bg="#B81D13", state=tk.NORMAL)
        else:
            go_button.config(fg="white", bg="green", state=tk.NORMAL)
        go_status_target = not go_status_target
    queue_text.config(state=tk.NORMAL)
    lines = queue_text.get("1.0", tk.END).splitlines()
    if len(lines) > 1:
        queue_text.delete(f"{len(lines) - 1}.0", f"{len(lines)}.0")
    else:
        queue_text.delete("1.0", tk.END)
    queue_text.config(state=tk.DISABLED)


# Function to handle queue cancel button clicks
def handle_queue_cancel_button_click():
    if askokcancel(title="Confirmation", message=(f"Are you sure you want to cancel the most recent command in the"
                                                  " command queue?")):
        cancel_command()


def float_to_hex_string(fp32_number: float):
    byte_string = struct.pack('!f', fp32_number)
    hex_string = ""
    for i in byte_string:
        hex_string += f"{i:02x}"
    return hex_string


# Function to handle state button clicks
def handle_state_button_click(number):
    if askokcancel(title="Confirmation", message=f"Are you sure you want to change state to {states[number]}?"):
        enqueue_command("S", f"{number:1x}",
                        f"change to {states[number]} state.")


# Function to handle GPS target submit button
def handle_gps_target_button_click():
    if not lat_string.get() or not lon_string.get() or not alt_string.get():
        showerror(title="Error", message="Fill out all fields.")
        return
    lat = float(lat_string.get())
    lon = float(lon_string.get())
    alt = float(alt_string.get())
    if lat < min(lat_bounds) or lat > max(lat_bounds) or lon < min(lon_bounds) or lon > max(lon_bounds):
        showerror(title="Error", message="Target position out of bounds!")
        return
    if askokcancel(title="Confirmation", message=(f"Are you sure you want to send a new GPS target?\n"
                                                  f"Lat: {lat:.5f}\N{DEGREE SIGN}\n"
                                                  f"Lon: {lon:.5f}\N{DEGREE SIGN}\n"
                                                  f"Alt: {alt:.2f}\n"
                                                  f"Take Photo: {'Yes' if take_photo.get() == 1 else 'No'}")):
        operands = (float_to_hex_string(lat) + float_to_hex_string(lon) +
                    float_to_hex_string(alt) + str(take_photo.get()))
        text = (f"set GPS target to {lat:.5f}\N{DEGREE SIGN}, {lon:.5f}\N{DEGREE SIGN}, {alt:.2f} m"
                f", take photo: {'yes' if take_photo.get() == 1 else 'no'}")
        enqueue_command("T", operands, text)


# Function to handle go button
def handle_go_button():
    global go_status_target
    if askokcancel(title="Confirmation", message=(f"Are you sure you want to "
                                                  f"{'enable' if not go_status_target else 'disable'} "
                                                  "drone deployment?")):
        go_button.config(fg="black", bg="orange", state=tk.DISABLED)
        go_status_target = not go_status_target
        enqueue_command("G", str(int(go_status_target)),
                        f"{'enabling' if go_status_target else 'disabling'} drone deployment.")


# Function to handle manual control
def handle_manual_control(direction):
    valid_directions = ["N", "E", "S", "W", "U", "D"]
    direction_strings = ["north", "east", "south", "west", "up", "down"]
    if direction not in valid_directions:
        raise RuntimeError("Invalid parameter for `handle_manual_control`")
    if not manual_distance_string.get():
        showerror(title="Error", message="Enter a distance to travel.")
        return
    distance = float(manual_distance_string.get())
    text = f"move {distance:.2f} meters {direction_strings[valid_directions.index(direction)]}."
    if askokcancel(title="Confirmation", message=(f"Are you sure you want to {text[0:-1]}?")):
        operands = (direction + float_to_hex_string(distance))
        enqueue_command("M", operands, text)


# Function to handle window closing event
def on_closing():
    if askokcancel(title="Confirmation", message=(f"Are you sure you want to quit?")):
        global window_open
        window_open = False
        window.destroy()

        # clean up
        f.close()
        if "serialPort" in globals():
            serialPort.close()


# Create the GUI windows
window = tk.Tk()
window.title("Ground Station")
window.state('zoomed')

# Create the frame for the top half of the interface
top_frame = tk.Frame(window)
top_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

# Create the frame for displaying incoming data
incoming_log_frame = tk.Frame(
    top_frame, bg="white", highlightbackground="#646464", highlightthickness=1)
incoming_log_frame.pack(side=tk.LEFT, fill=tk.BOTH,
                        expand=True, padx=10, pady=10)

incoming_log_label = tk.Label(
    incoming_log_frame, text="Incoming Data", font=heading_font, bg="white")
incoming_log_label.pack()

incoming_log = tk.Text(incoming_log_frame, height=1, width=20,
                       state=tk.DISABLED, font=log_font, wrap=tk.WORD, bg="white")
incoming_log.pack(side=tk.LEFT, fill=tk.BOTH,
                  expand=True, padx=(10, 0), pady=10)
incoming_log.tag_config("timestamp", foreground="gray")
incoming_log.tag_config("text", foreground="black")

incoming_log_scrollbar = ttk.Scrollbar(
    incoming_log_frame, command=incoming_log.yview)
incoming_log_scrollbar.pack(side=tk.RIGHT, fill=tk.Y, padx=(0, 10), pady=10)
incoming_log.config(yscrollcommand=incoming_log_scrollbar.set)

# Create the frame for displaying outgoing data
outgoing_log_frame = tk.Frame(
    top_frame, bg="white", highlightbackground="#646464", highlightthickness=1)
outgoing_log_frame.pack(side=tk.RIGHT, fill=tk.BOTH,
                        expand=True, padx=10, pady=10)

outgoing_log_label = tk.Label(
    outgoing_log_frame, text="Outgoing Data", font=heading_font, bg="white")
outgoing_log_label.pack()

outgoing_log = tk.Text(outgoing_log_frame, height=1, width=25,
                       state=tk.DISABLED, font=log_font, wrap=tk.WORD, bg="white")
outgoing_log.pack(side=tk.LEFT, fill=tk.BOTH,
                  expand=True, padx=(10, 0), pady=10)
outgoing_log.tag_config("timestamp", foreground="gray")
outgoing_log.tag_config("text", foreground="black")
outgoing_log.tag_config("send", foreground="orange")
outgoing_log.tag_config("ack", foreground="green")

outgoing_log_scrollbar = ttk.Scrollbar(
    outgoing_log_frame, command=outgoing_log.yview)
outgoing_log_scrollbar.pack(side=tk.RIGHT, fill=tk.Y, padx=(0, 10), pady=10)
outgoing_log.config(yscrollcommand=outgoing_log_scrollbar.set)

# Create the frame for the bottom half of the interface
bottom_frame = tk.Frame(window)
bottom_frame.pack(fill=tk.X, expand=False)

# Create the frame for the command queue
queue_frame = tk.Frame(bottom_frame, bg="white",
                       highlightbackground="#646464", highlightthickness=1)
queue_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=10, pady=10)

queue_label = tk.Label(queue_frame, text="Command Queue",
                       font=heading_font, bg="white")
queue_label.pack()

queue_text_frame = tk.Frame(queue_frame, bg="white",
                            highlightbackground="#646464")
queue_text_frame.pack(fill=tk.BOTH, expand=True)

queue_text = tk.Text(queue_text_frame, height=1, width=25,
                     state=tk.DISABLED, font=log_font, wrap=tk.WORD, bg="white")
queue_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(10, 0), pady=10)
queue_text.tag_config("id", foreground="gray")
queue_text.tag_config("text", foreground="black")

queue_text_scrollbar = ttk.Scrollbar(
    queue_text_frame, command=outgoing_log.yview)
queue_text_scrollbar.pack(side=tk.RIGHT, fill=tk.Y, padx=(0, 10), pady=10)
queue_text.config(yscrollcommand=queue_text_scrollbar.set)

queue_cancel_button = tk.Button(queue_frame, text="Cancel Last Command", font=text_font,
                                height=1, padx=10, pady=10, command=handle_queue_cancel_button_click, state=tk.DISABLED)
queue_cancel_button.pack(fill=tk.X, padx=10, pady=(0, 10))

# Create the frame for buttons
button_frame = tk.Frame(bottom_frame, bg="white",
                        highlightbackground="#646464", highlightthickness=1)
button_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

manual_state_label = tk.Label(
    button_frame, text="Manual State Override", font=heading_font, bg="white")
manual_state_label.pack()

state_left_button_frame = tk.Frame(button_frame, bg="white")
state_left_button_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

state_right_button_frame = tk.Frame(button_frame, bg="white")
state_right_button_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=10, pady=10)

# Create state buttons
for i in range(0, len(states)):
    if i < len(states) / 2:
        state_button = tk.Button(state_left_button_frame, text=states[i], font=text_font, width=12,
                                 height=1, padx=10, pady=10, command=lambda num=i: handle_state_button_click(num))
    else:
        state_button = tk.Button(state_right_button_frame, text=states[i], font=text_font, width=12,
                                 height=1, padx=10, pady=10, command=lambda num=i: handle_state_button_click(num))
    state_button.pack(pady=5)

# Create the frame for GPS target
target_frame = tk.Frame(bottom_frame, bg="white",
                        highlightbackground="#646464", highlightthickness=1)
target_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

target_label = tk.Label(target_frame, text="GPS Target",
                        font=heading_font, bg="white")
target_label.pack(padx=10)


def validate_numeric_entry(P):
    if not P or P == "-":
        return True
    try:
        float(P)
        return True
    except ValueError:
        return False


lat_label = tk.Label(target_frame, text=f"Latitude:\n[{lat_bounds[0]:.5f}, {lat_bounds[1]:.5f}]",
                     font=text_font, bg="white", anchor=tk.W, justify=tk.LEFT)
lat_label.pack(fill=tk.X, padx=10, pady=(10, 0))

lat_string = tk.StringVar()
lat_string.set(f"{lat_bounds[0]:.3f}")
lat_box = tk.Entry(target_frame, textvariable=lat_string,
                   validate="key", font=text_font, bd=2)
lat_box.configure(validatecommand=(
    lat_box.register(validate_numeric_entry), '%P'))
lat_box.pack(fill=tk.X, padx=10)

lon_label = tk.Label(target_frame, text=f"Longitude:\n[{lon_bounds[0]:.5f}, {lon_bounds[1]:.5f}]",
                     font=text_font, bg="white", anchor=tk.W, justify=tk.LEFT)
lon_label.pack(fill=tk.X, padx=10, pady=(10, 0))

lon_string = tk.StringVar()
lon_string.set(f"{lon_bounds[0]:.3f}")
lon_box = tk.Entry(target_frame, textvariable=lon_string,
                   validate="key", font=text_font, bd=2)
lon_box.configure(validatecommand=(
    lon_box.register(validate_numeric_entry), '%P'))
lon_box.pack(fill=tk.X, padx=10)

alt_label = tk.Label(target_frame, text="Altitude (meters):",
                     font=text_font, bg="white", anchor=tk.W, justify=tk.LEFT)
alt_label.pack(fill=tk.X, padx=10, pady=(10, 0))

alt_string = tk.StringVar()
alt_box = tk.Entry(target_frame, textvariable=alt_string,
                   validate="key", font=text_font, bd=2)
alt_box.configure(validatecommand=(
    alt_box.register(validate_numeric_entry), '%P'))
alt_box.pack(fill=tk.X, padx=10)

take_photo = tk.IntVar()
take_photo_check = tk.Checkbutton(
    target_frame, text="Take photo at target", variable=take_photo, font=text_font, bg="white")
take_photo_check.pack(padx=10, pady=(10, 0))

gps_submit_button = tk.Button(target_frame, text="Submit", font=text_font,
                              height=1, padx=10, pady=10, command=handle_gps_target_button_click)
gps_submit_button.pack(fill=tk.X, padx=10, pady=10)

# Create the frame for manual control
manual_control_frame = tk.Frame(
    bottom_frame, bg="white", highlightbackground="#646464", highlightthickness=1)
manual_control_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

manual_control_label = tk.Label(
    manual_control_frame, text="Manual Control", font=heading_font, bg="white")
manual_control_label.pack(padx=10)

manual_distance_label = tk.Label(manual_control_frame, text="Distance (meters):",
                                 font=text_font, bg="white", anchor=tk.W, justify=tk.LEFT)
manual_distance_label.pack(fill=tk.X, padx=10, pady=(10, 0))

manual_distance_string = tk.StringVar()
manual_distance_string.set("3")
manual_distance_box = tk.Entry(manual_control_frame, textvariable=manual_distance_string,
                               validate="key", font=text_font, bd=2)
manual_distance_box.configure(validatecommand=(
    manual_distance_box.register(validate_numeric_entry), '%P'))
manual_distance_box.pack(fill=tk.X, padx=10, pady=(0, 10))

manual_control_button_frame = tk.Frame(manual_control_frame, bg="white")
manual_control_button_frame.pack()

manual_button_config = {
    'font': text_font, 'height': 1, 'width': 3, 'padx': 10, 'pady': 10}
manual_grid_config = {'padx': 5, 'pady': 5}

north_button = tk.Button(manual_control_button_frame, text="N", **manual_button_config,
                         command=lambda direction="N": handle_manual_control(direction))
north_button.grid(row=0, column=1, **manual_grid_config)

east_button = tk.Button(manual_control_button_frame, text="E", **manual_button_config,
                        command=lambda direction="E": handle_manual_control(direction))
east_button.grid(row=1, column=2, **manual_grid_config)

south_button = tk.Button(manual_control_button_frame, text="S", **manual_button_config,
                         command=lambda direction="S": handle_manual_control(direction))
south_button.grid(row=2, column=1, **manual_grid_config)

west_button = tk.Button(manual_control_button_frame, text="W", **manual_button_config,
                        command=lambda direction="W": handle_manual_control(direction))
west_button.grid(row=1, column=0, **manual_grid_config)

up_button = tk.Button(manual_control_button_frame, text="U", **manual_button_config,
                      command=lambda direction="U": handle_manual_control(direction))
up_button.grid(row=0, column=2, **manual_grid_config)

down_button = tk.Button(manual_control_button_frame, text="D", **manual_button_config,
                        command=lambda direction="D": handle_manual_control(direction))
down_button.grid(row=2, column=0, **manual_grid_config)

# Create the button for enabling the drone
go_button = tk.Button(window, text="Go for deployment", font=heading_font,
                      fg="white", bg="#B81D13", height=1, padx=10, pady=10, command=handle_go_button)
go_button.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=10)

# Set the flag to indicate whether the windows are open
window_open = True

# Create and start the thread for printing incoming data
incoming_thread = threading.Thread(target=read_incoming)
# Set incoming thread as a daemon thread to terminate it when the main thread ends
incoming_thread.daemon = True
incoming_thread.start()

# Create and start the thread for sending outgoing data from the command queue
outgoing_thread = threading.Thread(target=send_outgoing)
# Set incoming thread as a daemon thread to terminate it when the main thread ends
outgoing_thread.daemon = True
outgoing_thread.start()

# Bind the window closing events to the on_closing function
window.protocol("WM_DELETE_WINDOW", on_closing)

# Start the GUI event loops for both windows
window.mainloop()
