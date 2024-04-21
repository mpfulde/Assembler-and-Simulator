; Fibonacci sequence going on forever
;
start:  not r0 r1
        and r0 r0 r1    ; ensures r0 contains 0
        not r1 r0       ; r1 contains 255
        add r1 r1 r1    ; r1 contains 254
        not r1 r1       ; not 254 = 1
out:    add r3 r0 r0    ; outputs 0
        add r3 r1 r0    ; outputs 1
        add r2 r0 r0    ; sets r2 to 0
loop:   add r2 r2 r1    ; increment by result
        add r3 r0 r2    ; output the result
        add r1 r2 r1    ; gets next number
        add r3 r1 r0    ; output the result
        bnz loop        ; repeat until r0=256=0


; largest valid number is 233 or E9 in hex
; first invalid number is 121 or 79 in hex
; this number is invalid due to an overflow in the resulting register as result
; of the arithmetic operation