(define (problem grounded-strips-tpp_GEN)
(:domain grounded-strips-tpp-propositional_GEN )
(:init ( at-truck1-depot1) ( on-sale-goods2-market1-level1) ( on-sale-goods1-market1-level1) ( loaded-goods2-truck1-level0) ( loaded-goods1-truck1-level0) ( stored-goods2-level0) ( stored-goods1-level0) ( ready-to-load-goods2-market1-level0) ( ready-to-load-goods1-market1-level0))
(:goal (and ( stored-goods2-level1) ( stored-goods1-level1) ( preference p_r0   ( ready-to-load-goods1-market1-level1) ) ( preference p_r1   ( at-truck1-depot1) )))
(:metric minimize (+
(* (is-violated p_r0) 1)
(* (is-violated p_r1) 5)
))
)
