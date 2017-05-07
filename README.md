# Button Basher

Using a [Makey Makey](http://www.makeymakey.com/) to control Spotify and play sounds through a mac, via buttons and other physical inputs.


## Setup - Makey Makey

First, set up the Makey Makey with custom keyboard mappings. It can't use the default key bindings unless you want to make the machine unusable for anything else. Instead, the controls should be mapped to keys that won't otherwise be used.

Update [Makey Makey](http://www.makeymakey.com/) firmware to map custom keys. I have an older device and so followed [this tutorial](https://learn.sparkfun.com/tutorials/makey-makey-advanced-guide). If you have a newer device you may be able to do it [through a browser](http://makeymakey.com/remap/).

I wanted to remap multiple keys, which the default Makey Makey firmware doesn't let you do, so I adapted [this firmware](https://github.com/DavidRieman/MaKeyMaKey_Flexible/) instead.

My firmware is in this repo, in the `/arduino` directory.

The article mentioned above contains detailed set-up instructions, here in brief is the process I followed:

   - Download [Arduino](http://arduino.cc/) app
   - Download either the [custom firmware](https://github.com/DavidRieman/MaKeyMaKey_Flexible/), the [original Makey Makey firmware](https://github.com/sparkfun/MaKeyMaKey) or use my version, and open the main arduino `.ino` file in the Arduino IDE
   - Add the _Additional Board Manager URL_ in the Arudino IDE settings:

    ```
    https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
    ```
   - From the _Board Manager_ menu select _SparkFun AVR Boards_
   - Select the Makey Makey board
   - Set the correct serial port

Once this is done, you should be able to connect buttons and other inputs to the Makey Makey. Instead of the default keys they should now map to your custom keys.

My custom firmware uses the 6 analog and 6 digital pins as button inputs. In addition the 'left' arrow holes is used for another button input, and the 'click' holes are used as an LED output.

An easy way to test this is through the [Keypress](https://dmauro.github.io/Keypress/) website that shows you onscreen the keys that are currently being pressed.


## Setup - mac

### Creating custom services with Automator

Once the Makey Makey is ready, the mac can be set up to detect and act on keyboard input with new services triggered by custom shortcuts.

I set up new services in Automator following [these instructions](http://apple.stackexchange.com/questions/175215/how-do-i-assign-a-keyboard-shortcut-to-an-applescript-i-wrote):

  - Launch the Automator app
  - Create a new service
  - Select "no input" and "any application"
  - Select "Run AppleScript" or ""
  - Enter AppleScript (see below)
  - Save the service

Here are some example AppleScript services:

`SpotifyToggle`

```
-- Toggle
on run args
	try
		tell application "Spotify"
			playpause
		end tell
	end try
end run
```

`SpotifyPrevious`

```
-- Previous
on run args
  try
    tell application "Spotify"
      previous track
    end tell
  end try
end run
```

`SpotifyNext`

```
-- Next
on run args
  try
    tell application "Spotify"
      next track
    end tell
  end try
end run
```

Alternatively, here are some custom services using Python, which can be run as a shell script:

`button-sfx`

```
import subprocess
subprocess.call(["afplay", "/path/to/button-basher/sfx/b.mp3"])
```

`SpotifyPauseplay`

```
import subprocess

osa = subprocess.Popen(['osascript', '-'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
state = osa.communicate('tell application "Spotify" to return player state')[0]

if state.strip() == "playing":
	subprocess.call(['osascript', '-e', 'tell application "Spotify" to pause'])
	subprocess.call(["afplay", "/path/to/button-basher/sfx/pause.mp3"])
else:
	subprocess.call(["afplay", "/path/to/button-basher/sfx/play.mp3"])
	subprocess.call(['osascript', '-e', 'tell application "Spotify" to play'])

```

### Triggering services with a custom keyboard shortcut

To set up custom keyboard shortcuts:

  - System Preferences > Keyboard > Shortcuts
  - Select Services
  - Add a shortcut for your service - click the name, then use the Makey Makey to add the input
  - System Preferences > Security & Privacy > Privacy
  - Select Accessibility
  - Click on the + sign, add Automator and Finder (`/System/Library/CoreServices/Finder.app`)
  - From any application, go to the menu and run the new shortcut once manually

That's it - now when you click on a button with the Makey Makey, the custom service should run.
