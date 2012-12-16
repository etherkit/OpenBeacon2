/*
 * main.c
 *
 *  Created on: Nov 20, 2012
 *      Author: Jason Milldrum
 *     Company: Etherkit
 *
 *     Copyright (c) 2012, Jason Milldrum
 *     All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice, this list
 *  of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice, this list
 *  of conditions and the following disclaimer in the documentation and/or other
 *  materials provided with the distribution.
 *
 *  - Neither the name of Etherkit nor the names of its contributors may be
 *  used to endorse or promote products derived from this software without specific
 *  prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 *  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include "morsechar.h"
#include "font.h"

// Hardware Defines
#define FSK_DDR					DDRB
#define FSK_PORT				PORTB
#define FSK						PB4

#define KEY_DDR					DDRB
#define KEY_PORT				PORTB
#define KEY						PB3

#define S1_DDR					DDRB
#define S1_PORT					PORTB
#define S1_PIN					PINB
#define S1						PB0

#define S2_DDR					DDRB
#define S2_PORT					PORTB
#define S2_PIN					PINB
#define S2						PB1

#define S3_DDR					DDRB
#define S3_PORT					PORTB
#define S3_PIN					PINB
#define S3						PB2

// Firmware constant defines
#define DFCW_DEFAULT_OFFSET		100

#define MULT_DAH				3			// DAH is 3x a DIT
#define MULT_WORDDELAY			7			// Space between words is 7 dits
#define MULT_HELL_WORDDELAY		200
#define MULT_HELL_CHARDELAY		20
#define MULT_HELL_GLYPHDELAY	60
#define HELL_ROW_RPT			3

#define DEFAULT_MODE			0
#define DEFAULT_WPM				12000

#define MSG_DELAY				10			// in minutes

#define CWID_DELAY				10			// CW ID interval in minutes
#define CWID_WPM				20000		// CW ID at 20 WPM

#define MODE_COUNT				5
#define MODE_DEFAULT			MODE_DFCW6

#define MSG_BUFFER_SIZE			41			// Message size in characters (+1 for \0)

// Enumerations
enum BOOL {FALSE, TRUE};
enum STATE {STATE_IDLE, STATE_DIT, STATE_DAH, STATE_DITDELAY, STATE_DAHDELAY, STATE_WORDDELAY, STATE_MSGDELAY,
			STATE_CHARDELAY, STATE_HELLCOL, STATE_HELLROW, STATE_CAL, STATE_WSPR, STATE_WSPR_INIT, STATE_PREAMBLE, STATE_HELLIDLE};
enum MODE {MODE_DFCW6, MODE_QRSS6, MODE_HELL, MODE_CW, MODE_CAL};


// Global variables
uint32_t cur_timer = 0;
uint32_t dit_length;
enum MODE cur_mode, prev_mode;
enum STATE cur_state, prev_state;
uint32_t cur_state_end, msg_delay_end, prev_state_end;
static char msg_buffer[MSG_BUFFER_SIZE];
char * cur_msg_p;
char * prev_msg_p;
char cur_character = '\0';
char prev_character = '\0';
char cur_hell_char = '\0';
uint8_t cur_hell_col = 0;
uint8_t cur_hell_row = 0;
uint16_t wpm, prev_wpm;
uint8_t msg_delay;
uint8_t dfcw_offset;
enum BOOL cwid = FALSE;
uint32_t next_cwid;

// Global variables used in ISRs
volatile uint32_t timer; // A 32-bit timer will count for 2^32 * 1 ms = ~8 years

// EEPROM variables
char EEMEM ee_msg_mem[MSG_BUFFER_SIZE] = "N0CALL";

// Global constants
const uint8_t hell_tune[HELL_ROWS] PROGMEM = {252, 185, 140, 85, 53, 23, 0};
const uint16_t dit_speed[MODE_COUNT] = {200, 200, 3500, 15000, 10000};

// Function prototypes
void set_wpm(uint32_t);
uint32_t get_msg_delay(uint8_t);
void init_tx(void);
void debounce(void);
void reset_buffer(void);
void init_cwid(void);
void tx_on(void);
void tx_off(void);

// Interrupt service routine
ISR(TIM0_COMPA_vect)
{
	// Tick the clock
	timer++;

	debounce();
}

void set_wpm(uint32_t new_wpm)
{
	// This is WPM * 1000 due to need for fractional WPM for slow modes
	//
	// Dit length in milliseconds is 1200 ms / WPM
	dit_length = (1200000L / new_wpm);
}

uint32_t get_msg_delay(uint8_t delay_minutes)
{
	// Number of clock ticks is the number of minutes * 59956 ticks/per min
	return (uint32_t)delay_minutes * 59956L;
}

void init_tx(void)
{
	// Reset the message buffer
	reset_buffer();

	// If in message delay mode, set the delay
	msg_delay_end = cur_timer + get_msg_delay(msg_delay);

	// Reset WPM
	wpm = dit_speed[cur_mode];
	set_wpm(wpm);


	// Reset to IDLE state
	cur_state_end = cur_timer;
	cur_state = STATE_IDLE;
}

void debounce(void)
{
	enum BOOL S1_active, S2_active, S3_active;

	S1_active = bit_is_clear(S1_PIN, S1);
	S2_active = bit_is_clear(S2_PIN, S2);
	S3_active = bit_is_clear(S3_PIN, S3);

	// Set mode
	if((!S1_active) && (!S2_active))
	{
		if(cur_mode != MODE_DFCW6 && cur_mode != MODE_CW)
		{
			cur_mode = MODE_DFCW6;
			init_tx();
		}
	}
	else if((S1_active) && (!S2_active))
	{
		if(cur_mode != MODE_QRSS6 && cur_mode != MODE_CW)
		{
			cur_mode = MODE_QRSS6;
			init_tx();
		}
	}
	else if((!S1_active) && (S2_active))
	{
		if(cur_mode != MODE_HELL && cur_mode != MODE_CW)
		{
			cur_mode = MODE_HELL;
			init_tx();
		}
	}
	else if((S1_active) && (S2_active))
	{
		if(cur_mode != MODE_CAL && cur_mode != MODE_CW)
		{
			cur_mode = MODE_CAL;
			init_tx();
		}
	}

	// Set message delay
	if(!S3_active)
		msg_delay = 0;
	else if(S3_active)
		msg_delay = MSG_DELAY;
}

void reset_buffer(void)
{
	memset(msg_buffer, '\0', MSG_BUFFER_SIZE);
	eeprom_read_block((void*)&msg_buffer, (const void*)&ee_msg_mem, MSG_BUFFER_SIZE - 1);
	cur_msg_p = msg_buffer;
	cur_character = '\0';
}

void init_cwid(void)
{
	cwid = TRUE;
	prev_mode = cur_mode;
	prev_wpm = wpm;
	prev_character = cur_character;
	prev_msg_p = cur_msg_p;
	prev_state_end = cur_state_end;
	prev_state = cur_state;

	cur_mode = MODE_CW;
	wpm = dit_speed[cur_mode];
	set_wpm(wpm);
	reset_buffer();

	// Give a DAH delay w/ TX off so we can properly distinguish CW from QRSS
	cur_state_end = cur_timer + (dit_length * MULT_DAH);
	cur_state = STATE_DAHDELAY;
}

void tx_on(void)
{
	KEY_PORT |= _BV(KEY);
}

void tx_off(void)
{
	KEY_PORT &= ~(_BV(KEY));
}

int main(void)
{
	// Set up Timer0 for event timer
	// 16.5 MHz clock, /256 prescale, 125 count = 1 ms timer interrupt
	TCCR0A |= _BV(WGM01); // CTC mode
	TCCR0B = _BV(CS01); // Prescale /8, gives 8 us clock tick
	OCR0A = 124;
	TIMSK |= _BV(OCIE0A); // Enable CTC interrupt

	// Set up Timer1 for fast PWM (500 kHz)
	TCCR1 = _BV(CS10);
	GTCCR = _BV(PWM1B) | _BV(COM1B1);
	OCR1B = 0; // Initial PWM value
	OCR1C = 255;
	PLLCSR = _BV(PLLE) | _BV(PCKE);
	OCR1B = 255; // Initial PWM value

	// Initialize ports
	FSK_DDR |= _BV(FSK);
	FSK_PORT &= ~(_BV(FSK));

	KEY_DDR |= _BV(KEY);
	KEY_PORT |= _BV(KEY);

	S1_DDR &= ~(_BV(S1));
	S1_PORT |= _BV(S1); // Enable pull-up

	S2_DDR &= ~(_BV(S2));
	S2_PORT |= _BV(S2); // Enable pull-up

	S3_DDR &= ~(_BV(S3));
	S3_PORT |= _BV(S3); // Enable pull-up

	dfcw_offset = DFCW_DEFAULT_OFFSET;

	// Transmitter off
	tx_off();

	// Set up the message buffer
	memset(msg_buffer, '\0', MSG_BUFFER_SIZE);
	cur_msg_p = msg_buffer;

	// Initialize states
	cur_mode = MODE_DEFAULT;
	cur_state = STATE_IDLE;

	eeprom_read_block((void*)&msg_buffer, (const void*)&ee_msg_mem, MSG_BUFFER_SIZE - 1);
	msg_delay_end = cur_timer + get_msg_delay(msg_delay);

	next_cwid = cur_timer + get_msg_delay(CWID_DELAY);

	init_tx();

	sei();


	while(1)
	{
		// Latch the current time
		// MUST disable interrupts during this read or there will be an occasional corruption of cur_timer
		cli();
		cur_timer = timer;
		sei();

		// Handle CW ID if one hasn't been triggered in 10 minutes
		if(cur_timer > next_cwid && !cwid && cur_mode != MODE_CW)
		{
			init_cwid();
			next_cwid = cur_timer + get_msg_delay(CWID_DELAY);
		}

		// State machine
		switch(cur_mode)
		{
		case MODE_DFCW6:
		case MODE_QRSS6:
		case MODE_CW:
			switch(cur_state)
			{
			case STATE_IDLE:
				// TX off
				tx_off();

				if(msg_delay > 0 && msg_delay_end <= cur_timer && cur_msg_p == msg_buffer)
				{
					msg_delay_end = cur_timer + get_msg_delay(msg_delay);
					cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
					//if(cur_mode != MODE_CW)
						cur_state = STATE_PREAMBLE;
					//else
						//cur_state = STATE_IDLE;
					break;
				}

				// If this is the first time thru the message loop, get the first character, then wait a moment before starting message if not CW
				if((cur_msg_p == msg_buffer) && (cur_character == '\0'))
				{
					cur_character = pgm_read_byte(&morsechar[(*cur_msg_p) - MORSE_CHAR_START]);
					if(cur_mode != MODE_CW)
					{
						cur_state_end = cur_timer + (dit_length * MULT_DAH);
						cur_state = STATE_PREAMBLE;
						break;
					}
				}

				// Get the current element in the current character
				if(cur_character != '\0')
				{
					if(cur_character == 0b10000000 || cur_character == 0b11111111)	// End of character marker or SPACE
					{
						// Set next state based on whether EOC or SPACE
						if(cur_character == 0b10000000)
						{
							cur_state_end = cur_timer + (dit_length * MULT_DAH);
							cur_state = STATE_DAHDELAY;
						}
						else
						{
							cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
							cur_state = STATE_WORDDELAY;
						}

						// Grab next character, set state to inter-character delay
						cur_msg_p++;

						// If we read a NULL from the announce buffer, set cur_character to NULL,
						// otherwise set to correct morse character
						if((*cur_msg_p) == '\0')
							cur_character = '\0';
						else
							cur_character = pgm_read_byte(&morsechar[(*cur_msg_p) - MORSE_CHAR_START]);
					}
					else
					{
						// Mask off MSb, set cur_element
						if((cur_character & 0b10000000) == 0b10000000)
						{
							cur_state_end = cur_timer + (dit_length * MULT_DAH);
							cur_state = STATE_DAH;
						}
						else
						{
							cur_state_end = cur_timer + dit_length;
							cur_state = STATE_DIT;
						}

						// Shift left to get next element
						cur_character = cur_character << 1;
					}
				}
				else // Buffer is now empty
				{
					// If in CW ID mode, reset back to original parameters
					if(cwid)
					{
						cur_mode = prev_mode;
						wpm = dit_speed[cur_mode];
						set_wpm(wpm);
						/*
						memset(msg_buffer, '\0', WSPR_BUFFER_SIZE);
						if(cur_buffer == BUFFER_1)
							eeprom_read_block((void*)&msg_buffer, (const void*)&ee_msg_mem_1, MSG_BUFFER_SIZE - 1);
						else
							eeprom_read_block((void*)&msg_buffer, (const void*)&ee_msg_mem_2, MSG_BUFFER_SIZE - 1);
							*/
						reset_buffer();
						cur_msg_p = prev_msg_p;
						cur_character = prev_character;
						next_cwid = cur_timer + get_msg_delay(CWID_DELAY);
						cwid = FALSE;

						cur_state_end = prev_state_end;
						cur_state = prev_state;

						// If in HELL mode, wait a bit before starting next message
						/*
						if(cur_mode == MODE_HELL)
						{
							cur_state_end = cur_timer + (dit_length * MULT_HELL_WORDDELAY);
							cur_state = STATE_WORDDELAY;
						}
						else
						{
							cur_state_end = prev_state_end;
							cur_state = prev_state;
						}
						*/
					}
					else
					{
						// Reload the message buffer and set buffer pointer back to beginning
						reset_buffer();

						if(msg_delay == 0)
						{
							// If a constantly repeating message, put a word delay at the end of message
							cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
							cur_state = STATE_WORDDELAY;
						}
						else
						{
							// Otherwise, set the message delay time
							if(msg_delay_end < cur_timer + (dit_length * MULT_WORDDELAY))
								cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
							else
								cur_state_end = msg_delay_end;

							cur_state = STATE_MSGDELAY;
						}


						// Do a CW ID
						if(cur_mode != MODE_CW)
						{
							init_cwid();
							next_cwid = cur_timer + get_msg_delay(CWID_DELAY);
						}

					}
				}

				break;

			case STATE_PREAMBLE:
				// Wait a word delay with TX on before starting message
				OCR1B = 0;

				if(cur_mode == MODE_QRSS6 || cur_mode == MODE_CW)
				{
					// Transmitter off
					tx_off();
				}
				else
				{
					// Transmitter on
					tx_on();
				}


				// When done waiting, go back to IDLE state to start the message
				if(cur_timer > cur_state_end)
				{
					cur_state = STATE_IDLE;
				}
				break;

			case STATE_DIT:
			case STATE_DAH:
				switch(cur_mode)
				{
				case MODE_DFCW6:
					// Transmitter on
					tx_on();

					// Set FSK to MARK (lower capacitance/higher freq)
					OCR1B = dfcw_offset;
					break;
				case MODE_QRSS6:
				case MODE_CW:
					// Transmitter on
					tx_on();

					// Set FSK to 0 (maximum capacitance/minimum freq)
					OCR1B = 0;
					break;
				default:
					break;
				}

				if(cur_timer > cur_state_end)
				{
					switch(cur_mode)
					{
					case MODE_DFCW6:
						// Transmitter on
						tx_on();

						// Set FSK to 0 (maximum capacitance/minimum freq)
						OCR1B = 0;
						break;
					case MODE_QRSS6:
					case MODE_CW:
						// Transmitter off
						tx_off();

						// Set FSK to 0 (maximum capacitance/minimum freq)
						OCR1B = 0;
						break;
					default:
						break;
					}

					cur_state_end = cur_timer + dit_length;
					cur_state = STATE_DITDELAY;
				}
				break;
			case STATE_DITDELAY:
			case STATE_DAHDELAY:
			case STATE_WORDDELAY:
			case STATE_MSGDELAY:
				OCR1B = 0;

				if(cur_state == STATE_MSGDELAY || cur_mode == MODE_QRSS6 || cur_mode == MODE_CW)
				{
					// Transmitter off
					tx_off();
				}
				else
				{
					// Transmitter on
					tx_on();
				}

				if(cur_timer > cur_state_end)
				{
					cur_state = STATE_IDLE;
				}
				break;


			default:
				break;
			}
			break;

		case MODE_HELL:
			switch(cur_state)
			{
			case STATE_IDLE:
				if(msg_delay > 0 && msg_delay_end <= cur_timer)
				{
					msg_delay_end = cur_timer + get_msg_delay(msg_delay);
				}

				// If this is the first time thru the message loop, get the first character
				if((cur_msg_p == msg_buffer) && (cur_hell_char == '\0'))
				{
					cur_hell_col = 0;
					cur_hell_char = pgm_read_byte(&fontchar[(*cur_msg_p) - FONT_START][cur_hell_col++]);
					cur_state_end = cur_timer + (dit_length);
					cur_state = STATE_HELLCOL;
				}
				else
				{
					cur_hell_char = pgm_read_byte(&fontchar[(*cur_msg_p) - FONT_START][cur_hell_col++]);

					if(cur_hell_col > HELL_COLS)
					{
						// Reset Hell column
						cur_hell_col = 0;

						// Grab next character
						cur_msg_p++;

						if((*cur_msg_p) == '\0')
						{
							// End of message
							// Reload the message buffer and set buffer pointer back to beginning
							reset_buffer();

							if(msg_delay == 0)
							{
								// If a constantly repeating message, put a word delay at the end of message
								cur_state_end = cur_timer + (dit_length * MULT_HELL_WORDDELAY);
							}
							else
							{
								// Otherwise, set the message delay time
								if(msg_delay_end < cur_timer + (dit_length * MULT_HELL_WORDDELAY))
									cur_state_end = cur_timer + (dit_length * MULT_HELL_WORDDELAY);
								else
									cur_state_end = msg_delay_end;
							}

							cur_state = STATE_WORDDELAY;

							// Do a CW ID
							init_cwid();
							next_cwid = cur_timer + get_msg_delay(CWID_DELAY);
						}
						else
						{
							cur_state_end = cur_timer + (dit_length * MULT_HELL_CHARDELAY);
							cur_state = STATE_CHARDELAY;
						}
					}
					else
					{
						//cur_hell_char = pgm_read_byte(&fontchar[(*cur_msg_p) - FONT_START][cur_hell_col]);
						cur_state_end = cur_timer + (dit_length);
						cur_state = STATE_HELLCOL;
					}
				}
				break;
			case STATE_HELLCOL:
				OCR1B = pgm_read_byte(&hell_tune[cur_hell_row]);
				if((cur_hell_char & (1 << cur_hell_row)) != 0)
				{
					// Pixel on
					tx_on();
				}
				else
				{
					// Pixel off
					tx_off();
				}

				if(cur_timer > cur_state_end)
				{
					cur_hell_row++;
					if(cur_hell_row > HELL_ROWS)
					{
						cur_hell_row = 0;
						cur_state = STATE_IDLE;
					}
					else
					{
						cur_state_end = cur_timer + dit_length;
						cur_state = STATE_HELLCOL;
					}
				}
				break;

			case STATE_WORDDELAY:
			case STATE_CHARDELAY:
				OCR1B = 0;

				// Transmitter off
				tx_off();

				if(cur_timer > cur_state_end)
					cur_state = STATE_IDLE;
				break;
			default:
				cur_state = STATE_IDLE;
				break;
			}
			break;

		case MODE_CAL:
			switch(cur_state)
			{
			case STATE_IDLE:
				tx_off();
				cur_hell_row++;
				if(cur_hell_row > HELL_ROWS)
					cur_hell_row = 0;

				cur_state_end = cur_timer + dit_length;
				cur_state = STATE_CAL;
				break;
			case STATE_CAL:
				tx_on();
				OCR1B = pgm_read_byte(&hell_tune[cur_hell_row]);

				if(cur_timer > cur_state_end)
					cur_state = STATE_IDLE;
				break;
			default:
				cur_state = STATE_CAL;
				break;
			}
			break;
		default:
			// Switch to a default mode???
			break;
		}

	}
}


