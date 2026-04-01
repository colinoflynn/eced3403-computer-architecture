# Minimal STM32F3 Nucleo Rust Blinky

Very simple rust blinky program for STM32F303RE Nucelo Boards.

### Installing Toolchain

Go to [rustup.rs](rustup.rs) to confirm directions, but the basic idea is:

```sh
# Install rustup — the Rust toolchain manager
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Verify
rustc --version && cargo --version

```

Add targets
```
# Cortex-M4 / M7 with & w/o hardware float (e.g. STM32F3/F4)
rustup target add thumbv7em-none-eabihf thumbv7em-none-eabi

# Cortex-M0 / M0+ (many simpler MCUs)
rustup target add thumbv6m-none-eabi
```

Install additional tools (may or may not be needed)
```
# probe-rs-tools gives us cargo-embed and a bunch of stuff
cargo install probe-rs-tools
```

### Building Target
run:

```sh
cargo build --release
```

This should generate a ELF file at `target\thumbv7em-none-eabihf\release\blinky`.

You can program this file using Ozone (if Nucleo is converter to J-Link OB).

If you want to make a hex file you can run e.g.:

```sh
arm-none-eabi-objcopy -O ihex target/thumbv7em-none-eabihf/release/blinky blinky.hex
```

To clean the code run:

```sh
cargo clean
```