; Z80 test - officially documented flags version.
;
; Copyright (C) 2012-2022 Patrik Rak (patrik@raxoft.cz)
;
; This source code is released under the MIT license, see included license.txt.

            macro       testname
            db          "doc"
            endm

maskflags   equ         1
onlyflags   equ         0
postccf     equ         0
memptr      equ         0

            include     main.asm

; EOF ;
