# Mericlebeef - DeadBeeF sleep timer.

## How to Install

### Step 1

#### Debian/Ubuntu
```sh
sudo apt install deadbeef deadbeef-dev pkg-config
```

### Arch
```sh
sudo pacman -S deadbeef pkgconf
```
Just make sure to install the deadbeef header and pkg-config.

### Step 2

Clone the repo and cd to it.

```sh
git clone https://github.com/mochidaz/mericlebeef
cd mericlebeef
```

### Step 3

Build

```sh
make
```

Or if you can't install the headers via package manager:

```sh
make DEADBEEF_INC=/path/to/deadbeef/include/dir
```

### Step 4

After compiling, a .so file will be generated. Using this repo's Makefile will
install it to `/opt/deadbeef/lib/deadbeef`, which is the only path I've tested.
You can look for where deadbeef stores plugins on your OS by yourself, and just simply copy
the .so file to it.


## Usage

After installing, you will be able to see a menu called "Mericlebeef" right on the menubar.
You can then activate your sleep timer by selecting one of the options. You can also
customize your own duration in minutes via `Preferences>Plugins>Mericlebeef`.