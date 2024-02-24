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

  * Patches GameGuard's check routines to allow it to run indefinitely without
    GameGuard.

  * Prevents PangYa from creating annoying topmost windows.

  * Sets `PANGYA_ARG` when possible to avoid the updater check.

  * Can be used as a relay for `ijl15.dll` or as a very simple patch for it.
    No need to modify or remove protection on `ProjectG.exe`.

  * **This program is not suitable for cheating. It does not support the only
    active region of PangYa and does not offer GameGuard emulation that would
    be needed to stay connected to an official server.** This program is
    designed for personal and educational use.

## Compiling
`rugburn` is compiled with OpenWatcom, a simple compiler with few dependencies.
It was chosen because it offered an easy path to cross-compilation while also
allowing for tiny binaries that did not depend on libc.

Prerequisites:
  * OpenWatcom V2: https://github.com/open-watcom/open-watcom-v2/releases
  * GNU Make
  * Go (optional: needed for Slipstream)

And all you have to do is run `make`.
```sh
make
```

And you should find an `rugburn.dll` in your `out/` directory.

By default, the makefile will assume OpenWatcom is installed into `/usr/bin/watcom`. You can override this by passing in the `WATCOM` variable:

```
make WATCOM=$HOME/Programs/watcom
```
## Install

 1. Move the original `ijl15.dll` binary in the PangYa folder to `ijl15_real.dll`.
    The exact name is important.

 2. Copy the rugburn `rugburn.dll` into the PangYa folder and rename it to `ijl15.dll`.

Do not use update.exe anymore. Just run ProjectG directly. (The update servers
are still active in US, so if you run the updater you may accidentally patch
over your files.)

## Using Slipstream
You can use the web patcher if just want to patch your `ijl15.dll` with the latest version of Rugburn. It does not require an active internet connection.

To use Slipstream manually, you can use the `make slipstream` command. Note that it requires the Go toolchain. You should find a patched `ijl15.dll` in your `out/` directory.

## Usage
Once installed, you can run ProjectG directly. Enjoy!

## Troubleshooting
If you have any issues, I can **not** guarantee that I can help you. However, please feel free to create a GitHub issue. Please describe your problem and if applicable, attach a copy of the `ijllog.txt` file.

## Contributing
I would be overjoyed if anyone wanted to contribute to this project! However, the project is considered _nearly_ feature complete and therefore new features may not always be accepted. Well-tested, well-written improvements to the patching routines would definitely be welcome.

Please note that I may take a while to get to your pull request. This project is not my fulltime job. Sorry!

## Why the name?
"Rugburn" was chosen arbitrarily, inspired by an anagram of GameGuard. Before release it was known as ijlshim or ggtfo, and you may see occasional references to this in the code or Git history.

## License
Most of the code of rugburn is licensed under the ISC license. Some portions are licensed differently. See [LICENSE.md](./LICENSE.md) for complete licensing information.

## Special Thanks
Special thanks to the [PangyaTools](https://github.com/pangyatools) community for motivation and advice.

## Trademark Notice
PangYa is a registered trademark of Ntreev Soft Co., Ltd. Corporation. Pangbox is not endorsed or related to Ntreev Soft Co., Ltd. Corporation in any way. "PangYa" and related trademarks are used strictly for purposes of identification.
