# blip
Small C library for sound synthesis of MML ([`Music Macro Language`](https://en.wikipedia.org/wiki/Music_Macro_Language)) strings. MML is a simple language for describing music, often used in old video games.

`blip` implements a subset of MML and is good enough to play simple tunes. It is designed to be small and portable, so it can be used in embedded systems. There are no external dependencies, dynamic allocations or floating point operations.

However, it is not an audio library. It does not provide any means of playing the generated sound. You only get a stream of samples. It is up to you to play them using whatever audio API you have available.

## MML syntax
Following is a list of supported MML commands. All commands are case-insensitive, whitespace is ignored.

`cdefgab` - notes C, D, E, F, G, A, B. Appending `+` or `#` makes the note sharp, appending `-` makes it flat. Note duration can be specified by a number (1-255) following the note. The number represents a fraction of a whole note. For example, `c4` is a quarter note, `c8` is an eighth note, `c16` is a sixteenth note, etc. Note commanfa `h` is interpreted as `b`.

`o` - octave, followed by a number (0-7). Sets the default octave for all upcoming notes.

`p` or `r` - pause or rest. Pauses are treated as notes with zero frequency.

`l` - length, followed by a number (1-255). Sets the default note length for all upcoming notes.

`t` - tempo, followed by a number (32-255). Sets the tempo in beats per minute. Default is 120.

`v` - volume, followed by a number (0-15). Sets the volume for all upcoming notes. Default is 15.

`<` or `>` - step down or step up the octave. Changes the default octave for all upcoming notes.

`&` - tie. Makes the previous note last longer. For example, `c8 & c8` plays a single quarter note.

`.` - extend length by half of the previous length. For example, `c8.` is same length as `c8 & c16`. If used multiple times, `c8...` is same as `c8 & c16 & c32 & c64`.

## Examples

`George Michael – Careless Whisper` [**MP3**](https://github.com/P-i-N/blip/blob/main/examples/GeorgeMichael_CarelessWhisper.mp3?raw=true)
```
t100l32aa+>cdl8ed16<af>e.d16<af.b+a+16fdb+.a+16f3a+a16fd<a+2aa+>cdefga
```

`Für Elise` [**MP3**](https://github.com/P-i-N/blip/blob/main/examples/FurElise.mp3?raw=true)
```
t100l16>ed+ed+ec-dc<a8rceab8reg+bb+8re>ed+ed+ec-dc<a8rceab8rdb+ba8rb>cde8.<g>fed8.<f>edc8.<e>dc<br8r>er8r>er8<d+er8d+ed+ed+ec-dc<a8rceab8reg+bb+8re>ed+ed+ec-dc<a8rceab8rdb+ba8,
r2l16<<a>ear8&r16<e>eg+r8&r16<a>ear2r<a>ear8&r16<e>eg+r8&r16<a>ear8&r16cgb+r8&r16<g>gbr8&r16<a>ear8&r16<e>e>eere>eerd+er8d+er2r<<<a>ear8&r16<e>eg+r8&r16<a>ear2r<a>ear8&r16<e>eg+r8&r16<a8
```

`Mungo Jerry – In The Summertime` [**MP3**](https://github.com/P-i-N/blip/blob/main/examples/MungoJerry_InTheSummertime.mp3?raw=true)
```
t110l16b>c+e8.ee8c-c+c+<bgg+e8b>c+e8.ee8c-c+c+<bgg+e8ef+f+8.e8f+ec+f+ec+f+ec+b>c+e8.ee8c-c+c+<bgg+e8f+g+b8.bb8f+g+f+ec+f+ec+ff+e8.ee8c+c-c+<bgg+e8,r8<e8r8<b8r8>e8r8<b16r16>c+16r16e8r8<b8r8>e8r8e16r16f+16r16a8r8e8r8<a8r8a16r16>c+16r16e8r8<b8r8>e8r8<b16r16>e16r16<b8r8>f+8r8<a8r8>e16r16<b16r16>e8r8<b8r8>e8<b8e8
```

`Game Of Thrones` [**MP3**](https://github.com/P-i-N/blip/blob/main/examples/GameOfThrones.mp3?raw=true)
```
v120o5r1r1r1r4r8l32cdd+fg4&g16.rc4&c16.rd+16f16g8.&grc8.&crl16d+fd2&d8.r2r4rf4&f.r32<a+4&a+.r32>d+df8.&f32r32<a+4&a+.r32>d+dc2&cr2r8r16l32cdd+fg4&g16.rc4&c16.rd+16f16g8.&grc8.&crl16d+fd2&d8.r2r4rf4&f.r32<a+4&a+.r32>d+df8.&f32r32<a+4&a+.r32>d+dc4.&cr2r4r8r16>d+4&d+.r32<g4&g.r32>cdd+8.l32&d+r<g8.&grl16>cd<a+4.&a+r8r16l32dd+dcd8g8&grl16aa+8>c8d8d4&d.r32<f4&f.r32b+a+>d8.&d32r32<f4&f.r32b+a+g+2&g+r2r4r>d+4&d+.r32<g4&g.r32>cdd+8.l32&d+r<g8.&grl16>cd<a+4.&a+r8r16a8g4.&gr8r16a+>d+d4&d.r32<f4&f.l32ra+8.&a+rb+8a+8.&a+rf8g4.&l16gr1rb+8d+8g+a+b+8d+8g+b+a+8d+8gg+a+d+8g8a+g+8c8fgg+8c8fg+g8>c8d+fg8c8d+f<g+8.&g+32r32>cdd+8<g+8>dd+d+4&d+.r32f4&f.r32g4.&gr1rc8<d+8g+a+b+8d+8g+b+a+8d+8gg+a+8d+8ga+g+8c8fgg+8c8fg+g8>c8d+fg8c8d+f<g+8.&g+32r32>cdd+8<g+8>dd+d+8.&d+32r16r32ff8f8&f32r32fc8<<g8g+a+b+8g8g+a+b+8g8g+a+1&a+8r>>b+8g8g+a+b+8g8g+a+b+8g8g+a+1&a+8,
v127o5r2l8<g<cl16>efg8c8efc8<c8>efg8c8efc8<c8>efg8c8efc8<c8>efg8c8efc8<c8>d+fg8c8d+fc8<c8>d+fg8c8d+f<g8<g8>a+>cd8<g8a+b+g8<g8>a+>cd8<g8a+b+a+8<a+8>>dd+f8<a+8>dd+<a+8<a+8>>dd+f8<a+8>dd+<f8<f8>g+a+b+8f8g+a+b+8<f8>g+a+b+8f8g+a+b+8c8>d+fg8c8d+fc8<c8>d+fg8c8d+f<g8<g8>a+>cd8<g8a+b+g8<g8>a+>cd8<g8a+b+a+8<a+8>>dd+f8<a+8>dd+<a+8<a+8>>dd+f8<a+8>dd+c8<<f8>g+a+b+8f8g+a+b+8<f8>g+a+b+8f8g+a+b+8c8>d+fg8c8d+fc8<c8>d+fg8c8d+f<g8<g8>a+>cd8<g8a+b+g8<g8>a+>cd8<g8a+b+a+8<a+8>>dd+f8<a+8>dd+<a+8<a+8>>dd+f8<a+8>dd+c8<<f8>g+a+b+8f8g+a+b+8<f8>g+a+b+8f8g+a+b+8c8>d+fg8c8d+fc8<c8>d+fg8c8d+f<g8<g8>a+>cd8<g8a+b+g8<g8>a+>cd8<g8a+b+a+8<a+8>>dd+f8<a+8>dd+<a+8<a+8>>dd+f8<a+8>dd+c8<<f8>g+a+b+8f8g+a+b+8<f8>g+a+b+8f8g+a+<g+4&g+.l32r>>d+8.&d+r<<g+8d+4&d+16.r>a+8.&a+r<d+8f4&f16.r>>c8.&cr<<f8c4&c16.r>>g8.&gr<<c8g+4&g+16.r>>d+8.&d+r<<g+8l16>d+a+>d+fga+<f>cfgg+b+c8<c8>d+fg8c8d+fc8<c8>d+fg8c8d+f<<g+4&g+.l32r>>d+8.&d+r<<g+8d+4&d+16.r>a+8.&a+r<d+8f4&f16.r>>c8.&cr<<f8c4&c16.r>>g8.&gr<<c8g+4&g+16.r>>d+8.&d+r<<g+8g+8&g+r>>d+8&d+r<<g8&gr>>d8&drl8<<c>gl16>d+fg8c8d+fg8c8d+f1&f8r<<c1
```

`The Simpsons` [**MP3**](https://github.com/P-i-N/blip/blob/main/examples/TheSimpsons.mp3?raw=true)
```
t167l16g1&g2<e2f+2g1&g2r8>>crcr4rcrcrcr8.crcr8.cr8.cr4rcrcr8.<c4.e4f+4a8g4.e4c4<a8>f+8f+8f+8g4.r4.c8c8c8c8c4.d+2r8<a8a8a8r8>d+8d+8d+8d+8d+4d+8<ba+b>c+d+c+d+ff+ff+g+ag+ab>fd+c+<b>d+c+<ba>c+<bagbagf<b4.>d+4f4>d+8c+4.<b4a4f8<a8a8a8b8r8a8a8a8b>d+fabagd+agfc+gfd+<bg+2b1>c+4e4d+2<b4g+4e1>d8f+g+a+8g+f+e8f+g+a+8g+f+erg+8crg+8erg+8<ar>g+8c4.e4f+4a8g4.e4e4e8c8c8c8c4b8e8f+8g8f+8e8<b8>c4.e4f+4a8g4.e4c4>c8d+8c8d8c8r8<c+r8.c+rc+8c+r8.c+rd+8c+rg8c+ra8c+r>>c8<<c+r<b4.>d+4f4g+8f+4.d+4<b4g+8b8b8b8b4>c+4<b8b8>d8d+4f8f+8b8a1&a8r8a8b8b8>e8b8a8g8a8g8f+8e8a8g8f+8e8d8e8f+8d8e8.d+8e2&e8.<<a+8a+8r8a+8r8a+8r8a+8>d+2a+1>c4d+4<a+2g4d+4c+8c+8c+8c+8r8c+8c+8r8d+8fga8gfd+8fga8r8<d+8fga8gfd+8fga8r4>c+8r8c+8r8c+8r8c+8d+dd+fgfgaa+>cc+d+gfd+c<c+4.f4g4a+8g+4.f4c+4g8c+8c+8c+8c+8r8c+8c+8c+8r8c+8c+8c+8c+8g+4.c+8c+8c+8c+8,
>c1&c2<<g2b2>c1&c2r8>erer4rererer8.erer8.er8.er4rerer8.c4.e4f+4a8g4.e4c4<a8r1r8e8e8e8e8e4.f+2r8<b8b8b8r8>f8f8f8f8f4f8r1r1r1r1c+8c+8c+8d+8r8<b8b8b8r1b2>c+1e4g+4f+2d+4<b4g+1>e8r2.r8>dr8.<br8.>dr8.<gr8.g4.>c4c4f+8c4.c4<b4a8d+8d+8d+8e4r1r1r1r2d+r8.d+rd+8d+r8.d+r>d+8<d+r>g8<d+r>a8<d+r8.d+rb4.>d+4f4g+8f+4.d+4<b4g+8d8d8d8d+4r1r2r8>b8a2&a8r8a8b8b8g8r1r2.r8f+8.e8f+8.r2<e8e8r8e8r8e8r8e8g2>c+1d+4f4d+2<a+4g4d+8d+8d+8d+8r8d+8d+8r1r1r4d+8r8d+8r8d+8r8d+8r1g+4.>c+4c+4g8c+4.c+4<g+4a+8f8f8f8f8r8f8f8f8r8f8f8f8f8b4.f8f8f8f8,
>e1&e2<c2d+2e1&e2r8>f+rf+r4rf+rf+rf+r8.f+rf+r8.f+r8.f+r4rf+rf+r1r1r1r4r<f+8f+8f+8g8g4.b2r8d+8d+8d+8r8f+8f+8f+8f+8f+4f+8r1r1r1r1r2r8d+8d+8d+8r1e2e1f+4a+4g+2e4e4<a+1r1>>er8.cr8.er8.<ar8.>c4.e4f+4a8f+4.e4c4r8<f+8f+8f+8g4r1r1r1r2gr8.grg8gr8.gr8.gr8.gr8.gr8.gr1r1rf8f8f8f+4r1r1r2.r8>>c8r1r2.r8<g8.f+8g2&g8.<g8g8r8g8r8g8r8g8a+2>d+1f4a4g2d+4<a+4g8g8g8g8r8g8g8r1r1r4g8r8g8r8g8r8g8r1>c+4.f4g4a+8g+4.f4c+4r8<g8g8g8g+8r8g8g8g+8r8g8g8g8g+8r4.g8g8g8g+8,
r1r1f+2g1&g2r1r1r1r1r1r1r8a+4.r2r8f+8f+8f+8r1r1r1r1r1r2r8f+8f+8f+8r1r2g+1a+4>c+4<b2g+4r1r1r1r1r4>g4.r1r1r1r1r2.<ar8.ara8ar8.ar8.ar8.ar8.ar8.ar1r1r2ra8r1r1r1r1r1r1r2.r8>d+2g1a4>c4<a+2g4d+4<a8a8a8a8r8a8a8r1r1r4a8r8a8r8a8r8a8r1r1r1r1r1>c+8c+8c+8c+8,
r1<c1&c1&c1c8.r2.rc8.r2.rc8.r4.rf+rf+r4rf+rf+rf+r8.f+rf+r8.f+r8.f+r4rf+rf+r4rf+rf+rf+r1r<b4.r2r8b4.r2r8b4.r2r8b4.r2.>frfr4rfrfrfr8.frfr8.fr8.fr1r1r1r1r1r1r1r1r4rf+rf+r4rf+rf+rf+r8.f+rf+r8.f+r8.f+r1r1r1r1r8.d+r4.rd+r4.rd+r4.rd+r2rfrfr4rfrfrfr8.frfr8.fr8.fr1r1r1r8.<b4r4>c8r4.c8r4.c8r4.c8r4.c8r4.c8r4.c8r4.c8r1r1r1r1r1r1r1r4.d+4<g4a4>c4r8grgr4rgrgrgr8.grgr8.gr8.gr8.c+4.r2r8c+4.r2r8c+8c+8c+8c+8,
r1o3c1&c1&c1c8.r8.f+r2rc8.r8.f+r8.f+r8.f+rc8.r8.f+r>c8erer<f+8r8>erererc8erer<f+8>er<f+8>er<f+8>c8erer<f+8r8>erererc8r4<f+8r8f+8r8f+8<b4.>f8r2<b4.>f8r8f8r8f8<b4.>f8r2<b4.>f8r8f8r8f8b8>d+rd+r<f8r8>d+rd+rd+r<b8>d+rd+r<f8>d+r<f8>d+r<f8b4.f8r2b4.f8r8f8r8f8erbrer<a+r>erbrer<a+r>erbrer<a+r>erbrer<a+r>erbrer<a+r>erbrer<a+r>erbrer<a+r>erbrer<a+r>er8.>g+rf+edref+g+rf+e<er8.a+r8.er8.a+r8.>c8erer<f+8r8>erererc8erer<f+8>er<f+8>er<f+8>f+8f+8f+8g2&g8<g2r1r1r2d+r8.ar8.d+r8.ar8.d+r8.ar8.d+r8.ar8.b8>d+rd+r<f8r8>d+rd+rd+r<b8>d+rd+r<f8>d+r<f8>d+r<f8b4.f8r2b4.f8r8f8r8f8b4.f8r4f4<b4r8>l8fcrgrcrgrcrgrcrgrcrgrcrgrcrgrcrgrl16d+ra+rd+r<ar>d+ra+rd+r<ar>d+ra+rd+r<ar>d+ra+rd+r<ar>d+ra+rd+r<ar>d+ra+rd+r<ar>d+r8.<ar8.>d+r8.<ar8.>d+r8.<ar8.>d+r8.<ar8.>d+r8.<ar8.>d+r8.<ar8.>d+r8.<ar8.>d+r8.<ar8.>d+4<g4a4>c4>c+8frfr<g8r8>frfrfrc+8frfr<g8>fr<g8>fr<g8c+4.gr2rc+4.gr8.gr4rc+8c+8c+8c+8,
l1rrrrrrrrrrrrrrl16r2r8<arar4rararar8.arar8.ar8.al1rrrrrrrrrrrrrrrrl16r4rf+rf+r4rf+rf+rf+r8.f+rf+r8.f+r8.f+
```

## License
Unlicense / public domain / I do't care. Do whatever you want with it.

https://unlicense.org

## Misc.

### Frequency table
`blip` uses 5.27 fixed-point frequency table of the 0th octave (notes C0 to B0) to calculate frequencies of all notes. Here is the Python script used to generate it:

```python
import math

freqs = []
for i in range(0, 12):
    f = 440 * math.pow(math.pow(2, 1.0/12.0), i - 57)
    freqs.append(int(f * (1 << 27) + 0.5 / 27))

print(", ".join("0x{:08x}".format(f) for f in freqs))
```
