# TODO: Implement functions for writing to the Xbee

import threading
import time
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import askokcancel, showerror, showwarning
import serial
import os

# User-defined constants
heading_size = 18
log_size = 12
text_size = 14

states = ['ARMED', 'LAUNCH', 'EJECTION', 'CONTAIN_REL', 'DEPLOYED',
          'CHUTE_REL', 'CHUTE_AVOID', 'AUTONOMOUS', 'DESCENT', 'MANUAL', 'LANDED']

go_attempts = 5

lat_bounds = [28.083, 28.084]
lon_bounds = [-80.600, -80.601]

ports = ["COM3", "COM4"]  # Tests ports in order
directory = "logs"

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

# Global flag for go status
go_status_received = False
go_status_target = False

# Open file with current date and time
os.makedirs(directory, exist_ok=True)
f = open(os.path.join(
    directory, f"{time.strftime('%Y-%m-%dT%H%M%S', time.localtime())}.txt"), "w")

warning_message_shown = False


# Function to read from the radio
def read_radio():
    if mock_radio:
        global warning_message_shown
        if not warning_message_shown:
            warning_message_shown = True
            # Show warning if radio is being mocked
            showwarning(title="Warning", message='The "mock_radio" debug flag is enabled. '
                                                 'The radio will NOT be initialized. All data is synthetic. '
                                                 'Set this flag to "False" to enable radio communication features.')
        return (f"1, 72940, -3.16311, 175.503, 18.8305, 0.0475154, -0.0779214, -0.0619372, -0.000734926, -0.00192485, "
                "0.00107818, 0.55271, -0.772782, -9.68336, 27.9323, -80.7091, -6.70498, 8, 0, -7.34079e-06, 0, 0, 0, 0\n")

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


# Function to read incoming data every second
def read_incoming():
    while window_open:
        msg = read_radio()
        if msg:
            global go_status_received
            if msg[0] == "1":
                go_status_received = True
            elif msg[0] == "0":
                go_status_received = False
            current_time = time.strftime("%H:%M:%S", time.localtime())
            f.write(current_time + " " + msg)
            print_incoming(msg)
        time.sleep(0.5)


# Function to print incoming data with timestamps
def print_incoming(msg):
    autoscroll = False
    if incoming_scrollbar.get()[1] == 1:
        autoscroll = True
    current_time = time.strftime("%H:%M:%S", time.localtime())
    incoming_log.config(state=tk.NORMAL)
    incoming_log.insert(tk.END, f"{current_time}  ", "timestamp")
    incoming_log.insert(tk.END, msg, "text")
    incoming_log.config(state=tk.DISABLED)
    if autoscroll:
        incoming_log.see(tk.END)


def print_outgoing_log(msg):
    autoscroll = False
    if outgoing_scrollbar.get()[1] == 1:
        autoscroll = True
    current_time = time.strftime("%H:%M:%S", time.localtime())
    outgoing_log.config(state=tk.NORMAL)
    outgoing_log.insert(tk.END, f"{current_time}  ", "timestamp")
    outgoing_log.insert(tk.END, f"{msg}\n", "text")
    outgoing_log.config(state=tk.DISABLED)
    if autoscroll:
        outgoing_log.see(tk.END)


# Function to handle state button clicks
def handle_state_button_click(number):
    if askokcancel(title="Confirmation", message=f"Are you sure you want to change state to {states[number]}?"):
        print_outgoing_log(
            f"Sending command to change to {states[number]} state.")


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
                                                  f"Lat: {lat:.6f}\n"
                                                  f"Lon: {lon:.6f}\n"
                                                  f"Alt: {alt:.2f}\n"
                                                  f"Landing Site: {'Yes' if landing_site.get() == 1 else 'No'}")):
        print_outgoing_log(f"Setting GPS target\n"
                           f"\tLat: {lat:.6f}\n"
                           f"\tLon: {lon:.6f}\n"
                           f"\tAlt: {alt:.2f}\n"
                           f"\tLanding Site: {'Yes' if landing_site.get() == 1 else 'No'}")


# Function to handle go button
def handle_go_button():
    global go_status_target
    if askokcancel(title="Confirmation", message=(f"Are you sure you want to "
                                                  f"{'enable' if not go_status_target else 'disable'} "
                                                  "drone deployment?")):
        go_button.config(fg="black", bg="#EFB700", state=tk.DISABLED)
        go_status_target = not go_status_target
        print_outgoing_log(
            f"{'Enabling' if go_status_target else 'Disabling'} drone deployment.")
        go_status_thread = threading.Thread(target=send_go_status)
        # Set incoming thread as a daemon thread to terminate it when the main thread ends
        go_status_thread.daemon = True
        go_status_thread.start()


# Function for go command thread
# Repeatedly sends until acknowledgement or timeout
def send_go_status():
    global go_status_target
    count = 0
    while True:
        if count >= 1 and go_status_received == go_status_target:
            print_outgoing_log(
                f"{'Go' if go_status_target else 'No-go'} command acknowledged!")
            break
        print_outgoing_log(
            f"Sending {'go' if go_status_target else 'no-go'} command. Awaiting acknowledgement.")
        if count >= go_attempts:
            print_outgoing_log(
                f"{'Go' if go_status_target else 'No-go'} command timeout. "
                f"No acknowledgement received after {count} attempts.")
            break
        count += 1
        time.sleep(1)
    go_status_target = go_status_received
    if not go_status_target:
        go_button.config(fg="white", bg="#B81D13", state=tk.NORMAL)
    else:
        go_button.config(fg="white", bg="#008450", state=tk.NORMAL)


# Function to handle manual control
def handle_manual_control(direction):
    if direction not in ["north", "east", "south", "west", "up", "down"]:
        raise RuntimeError("Invalid parameter for `handle_manual_control`")
    if not manual_distance_string.get():
        showerror(title="Error", message="Enter a distance to travel.")
        return
    distance = float(manual_distance_string.get())
    print_outgoing_log(
        f"Sending command to move {distance} meters {direction}.")


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

# Create the frame for displaying incoming data
incoming_frame = tk.Frame(
    window, bg="white", highlightbackground="#646464", highlightthickness=1)
incoming_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=10)

incoming_label = tk.Label(incoming_frame, text="Incoming Data", font=(
    "Arial", heading_size, "bold"), bg="white")
incoming_label.pack()

incoming_log = tk.Text(incoming_frame, height=10, width=20, state=tk.DISABLED, font=(
    "Arial", log_size), wrap=tk.WORD, bg="white")
incoming_log.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=10, pady=10)

incoming_scrollbar = ttk.Scrollbar(incoming_frame, command=incoming_log.yview)
incoming_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
incoming_log.config(yscrollcommand=incoming_scrollbar.set)
incoming_log.tag_config("timestamp", foreground="gray")
incoming_log.tag_config("text", foreground="black")

# Create the frame for displaying outgoing data
outgoing_frame = tk.Frame(window)
outgoing_frame.pack(fill=tk.BOTH, expand=True)

outgoing_log_frame = tk.Frame(
    outgoing_frame, bg="white", highlightbackground="#646464", highlightthickness=1)
outgoing_log_frame.pack(side=tk.RIGHT, fill=tk.BOTH,
                        expand=True, padx=10, pady=10)

outgoing_log_label = tk.Label(outgoing_log_frame, text="Outgoing Data", font=(
    "Arial", heading_size, "bold"), bg="white")
outgoing_log_label.pack()

outgoing_log = tk.Text(outgoing_log_frame, height=5, width=25, state=tk.DISABLED, font=(
    "Arial", log_size), wrap=tk.WORD, bg="white")
outgoing_log.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, pady=10)

outgoing_scrollbar = ttk.Scrollbar(
    outgoing_log_frame, command=outgoing_log.yview)
outgoing_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
outgoing_log.config(yscrollcommand=outgoing_scrollbar.set)
outgoing_log.tag_config("timestamp", foreground="gray")
outgoing_log.tag_config("text", foreground="black")

# Create the frame for buttons
button_frame = tk.Frame(outgoing_frame, bg="white",
                        highlightbackground="#646464", highlightthickness=1)
button_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

buttons_label = tk.Label(button_frame, text="Manual State Override", font=(
    "Arial", heading_size, "bold"), bg="white")
buttons_label.pack()

left_button_frame = tk.Frame(button_frame, bg="white")
left_button_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

right_button_frame = tk.Frame(button_frame, bg="white")
right_button_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=10, pady=10)

# Create state buttons
for i in range(0, len(states)):
    if i < len(states) / 2:
        state_button = tk.Button(left_button_frame, text=states[i], font=(
            "Arial", text_size), width=12, height=1, padx=10, pady=10, command=lambda num=i: handle_state_button_click(num))
    else:
        state_button = tk.Button(right_button_frame, text=states[i], font=(
            "Arial", text_size), width=12, height=1, padx=10, pady=10, command=lambda num=i: handle_state_button_click(num))
    state_button.pack(pady=5)

# Create the frame for GPS target
target_frame = tk.Frame(outgoing_frame, bg="white",
                        highlightbackground="#646464", highlightthickness=1)
target_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

target_label = tk.Label(target_frame, text="GPS Target", font=(
    "Arial", heading_size, "bold"), bg="white")
target_label.pack(padx=10)


def validate_numeric_entry(P):
    if not P or P == "-":
        return True
    try:
        float(P)
        return True
    except ValueError:
        return False


lat_label = tk.Label(target_frame, text=f"Latitude:\n[{lat_bounds[0]:.6f}, {lat_bounds[1]:.6f}]", font=(
    "Arial", text_size), bg="white", anchor=tk.W, justify=tk.LEFT)
lat_label.pack(fill=tk.X, padx=10, pady=(10, 0))

lat_string = tk.StringVar()
lat_box = tk.Entry(target_frame, textvariable=lat_string,
                   validate="key", font=("Arial", text_size), bd=2)
lat_box.configure(validatecommand=(
    lat_box.register(validate_numeric_entry), '%P'))
lat_box.pack(fill=tk.X, padx=10)

lon_label = tk.Label(target_frame, text=f"Longitude:\n[{lon_bounds[0]:.6f}, {lon_bounds[1]:.6f}]", font=(
    "Arial", text_size), bg="white", anchor=tk.W, justify=tk.LEFT)
lon_label.pack(fill=tk.X, padx=10, pady=(10, 0))

lon_string = tk.StringVar()
lon_box = tk.Entry(target_frame, textvariable=lon_string,
                   validate="key", font=("Arial", text_size), bd=2)
lon_box.configure(validatecommand=(
    lon_box.register(validate_numeric_entry), '%P'))
lon_box.pack(fill=tk.X, padx=10)

alt_label = tk.Label(target_frame, text="Altitude (meters):", font=(
    "Arial", text_size), bg="white", anchor=tk.W, justify=tk.LEFT)
alt_label.pack(fill=tk.X, padx=10, pady=(10, 0))

alt_string = tk.StringVar()
alt_box = tk.Entry(target_frame, textvariable=alt_string,
                   validate="key", font=("Arial", text_size), bd=2)
alt_box.configure(validatecommand=(
    alt_box.register(validate_numeric_entry), '%P'))
alt_box.pack(fill=tk.X, padx=10)

landing_site = tk.IntVar()
landing_site_check = tk.Checkbutton(target_frame, text="Designate as landing site",
                                    variable=landing_site, font=("Arial", text_size), bg="white")
landing_site_check.pack(padx=10, pady=10)

gps_submit_button = tk.Button(target_frame, text="Submit", font=(
    "Arial", text_size), width=12, height=1, padx=10, pady=10, command=handle_gps_target_button_click)
gps_submit_button.pack(padx=10, pady=10)

# Create the frame for manual control
manual_control_frame = tk.Frame(
    outgoing_frame, bg="white", highlightbackground="#646464", highlightthickness=1)
manual_control_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

manual_control_label = tk.Label(manual_control_frame, text="Manual Control", font=(
    "Arial", heading_size, "bold"), bg="white")
manual_control_label.pack(padx=10)

manual_distance_label = tk.Label(manual_control_frame, text="Distance (meters):", font=(
    "Arial", text_size), bg="white", anchor=tk.W, justify=tk.LEFT)
manual_distance_label.pack(fill=tk.X, padx=10, pady=(10, 0))

manual_distance_string = tk.StringVar()
manual_distance_string.set("3")
manual_distance_box = tk.Entry(manual_control_frame, textvariable=manual_distance_string,
                               validate="key", font=("Arial", text_size), bd=2)
manual_distance_box.configure(validatecommand=(
    manual_distance_box.register(validate_numeric_entry), '%P'))
manual_distance_box.pack(fill=tk.X, padx=10, pady=(0, 10))

manual_control_button_frame = tk.Frame(manual_control_frame, bg="white")
manual_control_button_frame.pack()

manual_button_config = {
    'font': ("Arial", text_size, "bold"), 'height': 1, 'width': 3, 'padx': 10, 'pady': 10}
manual_grid_config = {'padx': 5, 'pady': 5}

north_button = tk.Button(manual_control_button_frame, text="N", **manual_button_config,
                         command=lambda direction="north": handle_manual_control(direction))
north_button.grid(row=0, column=1, **manual_grid_config)

east_button = tk.Button(manual_control_button_frame, text="E", **manual_button_config,
                        command=lambda direction="east": handle_manual_control(direction))
east_button.grid(row=1, column=2, **manual_grid_config)

south_button = tk.Button(manual_control_button_frame, text="S", **manual_button_config,
                         command=lambda direction="south": handle_manual_control(direction))
south_button.grid(row=2, column=1, **manual_grid_config)

west_button = tk.Button(manual_control_button_frame, text="W", **manual_button_config,
                        command=lambda direction="west": handle_manual_control(direction))
west_button.grid(row=1, column=0, **manual_grid_config)

up_button = tk.Button(manual_control_button_frame, text="U", **manual_button_config,
                      command=lambda direction="up": handle_manual_control(direction))
up_button.grid(row=0, column=2, **manual_grid_config)

down_button = tk.Button(manual_control_button_frame, text="D", **manual_button_config,
                        command=lambda direction="down": handle_manual_control(direction))
down_button.grid(row=2, column=0, **manual_grid_config)

# Create the button for enabling the drone
go_button = tk.Button(window, text="Go for deployment", font=("Arial", heading_size, "bold"),
                      fg="white", bg="#B81D13", height=1, padx=10, pady=10, command=handle_go_button)
go_button.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=10)

# Set the flag to indicate whether the windows are open
window_open = True

# Create and start the thread for printing incoming data
incoming_thread = threading.Thread(target=read_incoming)
# Set incoming thread as a daemon thread to terminate it when the main thread ends
incoming_thread.daemon = True
incoming_thread.start()

# Bind the window closing events to the on_closing function
window.protocol("WM_DELETE_WINDOW", on_closing)

# Start the GUI event loops for both windows
window.mainloop()
