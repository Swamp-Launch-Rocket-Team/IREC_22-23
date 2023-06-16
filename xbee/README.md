# Ground Station and Drone Radio Communication Protocol

*All messages are terminated with a new line character (`\n`).*

## Drone-to-Ground
Drone transmits a message indicating its status once every second. Length of message varies.

`{Command ID Received}, {Drone Message ID}, {State}, {GPS Lat}, {GPS Lon}, {Velocity X}, {Velocity Y}, {Velocity Z}`
- `Command ID Received`: 3-digit decimal integer of the ID of the most recent command received and processed from the ground staiton. Starts at 000. 
- `Drone Message ID`: 5-digit decimal integer (using leading zeros when necessary) starting at 00001 and incrementing after every drone-to-ground message.
- `State`: String of the name of the current drone state.

## Ground-to-Drone

### General Format
`{Opcode}{Command ID}{Operand 1}...`
- `Opcode`: Single character corresponding to the operation to be performed.
- `Command ID`: 3-digit decimal integer (using leading zeros when necessary) starting at 001 and incrementing after every new command. If the same command is sent repeatedly, it uses the same command ID.
- `Operands`: One or more operands follow the opcode and command ID. The operands for each opcode are described in the following sections.

### Go for Deployment Command
Enables or disables deployment. 5 characters long.

`G{Command ID}{Go Status}`
- `Go Status`: 1-digit boolean representing go status.

### State Transition Command
Commands the drone to switch to a certain state. 5 characters long.

`S{Command ID}{State ID}`

- `State ID`: 1-digit hexadecimal corresponding to a certain state.
  - `0`: Armed
  - `1`: Launch
  - `2`: Ejection
  - `4`: Container Release
  - `3`: Deployed
  - `5`: Parachute Release
  - `6`: Parachute Avoidance
  - `7`: Autonomous
  - `8`: Descent
  - `9`: Manual
  - `A` or `a`: Landed

### GPS Target Command
Commands the drone to navigate to a target position. 29 characters long.

`T{Command ID}{Lat}{Lon}{Alt}{Picture}`
- `Lat`: 8-digit hexadecimal IEEE 754 single-precision floating-point representation the target latitude.
- `Lat`: 8-digit hexadecimal IEEE 754 single-precision floating-point representation the target longitude.
- `Lat`: 8-digit hexadecimal IEEE 754 single-precision floating-point representation the target altitude.
- `Picture`: 1-digit boolean representing whether to take a picture upon reaching the target.

### Manual Control Command
Commands the drone to travel a certain distance in a certain direction. 13 characters long.

`M{Command ID}{Direction}{Distance}`
- `Direction`: Single character corresponding to a direction
  - `N`: North
  - `E`: East
  - `S`: South
  - `W`: West
  - `U`: Up
  - `D`: Down
- `Distance`: 8-digit hexadecimal IEEE 754 single-precision floating-point representation of the distance to travel in meters.
