(define (domain grounded-strips-tpp-propositional_GEN)
(:requirements :conditional-effects :preferences :strips)

(:predicates
(at-truck1-market1)
(on-sale-goods1-market1-level0)
(ready-to-load-goods1-market1-level1)
(loaded-goods1-truck1-level1)
(stored-goods1-level1)
(stored-goods1-level0)
(loaded-goods1-truck1-level0)
(on-sale-goods1-market1-level1)
(ready-to-load-goods1-market1-level0)
(at-truck1-depot1)
(r8)
(r9)
(r10)
(r11)
(r12)
(r13)
)

(:action unload-goods1-truck1-depot1-level0-level1-level0-level1
:parameters ()
:precondition (and ( stored-goods1-level0) ( loaded-goods1-truck1-level1) ( at-truck1-depot1))
:effect (and ( loaded-goods1-truck1-level0) ( stored-goods1-level1) ( not(loaded-goods1-truck1-level1)) ( not(stored-goods1-level0)))
)
(:action load-goods1-truck1-market1-level0-level1-level0-level1
:parameters ()
:precondition (and ( ready-to-load-goods1-market1-level1) ( loaded-goods1-truck1-level0) ( at-truck1-market1))
:effect (and ( loaded-goods1-truck1-level1) ( ready-to-load-goods1-market1-level0) ( r11) ( not(loaded-goods1-truck1-level0)) ( not(ready-to-load-goods1-market1-level1)))
)
(:action drive-truck1-market1-depot1
:parameters ()
:precondition  ( at-truck1-market1)
:effect (and ( at-truck1-depot1) ( not(at-truck1-market1)))
)
(:action buy-truck1-goods1-market1-level0-level1-level0-level1
:parameters ()
:precondition (and ( ready-to-load-goods1-market1-level0) ( on-sale-goods1-market1-level1) ( at-truck1-market1))
:effect (and ( on-sale-goods1-market1-level0) ( ready-to-load-goods1-market1-level1) ( r8) ( r9) ( not(on-sale-goods1-market1-level1)) ( not(ready-to-load-goods1-market1-level0)))
)
(:action drive-truck1-depot1-market1
:parameters ()
:precondition  ( at-truck1-depot1)
:effect (and ( at-truck1-market1) ( r10) ( r12) ( r13) ( not(at-truck1-depot1)))
)

)
