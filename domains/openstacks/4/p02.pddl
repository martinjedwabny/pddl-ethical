(define (problem grounded-strips-os-sequencedstrips-small-5_GEN)
(:domain grounded-strips-openstacks-sequencedstrips_GEN )
(:init ( not-made-p1) ( not-made-p2) ( not-made-p3) ( not-made-p4) ( not-made-p5) ( waiting-o5) ( waiting-o4) ( waiting-o3) ( waiting-o2) ( waiting-o1) ( stacks-avail-n0) ( machine-available-))
(:goal (and ( shipped-o5) ( shipped-o4) ( shipped-o3) ( shipped-o2) ( shipped-o1) ( preference p_r0   ( stacks-avail-n1) ) ( preference p_r1   ( waiting-o1) ) ( preference p_r2   ( started-o1) ) ( preference p_r3  ( not(r3)) )))
(:metric minimize (+
(* (is-violated p_r0) 1)
(* (is-violated p_r1) 5)
(* (is-violated p_r2) 3)
(* (is-violated p_r3) 3)
))
)
