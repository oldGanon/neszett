
![Neszett][logo]

---

# Description

A NES/Famicom emulator with a focus on accuracy and low input latency.
Many ergonomic features you might expect are still missing.

---

# Usage

Dragging a ROM file onto the window load said ROM if it is supported by the emulator otherwise it will silently fail in the background.
Resize the window by dragging the right or lower edge of the window.
Move the window by dragging anywhere else.
Shift+Enter toggles Fullscreen.
R soft resets the console.
Controls mapping can be changed in the _controls.txt_ file.

---

# Mappers

Supported iNES mappers are 0,1,2,3,4,7,24,69

---

# Tests

| Test ROM            | Status              | Notes |
| ------------------- |:-------------------:| ----- |
| apu_mixer           | sounds good         | Should maybe be compared to the output of a real NES. |
| apu_reset           | passes all          | |
| apu_test            | passes all          | |
| cpu_dummy_reads     | passes all          | |
| cpu_interrupts      | passes all          | |
| cpu_reset           | passes all          | |
| instr_test-v3       | passes official     | Unofficial/Undocumented op codes arent implemented yet. |
| instr_timing        | passes all          | |
| ppu_sprite_hit      | passes all          | |
| ppu_sprite_overflow | passes 3/5          | |
| ppu_vbl_nmi         | passes 9/10         | NMI off timing seems to stop NMI up until interrupt instruction fetch. |
| tvpassfail          | passes all          | |
| nestest             | passes all          | |

---

# TODO

- PAL
- open bus behaviour
- unofficial op codes
- graphical user interface

# Videos

<a href="http://www.youtube.com/watch?feature=player_embedded&v=z7aRUu_yGLo
" target="_blank"><img src="https://raw.githubusercontent.com/oldGanon/neszett/master/docs/smb1_title.png" 
alt="Super Mario Bros. in 04:57.31 (TAS) by HappyLee" width="300" height="224" border="10" /></a>

<a href="http://www.youtube.com/watch?feature=player_embedded&v=CWrrgy-6Uc8
" target="_blank"><img src="https://raw.githubusercontent.com/oldGanon/neszett/master/docs/smb3_title.png" 
alt="Super Mario Bros. 3 in 10:24.94 (TAS) by Lord Tom, Maru & Tompa" width="300" height="224" border="10" /></a>

---

[logo]: docs/logo.png "Neszett"
