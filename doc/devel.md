# caio - 8 bits home computers emulator

## Architecture

The core of the emulator is a library that provides generic methods (signal
processing, keyboard translators, joystick support, etc.) and a set of
*software devices* that implement the behaviour of *hardware devices* (RAMs,
ROMs, micro-processors, video controllers, audio controllers, etc.).
By combining these elements, different hardware platforms can be emulated.

The behaviour of the chips are emulated and not their internal circuitry,
in some cases it is very difficult (if not impossible) to achieve a 100% of
accuracy within reasonable levels of complexity and execution speeds:

The most taxing and difficult part during the development of a device
emulator is not the implementation of the device itself but its internal bugs.
In some cases bugs are hidden, making the behaviour of the device quite
different from those described in its technical specifications and difficult
to reproduce, sometimes bugs are known but the documentation explaining them
are inaccurate or hard to find, in other cases, bugs were discovered over
the years and exploited as features, (in)famous examples of these are the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) audio chip and
the [MOS 6569](https://en.wikipedia.org/wiki/MOS_Technology_VIC-II) video
controller, both found on the
[Commodore 64](https://en.wikipedia.org/wiki/Commodore_64).

The efforts required to implement bugs can be enormous, making the development
of a device a time consuming nightmare.
For this reason, when dealing with buggy devices, an internal implementation
is done as much accurate as possible within the boundaries set by the
availability of developer time, the specific physical hardware to emulate,
documentation, and several doses of passion. If the final result is
*not good enough* and third party solutions are available then those solutions
are also used. An example of this is the emulator of the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) chip, where
two different implementations are provided, the internal one which reached an
arguable decent level of accuracy and another one that uses the
[reSID](https://en.wikipedia.org/wiki/ReSID) library which emulates the device
quite accurately thanks to a big deal of reverse engineering work done by its
author.

For more information about caio's internal structure read the source
code, it is fully documented.

## Random notes done during development

### MOS 6581

The very first implementation of the
[MOS 6581](https://en.wikipedia.org/wiki/MOS_Technology_6581) was entirely
based on the specifications found on the
[C64 User's Guide](https://www.c64-wiki.com/wiki/Commodore_64_User%27s_Guide),
since ["Many characteristics of the SID sound are bugs"](https://www.c64-wiki.com/wiki/SID#Trivia)
it did not sound like the original chip.

The current implementation of this device evolved a lot since its first
incarnations reaching better levels of emulation accuracy.
Still, it is far from being *really good* so another implementation that uses
the [reSID](https://github.com/daglem/reSID) library is also provided.

#### Filters:

The specifications say that filter frequencies are defined by the following
formula:

```
  fc = FC_MIN + (FC_MAX - FC_MIN) * ufc / 2048
```

where `ufc` is the 11 bits frequency value set by the user.
This formula is not correct. The relationship between the user defined
values and the actual filter frequencies are non-linear and every single SID
chip out there sounds different from the other.

Not having a physical SID to play with, we started looking at other emulators
of this chip, in particular, the [reSID](https://en.wikipedia.org/wiki/ReSID)
library, whose author did a big deal of reverse engineering. The version 0.16
of this library uses a look-up table to translate between user defined values
and filter frequencies.
In caio's implementation, the curve from this table is approximated using two
[sigmoid functions](https://en.wikipedia.org/wiki/Sigmoid_function).

The plot below shows the look-up table used by reSID, the sigmoid approximation
used by caio, and the formula described in the technical specifications:

<img src="filter.png" width="640"/>

The resonance value is also translated to a quadratic response Q factor, but
it is not mentioned in the documentation how this factor is obtained.
The author of reSID-v0.16 found that this value varies from 0.707 to 1.7,
but it seems that neither that is correct.
At the moment, caio's implementation uses the following linear formula to
calculate the actual Q factor:

```
  Q = 0.707 + (1.7 - 0.707) * res / 15

```
where `res` is the 4 bits resonance value specified by the user.
This formula is not correct and it will change in a near future.

#### Other bugs:

The volume *click* bug and others are implemented so programs like
[SAM](https://en.wikipedia.org/wiki/Software_Automatic_Mouth) work.

