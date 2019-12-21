OpenBeacon 2 Serial Communication Protocol
==========================================

Synopsis
--------
The OpenBeacon 2 serial communications protocol is a flexible, discoverable, yet compact way to pass data between OpenBeacon 2 and a PC attached via the USB-UART. This allows for synchronization of time, setting of all configuration parameters, and complete control of OpenBeacon 2 from the PC.

Packet Description
------------------
A well-formed serial packet consists of three main components: a header, an optional JSON payload, and a termination byte. The header is formed from one identification byte (always 0x07/ASCII BEL), followed by a one-byte message type, and then a two-byte JSON payload length in big-endian format.

The optional payload is a minified JSON string whose length in bytes must match the payload length field specified previously. Because this protocol is meant to be implemented on a microcontroller, the maximum length of this JSON string is 400 bytes.

Finally, the end of the packet is indicated with one byte (always 0x0A/ASCII LF).

```
+--------------------+
| ID [0x07/BEL] (1)  |
+--------------------+
| Message Type (1)   |
+--------------------+
| Payload Length (2) |
|                    |
+--------------------+
| JSON Payload (var) |
|        ...         |
|   max 400 bytes    |
+--------------------+
| END [0x0A/LF] (1)  |
+--------------------+
```

Message Types Summary
---------------------

| Type | Name | Sender | Description |
|------|------|--------|-------------|
| 0x00 | Time Sync Request | OB2 | Request current time from PC |
| 0x01 | Time Sync Response | PC | Send current time to OB2 |
| 0x02 | Parameter Request | PC | Request to get or set a configuration value |
| 0x03 | Parameter Response | OB2 | Confirm parameter request |
| 0x04 | Command Request | PC | Request to execute action on OB2 |
| 0x05 | Command Response | OB2 | Confirm command request |
| 0x06 | Enumeration Request | PC | Request a list of valid values in an enumeration
| 0x07 | Enumeration Response | OB2 | List of valid values in an enumeration
| 0x08 | Serialize Configuration Request | PC |
| 0x09 | Serialize Configuration Response | OB2 |
| 0xFE | Notification | OB2 | Notification to client PC |
| 0xFF | Error | Either | Response to an unknown packet type |

Message Types
-------------

- ### 0x00 &mdash; Time Sync Request

#### Required Fields

None

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |

- ### 0x01 &mdash; Time Sync Response

#### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| timestamp | uint64_t | Current time in Unix timestamp format |

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |

- ### 0x02 &mdash; Parameter Request

#### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| config | string | Configuration [parameter](#openbeacon-mini-parameters) to get or set |
| get &#124; set | bool (true) | Get or set directive |

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| value | any | Configuration value (required if a set directive) |
| id | uint64_t | Unique packet ID |

- ### 0x03 &mdash; Parameter Response

#### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| config | string | Configuration parameter to get or set |
| value | any | Configuration value |

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |

- ### 0x04 &mdash; Control Request

#### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| action | string | Action to initiate |

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| value | any | Configuration value |
| id | uint64_t | Unique packet ID |

- ### 0x06 &mdash; Enumeration Request

#### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| enum | string | [Enumeration](#openbeacon-mini-enumerations) list to retrieve |

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |

- ### 0xFE &mdash; Notification

#### Required Fields

None

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |
| level | uint8_t | Notification level |
| text | string | Notification text |
| freq | string | Frequency |
| mode | string | Mode |
| data | string | Additional data |

- ### 0xFF &mdash; Error

#### Required Fields

None

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |
| type | uint8_t | Error number |
| name | string | Text description of error |

OpenBeacon 2 Parameters
--------------------------
| Parameter | Type | Description |
|-----------|------|-------------|
| mode | Mode enum | Operating mode |
| band | uint8_t | Frequency band |
| base_freq | uint32_t | Operating frequency |
| wpm | uint16_t | CW words per minute |
| tx_intv | uint8_t | Transmit interval (in minutes) |
| dfcw_offset | uint8_t | Frequency offset for DFCW mode (in Hz) |
| buffer | uint8_t | Active buffer (1-4) |
| callsign | char[20] | Station callsign |
| grid | char[10] | Station Maidenhead grid locator |
| power | uint8_t | OpenBeacon 2 RF output power (in dBm) |
| pa_bias | uint16_t | TX power amplifier bias voltage (in mV) |
| cwid | bool | CWID after transmission |
| msg_buffer_1 | char[40] | Message buffer 1 |
| msg_buffer_2 | char[40] | Message buffer 2 |
| msg_buffer_3 | char[40] | Message buffer 3 |
| msg_buffer_4 | char[40] | Message buffer 4 |
| si5351_int_corr | int32_t | Si5351A internal reference frequency correction |
| rnd_tx | bool | Transmit on random frequency within mode subband |

OpenBeacon 2 Commands
------------------------
| Action |  Description |
|--------|--------------|
| tx_enable | Enable transmitting |
| tx_disable | Disable transmitting |
| tx_cancel | Cancel current transmission |

OpenBeacon 2 Enumerations
----------------------------
| Enumeration | Description |
|-------------|-------------|
| modes | Operating modes |
| bands | Amateur frequency bands |
| band_modules | OpenBeacon 2 band modules |
| inst_band_modlues | Band modules currently installed |


Acknowledgments
---------------
This design was inspired by a [blog post](http://esr.ibiblio.org/?p=8254) from Eric S. Raymond about the design philosophy behind a robust wire protocol (specifically his work on NTPv5). He convinced me to move from a wholly binary to a more flexible mixed protocol with a JSON payload.
