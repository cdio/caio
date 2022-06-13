# cemu - 8 bits home computers emulator

## c64 sid chip (mos-6581)

The emulation of the MOS6581 chip is based on the specifications found on the
[C64 User's Guide](https://www.c64-wiki.com/wiki/Commodore_64_User%27s_Guide)
but since ["Many characteristics of the SID sound are bugs"](https://www.c64-wiki.com/wiki/SID#Trivia)
cemu's implementation does not sound 100% like the original chip.

### filters:

The specifications say that the actual filter frequencies are defined by the
following formula:

```
  fc = FC_MIN + (FC_MAX - FC_MIN) * ufc / 2048
```

where _ufc_ is the 11-bits frequency value set by the user. Unfortunately,
this formula is completely wrong. The relationship between the user defined
values and the actual filter frequency is non-linear and there is no SID
chip out there that sounds like another.

The author of the [reSID](https://en.wikipedia.org/wiki/ReSID) library did
some reverse engineering work and came out with a lookup table used to
to translate from the user defined values to the actual filter frequencies.

In cemu's implementation, this curve is approximated using a
[sigmoid function](https://en.wikipedia.org/wiki/Sigmoid_function).

The plot below shows the lookup table used by the reSID library, the
approximated function used by cemu, and the formula described in the SID's
technical specifications.

![](filter.png)

The resonance value is also translated to a quadratic response Q factor,
again, the author of the reSID library found that its value varies from
0.707 to 1.7.
cemu's implementation uses the following formula to calculate the actual
resonance of the quadratic response:

```
  Q = 0.707 + (1.7 - 0.707) * res / 15

```
where _res_ is the user specified 4-bits resonance value.

### other bugs:

The volume "click" bug and others are implemented so programs like
[SAM](https://en.wikipedia.org/wiki/Software_Automatic_Mouth) work.

