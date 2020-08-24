"""Multiband DFCW and WSPR Script

This script will transmit buffer 1 in DFCW6 mode on a fixed frequency 
within a band and then immediately transmit WSPR on the same band on
a random frequency within the WSPR sub-band. All available bands will
be cycled through.
"""

from time import sleep

tx_over = False

# Edit these values for your desired DFCW transmission frequencies
dfcw_freq_table = {"80m": 3560000,
                   "60m": 5358000,
                   "40m": 7039843,
                   "30m": 10139943,
                   "20m": 14096843,
                   "17m": 18080043,
                   "15m": 21060000,
                   "12m": 24926000,
                   "10m": 28060000}

callsign = 'NT7S'
buffer = '{} HNY'.format(callsign)  # Change the message here


def TXEnd():
    global tx_over

    tx_over = True


# Register callback for TX End
self.register('tx_end', TXEnd)

self.async_alert("Script begin - Ctrl-C to terminate")

try:
    app('set callsign {}'.format(callsign))
    app('set msg_buffer_1 \"{}\"'.format(buffer))
    app('set buffer 1')

    while True:
        for band in self.available_bands:
            app('set band {}'.format(band))

            # DFCW6 transmission
            app('set mode dfcw6')
            app('set rnd_tx false')
            app('set base_freq {}'.format(dfcw_freq_table[band]))
            app('tx enable')
            while tx_over == False:
                sleep(0.001)
            tx_over = False
            app('tx disable')

            # WSPR transmission
            app('set mode wspr')
            app('set rnd_tx true')
            app('tx enable')
            while tx_over == False:
                sleep(0.001)
            tx_over = False
            app('tx disable')

except:
    app('tx cancel')
    app('tx disable')
    self.async_alert("Script termniation")

app('tx disable')
self.async_alert("Script end")
