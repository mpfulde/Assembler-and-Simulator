start:  not r0 r1
        and r0 r1 r0
        not r1 r0
        add r1 r1 r1	
        bnz end
        not r1 r1
end:    and r3 r1 r1
        bnz end