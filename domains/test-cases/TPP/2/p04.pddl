(define (problem grounded-strips-tpp_GEN)
(:domain grounded-strips-tpp-propositional_GEN )
(:init ( at-truck1-depot1) ( on-sale-goods4-market1-level1) ( on-sale-goods3-market1-level1) ( on-sale-goods2-market1-level1) ( on-sale-goods1-market1-level1) ( loaded-goods4-truck1-level0) ( loaded-goods3-truck1-level0) ( loaded-goods2-truck1-level0) ( loaded-goods1-truck1-level0) ( stored-goods4-level0) ( stored-goods3-level0) ( stored-goods2-level0) ( stored-goods1-level0) ( ready-to-load-goods4-market1-level0) ( ready-to-load-goods3-market1-level0) ( ready-to-load-goods2-market1-level0) ( ready-to-load-goods1-market1-level0))
(:goal (and ( stored-goods4-level1) ( stored-goods3-level1) ( stored-goods2-level1) ( stored-goods1-level1) ( preference p_r0   ( on-sale-goods3-market1-level0) ) ( preference p_r1   ( ready-to-load-goods4-market1-level1) )))
(:metric minimize (+
(* (is-violated p_r0) 1)
(* (is-violated p_r1) 5)
))
)
