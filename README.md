# IO433

![logo](docs/logo.png)

ESP32 (TTGO T-Display) & CC1101 based 433Mhz sniffer

```bash
│  # The current dir is a platformIO project and can be imported straight for here.
├── docs     # docs and pictures
└── [others] # platformio project files and libraries
```

## Bill of Materials

| Qty. | Description | Obs. |
|------|------------------------|-----------------------------------------------------------------|
| 1 | ESP32 TTGO T-Display | ESP32 controller with two buttons and small LCD for navigation |
| 1 | TI-CC1101 | A CC1101 based module for 433Mhz communications |
| 8 | Wires | Some wires :) |

![IO433 Parts](docs/parts.png)

* All the components can be found in the usual online stores such as *Aliexpress* and *eBay*.


## IO433 Wiring

### Prototype photo and assembly suggestion

![IO433 PCB](docs/io433-prototype.png)

This is just a suggestion on how to wire the ESP32 and the CC1101. A perforated PCB can be used to make the prototype more sturdy.

### IO433 Wiring

![IO433 Wiring](docs/io433-wiring.png)

Details of the wiring between the ESP32 and CC1101. The pinout choosing is important. If changed, not only the code should change to reflect the new pinout, as one must make sure the corresponding ports on the ESP32 support the I/O operation mode that the code needs. 

## How to flash IO433

### Using [Platformio](https://platformio.org/)

* Clone this repository 
* Open in PlatformIO
* Build and upload

## How to use it?

Plug the ESP32 to a battery or a USB-C cable. The current menu structure is the following:

```bash
│ Main
├── Copy   # Copy signal to current memory bank
├── Replay # Replay signal from current memory bank
├── Dump   # Dump current memory bank to screen
└── More
    ├── Monitor # Dump raw signal to screen and RSSI info
    ├── Raw Out # Dump current signal to serial as fast as possible (default 1000000bps serial)
    ├── About   # About menu
```

Button behaviour:

* UP and DOWN short press, moves between menu items
* UP long press is moves back a level
* DOWN long press enters current submenu or function
* UP or DOWN double press moves back and forward from memory banks (to store/replay multiple signals) 

