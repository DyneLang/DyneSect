# DyneSect

Dissect, understand, and eventually modify the Apple Newton ROM.

## What Is This?

NewtonOS 2.1 — the operating system of the Apple MessagePad MP2x00 — is a
fascinating piece of software from the early 1990s. It runs on a 32-bit ARM
processor and lives entirely in an 8 MB ROM, written in a mix of ARM assembly,
C++, and NewtonScript.

DyneSect is a toolkit for taking that ROM apart and putting it back together.
It reads the AIF and REx image files distributed with the Newton DDK (the
original C++ developer kit), and works toward a fully symbolic disassembly
that can be reassembled into a bit-identical ROM.

This matters to emulator maintainers who need to understand obscure OS behavior,
fix bugs, and keep aging Newton hardware alive.

## The Pipeline

The foundation is a round-trip identity test:

1. Read the AIF and REx files and combine them into a single address space.
2. Emit an assembler-readable hex dump — every byte accounted for, nothing
   interpreted yet.
3. Feed that through the original 1995 Norcroft assembler and linker.
4. Diff the output against the original ROM. It must be identical.

Once the pipeline produces a verified identity ROM, the real work begins.

## Iterative Dissection

With a working round-trip in place, we can start replacing raw hex with
meaning, one section at a time:

**Assembly recovery** — Static analysis identifies function boundaries and
control flow. Hex blobs become labeled ARM assembly. The AIF file ships with
symbol information; the DDK C++ headers add types, class layouts, and vtable
shapes. Each piece of recovered information feeds back into the disassembler.

**C++ reconstruction** — Where we have enough type information, functions can
be expressed as C++ source with `__asm` blocks. These get compiled with the
original Norcroft C++ compiler and linked back in. If the output is still
identical, the reconstruction is correct.

**NewtonScript** — The ROM contains a significant amount of NewtonScript
bytecode. Using the Newton Framework, we disassemble it into source form and
verify by recompiling it back to identical byte code.

**Assets** — ASCII and UTF-16 strings, images, and sounds are extracted,
catalogued, and made patchable.

**Ghidra integration** — All recovered type and label information is stored in
a JSON file that can be imported into a Ghidra project, so the analysis
accumulates rather than starting over each time.

## Goal

When every byte of the ROM has a symbolic home — a label, a type, a source
file — we can start making changes. Adding features, fixing bugs, patching
behavior. Even if the code shifts in size, the toolchain understands the
structure well enough to link everything back into a fully functional ROM.

A 30-year-old operating system, fully understood and modifiable from source.
