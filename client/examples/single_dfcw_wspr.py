"""Single Band DFCW and WSPR Script

This script will transmit buffer 1 in DFCW6 mode on a fixed frequency 
within a band and then immediately transmit WSPR on the same band on
a random frequency within the WSPR sub-band.
"""

from time import sleep

tx_over = False

# Edit these values for your desired DFCW transmission frequencies
dfcw_freq_table = {"80m": 3560000,
                   "60m": 5358000,
                   "40m": 7039800,
                   "30m": 10139900,
                   "20m": 14096800,
                   "17m": 18080000,
                   "15m": 21060000,
                   "12m": 24926000,
                   "10m": 28060000}

callsign = 'NT7S'
band = '30m'  # Change the desired band here
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
    app('set band {}'.format(band))

    while True:
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
    self.async_alert("Script termination")

app('tx disable')
self.async_alert("Script end")
