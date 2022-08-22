(define (problem grounded-strips-tpp_GEN)
(:domain grounded-strips-tpp-propositional_GEN )
(:init ( at-truck1-depot1) ( on-sale-goods4-market1-level1) ( on-sale-goods3-market1-level1) ( on-sale-goods2-market1-level1) ( on-sale-goods1-market1-level1) ( loaded-goods4-truck1-level0) ( loaded-goods3-truck1-level0) ( loaded-goods2-truck1-level0) ( loaded-goods1-truck1-level0) ( stored-goods4-level0) ( stored-goods3-level0) ( stored-goods2-level0) ( stored-goods1-level0) ( ready-to-load-goods4-market1-level0) ( ready-to-load-goods3-market1-level0) ( ready-to-load-goods2-market1-level0) ( ready-to-load-goods1-market1-level0))
(:goal (and ( stored-goods4-level1) ( stored-goods3-level1) ( stored-goods2-level1) ( stored-goods1-level1) ( preference p_r0   ( on-sale-goods3-market1-level0) ) ( preference p_r1   ( ready-to-load-goods4-market1-level1) ) ( preference p_r2   ( ready-to-load-goods3-market1-level1) ) ( preference p_r3  ( not(at-truck1-depot1)) ) ( preference p_r4   ( stored-goods4-level1) ) ( preference p_r5  ( not(on-sale-goods4-market1-level1)) ) ( preference p_r6   ( loaded-goods1-truck1-level0) ) ( preference p_r7  ( not(loaded-goods2-truck1-level0)) ) ( preference p_r8   ( ready-to-load-goods2-market1-level0) ) ( preference p_r9  ( not(on-sale-goods2-market1-level1)) ) ( preference p_r10  ( not(r10)) ) ( preference p_r11  ( not(r11)) ) ( preference p_r12   ( r12) ) ( preference p_r13   ( r13) ) ( preference p_r14  ( not(r14)) ) ( preference p_r15  ( not(r15)) ) ( preference p_r16   ( r16) ) ( preference p_r17   ( r17) )))
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
(* (is-violated p_r16) 1)
(* (is-violated p_r17) 5)
))
)
