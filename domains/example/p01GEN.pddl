(define (problem hospital-01_GEN)
(:domain hospital)
(:init ( athouse))
(:goal (and ( athospital) ( preference p_fast  ( not(tookhighway))) ( preference p_lied  ( not(lied)))))
(:metric maximize (+
(* (is-violated p_fast) -1)
(* (is-violated p_lied) -1)
))
)
