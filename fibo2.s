; Fibonacci sequence going up to 12
;
start:  not r0 r1
        and r0 r0 r1    ; ensures r0 contains 0
        not r1 r0       ; r1 contains 255
        add r1 r1 r1    ; r1 contains 254
        not r1 r1       ; not 254 = 1
        add r2 r1 r1    ; r2 = 2
        add r2 r2 r1    ; r2 = 3
        add r2 r2 r2    ; r2 = 6
        add r2 r2 r2    ; r2 = 12
        not r2 r2       ; r2 = ~12 (1111 0011)
        add r2 r1 r2    ; r2 = ~11 (1111 0100) so we end on 89 like described
out:    add r3 r0 r0    ; outputs 0
        add r3 r1 r0    ; outputs 1
        add r3 r1 r0    ; outputs 1 again (3rd number)
        add r0 r1 r0    ; set r0 to one
        add r2 r0 r2    ; add one to account for printing init value
        add r2 r0 r2    ; add 2
        add r2 r0 r2    ; add 3
loop:   and r0 r3 r3    ; temporarily store prev val in r0
        add r3 r3 r1    ; displays first value of loop (starts at 3)
        and r1 r0 r0    ; restores prev val to r1
        not r0 r1       ; begin work to reset r0 to 1 r0 is garbage
        and r0 r0 r1    ; gets r0 to be 0, r0 = 0
        not r0 r0       ; r0 = 255
        add r0 r0 r0    ; r0 = 254
        not r0 r0       ; not 254 = 1
        add r2 r0 r2    ; add until r2 = 256 = 0
        bnz loop

; program ends here
; One limitation is the lack of branch flexibility. We only branch when the
last operation in the ALU was not equal to 0, if it were possible to branch if
say last operation was equal to 12, then it would create a simpler to know when
 to exit.
; One instruction that would make this problem easier to solve would be an
increment instruction,
; a lot of what makes this rather difficult is the lack of registers to hold
different values, so having this increment operation would open up another reg
for use
; we would no longer need to rely on reading from r3 right after outputing, the
 temporary storing a value in r0 and then replacing with one, but more
 importantly it would increment the value of r2 until it reaches 256 (or 0
 overflowed) and then not branch (the branching still works)
; While the instruction only needs one argument, (ie the input reg would also
be the destination reg) the option to have 2 opens additional use cases, making
 the initial values easier to handle as well
; inc Rd Rn, ex. inc r0 r2 ; increment the value in r2, and then set it to r0,
 r0 = r2 + 1
; the op code would be nested in the 10 op code, since this is the only other 2
 operand instruction that contains unused bits, those 2 bits can be used in
 order to determine which instruction. As it stands now those 2 bits are set to
  00 for not, then setting them to 01 would mean use the inc op
; 10100100 ; hex: A7, inc r0 r2, format is Opcode1 Rn Opcode2 Rd.
