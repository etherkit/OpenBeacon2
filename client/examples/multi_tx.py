from time import sleep

band_change = False
modes = ['WSPR', 'DFCW6', 'JT65']

def TXEnd():
    global band_change

    band_change = True

# Register callback for TX End
self.register('tx_end', TXEnd)

self.async_alert("Script begin - Ctrl-C to terminate")

try:
    app('set buffer 1')
    app('set rnd_tx true')
    app('tx enable')

    while True:
        for mode in modes:
            app('set mode {}'.format(mode))
            for band in self.available_bands:
                app('set band {}'.format(band))
                while band_change == False:
                    sleep(0.001)
                band_change = False
except:
    app('tx cancel')
    app('tx disable')
    self.async_alert("Script termniation")

app('tx disable')
self.async_alert("Script end")