(define (problem hospital-01-GEN)
(:domain hospital-GEN)
(:init ( athouse))
(:goal (and ( athospital) ( preference p-fast  ( tookhighway)) ( preference p-lied  ( not(lied)))))
(:metric maximize (+
(* (is-violated p-fast) -1)
(* (is-violated p-lied) -1)
))
)
