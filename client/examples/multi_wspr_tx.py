class termcolors:
    FG_BLUE = '\033[94m'
    FG_YELLOW = '\033[93m'
    FG_GREEN = '\033[92m'
    FG_RED = '\033[91m'
    FG_MAGENTA = '\033[95m'
    FG_WHITE = '\033[97m'
    FG_BLACK = '\033[30m'
    BG_GREEN = '\033[42m'
    BG_RED = '\033[41m'
    BG_BLACK = '\033[40m'
    BOLD = '\033[1m'
    NORMAL = '\033[0m'
    UNDERLINE = '\033[4m'
    ENDC = '\033[0m'

band_change = False

def TXEnd():
    global band_change

    band_change = True

# Register callback for TX End
self.register('tx_end', TXEnd)

# self.prompt = termcolors.FG_MAGENTA + ']' + termcolors.NORMAL + ' '
self.async_alert("Script begin - Ctrl-C to terminate")

try:
    app('set buffer 1')
    app('set rnd_tx true')
    app('tx enable')

    while True:
          for band in self.available_bands:
              app('set band {}'.format(band))
              while band_change == False:
                  pass
              band_change = False
except:
    app('tx cancel')
    app('tx disable')
    self.async_alert("Script termniation")

app('tx disable')
self.async_alert("Script end")
# self.prompt = termcolors.FG_MAGENTA + '>' + termcolors.NORMAL + ' '
