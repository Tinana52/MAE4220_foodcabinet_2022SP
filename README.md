# MAE4220_foodcabinet_2022SP
This is the github repository for MAE4220 food cabinet project

The TTN_decoder code is used as the uplink payload formatter in The Things Network. It decodes three floats, which represent the weight on the three shelves, from bytes.

The setup_ttn code is used with the Arduino IDE. It declares our data packet, initialize measurements and defines how we collect data from sensors. It also takes care of FRAM storage, which ensures we can resume data collection even if the event of disconnection due to either power outage or broken electrical components occur. 
