(define (problem grounded-strips-os-sequencedstrips-small-4_GEN)
(:domain grounded-strips-openstacks-sequencedstrips_GEN )
(:init ( not-made-p1) ( not-made-p2) ( not-made-p3) ( not-made-p4) ( not-made-p5) ( waiting-o5) ( waiting-o4) ( waiting-o3) ( waiting-o2) ( waiting-o1) ( stacks-avail-n0) ( machine-available-))
(:goal (and ( shipped-o5) ( shipped-o4) ( shipped-o3) ( shipped-o2) ( shipped-o1) ( preference p_r0   ( stacks-avail-n1) ) ( preference p_r1   ( waiting-o1) ) ( preference p_r2   ( started-o1) ) ( preference p_r3  ( not(waiting-o2)) ) ( preference p_r4   ( made-p5) ) ( preference p_r5  ( not(not-made-p5)) ) ( preference p_r6   ( shipped-o5) ) ( preference p_r7  ( not(shipped-o4)) ) ( preference p_r8   ( waiting-o5) ) ( preference p_r9  ( not(r9)) ) ( preference p_r10  ( not(r10)) ) ( preference p_r11  ( not(r11)) ) ( preference p_r12   ( r12) ) ( preference p_r13   ( r13) ) ( preference p_r14  ( not(r14)) ) ( preference p_r15  ( not(r15)) )))
(:metric minimize (+
(* (is-violated p_r0) 1)
(* (is-violated p_r1) 5)
(* (is-violated p_r2) 3)
(* (is-violated p_r3) 3)
(* (is-violated p_r4) 4)
(* (is-violated p_r5) 2)
(* (is-violated p_r6) 3)
(* (is-violated p_r7) 5)
(* (is-violated p_r8) 4)
(* (is-violated p_r9) 5)
(* (is-violated p_r10) 3)
(* (is-violated p_r11) 3)
(* (is-violated p_r12) 5)
(* (is-violated p_r13) 1)
(* (is-violated p_r14) 4)
(* (is-violated p_r15) 3)
))
)
