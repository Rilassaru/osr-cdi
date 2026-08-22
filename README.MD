# OSR-CDI

Open-source programmable CDI (Capacitor Discharge Ignition) unit design and source repository for single-cylinder 2-stroke / 4-stroke motorcycles.

This repository includes complete design files for KiCad PCB layouts, PIC microcontroller firmware (MPLAB X), Windows-based map configuration utility "WinCDI" (Visual Studio), and baseline ignition map datasets.

---

## Directory Structure

```text
OSR-CDI/
|-- circuit/             # KiCad schematics, PCB layout, and manufacturing files (JLCPCB)
|   |-- KiCad_pcba/      # Main PCB board design and custom footprint library
|   `-- ...              # Variant circuits and legacy hardware designs
|-- firmware/            # PIC MCU firmware source code (MPLAB X IDE)
|-- wincdi/              # WinCDI map editing software source code (Visual Studio)
|   `-- lib/             # Pre-built static libraries and dependencies (FLTK, HIDAPI)
|-- genuine-map-files/   # Baseline and custom ignition curve maps (.bin / .hex / .csv)
`-- README.md

===========================================
Requirements & Toolchain
===========================================
   Circuit Design
      Tool: KiCad
      Notes: Custom footprints included in OSR-CDI_footprints.pretty

   Firmware
      Tool: MPLAB X IDE & XC8 Compiler
      Notes: Targets PIC16F1455 series microcontrollers

   Flashing / Debugging
      Tool: PICkit 3 / 4, MPLAB SNAP, MPLAB IPE
      Notes: Via on-board ICSP header pins

   GUI Utility (WinCDI)
      Tool: Visual Studio (C++ Desktop Development)
      Notes: Statically links against pre-built libraries in wincdi/lib/

===========================================
Getting Started
===========================================
1. Hardware Fabrication
   Ordering PCBs (e.g., JLCPCB):
      Production-ready Gerber ZIP archives, BOM (Bill of Materials) CSV files, and CPL placement CSV files are located in the circuit/KiCad_pcba/jlcpcb/ directory.

   Editing Schematics / Layout:
      Open the KiCad project files inside circuit/KiCad_pcba/. All custom footprints are bundled using relative paths.

2. Firmware Build & Flash
   1.Open the project under firmware/ using MPLAB X IDE.
   2.Select your target device (e.g., PIC16F1455) and XC8 compiler version, then run Clean and Build.
   3.Connect your programmer (PICkit / SNAP) to the ICSP header on the board and click Make and Program Device.

3. Building WinCDI (Windows Utility)
   1.Open the solution file (.sln) inside the wincdi/ directory with Visual Studio.
   2.Set configuration to Release (or Debug) and run Build Solution.
      Note: Required third-party libraries (FLTK, HIDAPI) are pre-configured in wincdi/lib/ for static linkage.

===========================================
Releases & Pre-built Packages
===========================================
If you only need ready-to-use binaries (WinCDI installers, compiled .hex firmware, and Gerber fabrication files), navigate to the [Releases] section on the right side of this repository.

===========================================
Disclaimer
===========================================
   Use this project, firmware, circuit schematics, and ignition map data entirely at your own risk.

   The authors, developers, and contributors assume no liability for any engine failure, vehicle damage, physical injury, electrical hazard, or any other damages resulting from the use of these materials.

   CDI circuits handle lethal high voltages and incorrect ignition timing can easily cause severe engine damage. Handle with care.

===========================================
License
===========================================
Project License
The core OSR-CDI design, firmware, and WinCDI utility code are released under the BSD 3-Clause License:

Copyright (c) OSR-CDI Project Contributors.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE AND HARDWARE DESIGNS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================
Third-Party Libraries & Notices
===========================================
The wincdi/lib/ directory contains external binary dependencies used for building WinCDI. All rights belong to their respective copyright holders:

   FLTK (Fast Light Toolkit)
      License: GNU Library General Public License (LGPL) v2.0 with the FLTK Library Exception
      Website: https://www.fltk.org/
      The FLTK Library Exception explicitly permits static linking with applications without requiring the host application source code to be licensed under the LGPL.

   HIDAPI
      License: Multi-licensed (BSD-3-Clause / GPL v3 / HIDAPI License)
      Website: https://github.com/libusb/hidapi
   
   libpng / zlib / libjpeg
      License: zlib/libpng License / Independent JPEG Group License
      Included as static dependencies bundled with FLTK image routines.
   
   Microsoft Visual C++ 2010 Redistributable Component (msvcr100.dll)
      License: Microsoft Software License Terms
