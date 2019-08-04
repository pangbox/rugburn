# rugburn :fire:

_This project is not endorsed by or related to Ntreev Soft Corporation. See [Trademark Notice](#trademark-notice) for more information._

`rugburn` is an unobtrusive and small shim for `ijl15.dll` that allows you to
run unmodified PangYa™ without GameGuard.

This also allows you to run PangYa under Wine :)

To get started, grab a binary from the [Releases](https://github.com/pangbox/rugburn/releases) page and follow the [installation instructions](#installation).

Features:

  * Redirects network traffic to localhost. (You can modify this to redirect
    wherever you want; see [src/hooks/ws2_32/redir.c](./src/hooks/ws2_32/redir.c)
    and [src/hooks/wininet/netredir.c](./src/hooks/wininet/netredir.c).)

  * Patches GameGuard's check routines.

  * Prevents PangYa from creating annoying topmost windows.

  * Sets PANGYA_ARG when possible to avoid the updater check.

  * Implements the IJL15 API and forwards it to `ijl15_real.dll`,
    no need for tricky patching.

  * **This program is not suitable for cheating. It does not support the only
    active region of PangYa and does not offer GameGuard emulation that would
    be needed to stay connected to an official server.** This program is
    designed for unofficial servers and educational use!

## Compiling

`rugburn` is compiled with OpenWatcom, a simple compiler with few dependencies.
It was chosen because it offered an easy path to cross-compilation while also
allowing for tiny binaries that did not depend on libc.

You can modify the Make variables `WATCOM`, `WCC`, and `WLINK` to match your
environment, then run `make`. An `ijl15.dll` should appear in the `out` folder.

## Installation

 1. Move the original `ijl15.dll` binary in the PangYa folder to `ijl15_real.dll`.
    The exact name is important.

 2. Copy the rugburn `ijl15.dll` into the PangYa folder.

Do not use update.exe anymore. Just run ProjectG directly. The update servers
are still active in US, so if you run the updater you may accidentally patch
over your files.

## Usage

Once installed, you can run ProjectG directly. Enjoy!

## Troubleshooting

If you have any issues, I can **not** guarantee that I can help you. However, please feel free to create a GitHub issue. Please describe your problem and if applicable, attach a copy of the `ijllog.txt` file.

## Contributing

I would be overjoyed if anyone wanted to contribute to this project! However, the project is considered _nearly_ feature complete and therefore new features may not always be accepted. Well-tested, well-written improvements to the patching routines would definitely be welcome.

Please note that I may take a while to get to your pull request. This project is not my fulltime job. Sorry!

## Why the name?
I went through a lot of names during development. I wanted something catchy but not too lame. The initial codename for the project was 'ggtfo', a portmanteau of 'gg' for 'GameGuard' and 'gtfo' for... yeah. This was mostly out of anger, and seemed a bit too edgy.

Another codename it had was 'ijlshim' reflecting the nature of how it works. However, this seemed way too boring. As far as literal names go, it also did not really describe _what_ the software did.

Inspiration came for this name when trying to find anagrams for the word 'GameGuard' and the phrase 'Damage Rug' showed up. 'Damage Rug' was pretty amusing, but it isn't exactly catchy, so I decided to drop the anagram idea and just go with the virtually non-sense name 'rugburn.'

So as is often the case with open source projects, the name is meaningless.

## License
Most of the code of rugburn is licensed under the ISC license. Some portions are licensed differently. See [LICENSE.md](./LICENSE.md) for complete licensing information.

## Special Thanks
Special thanks to the [PangyaTools](https://github.com/pangyatools) community for motivation and advice.

## Trademark Notice
PangYa is a registered trademark of Ntreev Soft Co., Ltd. Corporation. Pangbox is not endorsed or related to Ntreev Soft Co., Ltd. Corporation in any way. "PangYa" and related trademarks are used strictly for purposes of identification.