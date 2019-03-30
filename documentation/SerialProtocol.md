OpenBeacon Mini Serial Communication Protocol
=============================================

Synopsis
--------
The OpenBeacon Mini serial communications protocol is a flexible, discoverable, yet compact way to pass data between OpenBeacon Mini and a PC attached via the USB-UART. This allows for synchronization of time, setting of all configuration parameters, and complete control of OpenBeacon Mini from the PC.

Packet Description
------------------
A well-formed serial packet consists of three main components: a header, an optional JSON payload, and a termination byte. The header is formed from one identification byte (always 0x07/ASCII BEL), followed by a one-byte message type, and then a two-byte JSON payload length in big-endian format.

The optional payload is a minified JSON string whose length in bytes must match the payload length field specified previously. Because this protocol is meant to be implemented on a microcontroller, the maximum length of this JSON string is 500 bytes.

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
|   max 500 bytes    |
+--------------------+
| END [0x0A/LF] (1)  |
+--------------------+
```

Message Types Summary
---------------------
| Type | Name | Sender | Description |
|------|------|--------|-------------|
| 0x00 | Time Sync Request | OBM | Request current time from PC |
| 0x01 | Time Sync Response | PC | Send current time to OBM |
| 0x02 | Control Request | PC | Request to set a configuration value or execute action|
| 0x03 | Control Response | OBM | Confirm control request |
| 0xFE | Notification | OBM | Notification to client PC |
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

- ### 0xFE &mdash; Notification

#### Required Fields

None

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |
| level | uint8_t | Notification level |
| text | string | Notification text |

- ### 0xFF &mdash; Error

#### Required Fields

None

#### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| id | uint64_t | Unique packet ID |
| type | uint8_t | Error number |
| name | string | Text description of error |

Acknowledgments
---------------
This design was inspired by a [blog post](http://esr.ibiblio.org/?p=8254) from Eric S. Raymond about the design philosophy behind a robust wire protocol (specifically his work on NTPv5). He convinced me to move from a wholly binary to a more flexible mixed protocol with a JSON payload.
