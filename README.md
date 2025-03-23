# Network_diagnostic_utility

## About Utility

This is a network utility written in C++ that allows you to test the accessibility of intermediate nodes on the network path from the client computer to any selected FQDN(Fully Qualified Domain Name). The utility is written for Linux and uses RAW sockets.

### Prerequisites

- g++ complier
- OpenWRT SDK

### Installation & Building

Installation:

```sh
git clone https://github.com/SEYTER1NOBROSKI/Network_diagnostic-utility.git Network_diagnostic_utility
```

### To build app:

To build a program for Linux, you need to go to the root of the project and run this command:
```sh
make -f MakeFile
```

To build the program for OpenWRT you need:
1. You need to find out what version of OpenWRT you have. To do this in OpenWRT you need to use this command:
```sh
cat /etc/os-release
```

2. After that, you can install the required version of OpenWRT SDK (Replace XX.XX.XX with your OpenWrt version)
```sh
wget https://downloads.openwrt.org/releases/XX.XX.XX/targets/x86/64/openwrt-sdk-XX.XX.XX-x86-64_gcc-XX.X.X_musl.Linux-x86_64.tar.xz
```
Unpack the SDK:
```sh
tar -xf openwrt-sdk-XX.XX.XX-x86-64.tar.xz
cd openwrt-sdk-XX.XX.XX-x86-64
```

3. Before compiling, you should configure the OpenWrt Toolchain
Loading dependencies for compilation:
```sh
./scripts/feeds update -a
./scripts/feeds install -a
```
Adding cross-compilation tools to PATH:
```sh
export STAGING_DIR=$(pwd)/staging_dir
export PATH=$STAGING_DIR/toolchain-x86_64_gcc-XX.XX.XX_musl/bin:$PATH
```

4. Compiling the program for OpenWrt. In the root of the project run this command:
```sh
make -f Makefile
```

After that, we will have a program for OpenWRT that can be downloaded and used in this OS.
