# IR_remote_control_repeater

Using an Atmel ATMega328 with an IR receiver diode connected to decode the IR signal 
from a TV remote control and send the received code again via an IR transmitting diode. 

Cable TV set-top box in the living room has AV-outputs which are hooked up to 4 
(video/left/right/GND) of the 8 wires in an existing Cat5 LAN cable to the AV-inputs 
of the TV in the bedroom, allowing the two TV sets to show the same content. 
To allow changing the channel, the IR receiver is placed in bedroom. The output of the
repeater is connected via 2 of the remaining 4 Cat5 wires to the transmitter diode in 
the living room. When pressing a button on the remote control in the bedroom, the repeater
decodes the remote control's code, and places the same code on the output which is sent 
to the transmitter diode. This diode is placed in front of the IR input of the set-top-box. 
