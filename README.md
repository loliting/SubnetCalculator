# Subnet Calculator

## Overview
Subnet Calculator is a simple calculator of IPv4 and IPv6 subnetworks based on [libSubnetCalculator](https://github.com/loliting/libSubnetCalculator)

This project is licensed under the terms of the GPLv2 or any later version. See the LICENSE file for the full text of the license.

## Instalation

### Compiling from source
Only dependency of Subnet Calculator is Qt 6 (libSubnetCalculator is bundled as a git submodule).

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Official binary packages
Currently we do not ship any binaries.