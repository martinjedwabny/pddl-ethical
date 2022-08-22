(define (problem grounded-strips-tpp_GEN)
(:domain grounded-strips-tpp-propositional_GEN )
(:init ( at-truck1-depot1) ( on-sale-goods2-market1-level1) ( on-sale-goods1-market1-level1) ( loaded-goods2-truck1-level0) ( loaded-goods1-truck1-level0) ( stored-goods2-level0) ( stored-goods1-level0) ( ready-to-load-goods2-market1-level0) ( ready-to-load-goods1-market1-level0))
(:goal (and ( stored-goods2-level1) ( stored-goods1-level1) ( preference p_r0   ( ready-to-load-goods1-market1-level1) ) ( preference p_r1   ( at-truck1-depot1) ) ( preference p_r2   ( on-sale-goods2-market1-level0) ) ( preference p_r3  ( not(ready-to-load-goods1-market1-level0)) ) ( preference p_r4   ( stored-goods2-level1) ) ( preference p_r5  ( not(loaded-goods1-truck1-level0)) ) ( preference p_r6   ( loaded-goods1-truck1-level0) ) ( preference p_r7  ( not(loaded-goods2-truck1-level0)) ) ( preference p_r8   ( on-sale-goods1-market1-level1) ) ( preference p_r9  ( not(r9)) ) ( preference p_r10  ( not(r10)) ) ( preference p_r11  ( not(r11)) ) ( preference p_r12   ( r12) ) ( preference p_r13   ( r13) ) ( preference p_r14  ( not(r14)) ) ( preference p_r15  ( not(r15)) )))
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
