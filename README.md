![Image of an X32 Console](https://github.com/user-attachments/assets/7d64a87e-0f17-4001-9129-11fba84c0852)

# Progress Updates...!

The current iteration! We now not only have a a functioning header bar, but also a functioning sidebar! This only took
like 48 hours to figure out over the past month... thanks C++. Do you understand my username now?
![UI with Sidebar and commands](Readmeassets/img.png)


# Welcome to XM32CE!

XM32CE is a program built in C++ (using the JUCE Framework) that provides realtime OSC cues designed for live
production use. It's designed with realtime performance, stability and precision in mind and as the name suggests, is
specially designed to be compatible with the Behringer and Midas X/M32 lineup of consoles.

## How does XM32CE work?

OSC! Open Sound Control 1.0 is the specification used by X/M32 consoles to communicate with other network devices. 
You'll be shocked to hear that XM32CE uses OSC!11!!!1! In all seriousness, all commands are manually sent as individual
commands using a "translation layer" between the program's action data and the actual OSC commands. For example, a
fade command is represented in the `CueOSCAction` struct, but is then translated into individual OSC commands to make
the fade possible.

## What's the difference? X/M32 consoles already have cue & scene support...

XM32CE stands for X/M32 Control **Extensions**. The main benefit of this app is support for fading (and not just for 
values), all controls that can be sent via OSC (i.e., basically everything) and a dedicated machine to manage it all. 
The integrated cue and scene functions of the X/M32 have never been particularly... extensive.
Also, it's much easier to make cues on a computer than programming it on an actual console and a lot easier to 
view the cues in a real production environment using a dedicated app. 


## I can't afford a ~$2000 AUD console... how do I test this app?

You'll be glad to hear that you're not the only broke one here. There is a man who deserves a Nobel Peace Prize for
making a whole assortment of X/M32 tools and documentation. This includes an X32 emulator, which does everything an X32
processor does... just without processing audio. But that makes it perfect for testing an app like mine, which handles
OSC communications. You can find this emulator and other cool stuff Maillot's website 
[linked here](https://sites.google.com/site/patrickmaillot/x32).

Run the X32 Emulator, then simply use an app that could control an X/M32 console (e.g., Mixing Station, X/M32 Edit,
etc.) and put in the IP address of the device running X32 Emulator. Put in the same IP address on this app, and violà!
Magic!

> *Note*: Patrick-Gilles Maillot, if you're reading this, you deserve a Nobel Peace Prize for all the utils you've made
> because without them, I probably would've crashed out so hard, I would have internally combusted and blown up
> everything within a 500km radius of Sydney CBD. In other words, thank you!



# Credits!

- **Patrick-Gilles Maillot** - this project would not have been remotely possible without Maillot's documentation,
tools and extensive research and testing with the X/M32 console lineup. I was constantly referring back to his
documentation and using his X32 emulator (in combination with the official M32 Edit app). Check out his website for 
X/M32 tools at [https://sites.google.com/site/patrickmaillot/x32](https://sites.google.com/site/patrickmaillot/x32). 
- **JUCE** - I've been able to build my app on this framework since it's free for educational purposes - which this 
is for (fun fact, this is my HSC major project: wish me luck!) JUCE has made my life exponentially easier; the 
alternative was figuring out OpenGL or Qt...
- **Gemini 2.5 Pro** - wonder how long it took me to write all the Fx configurations in `XM32Maps.h`? Well, not that 
long thanks to Gemini 2.5 Pro being the most **goated** AI model ever. Imagine if I had to type out all 1000+ lines 
myself...
- And a not-so-honourable mention, GitHub Copilot. I still used it... but in all truthfulness, when it wasn't
hallucinating, its suggestions weren't of much help anyway. I planned to use Copilot to avoid having to pull up the 
documentation every 2 seconds, but seeing that the only consistent thing about Copilot was how consistently it suggested
code that raised blatant compiler errors, I gave up trusting at some stage and realised that for once, an AI model was
more clueless than I was.


# Licensing

Copyright © 2025 Luke Liu

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
