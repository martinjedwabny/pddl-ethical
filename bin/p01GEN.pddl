(define (problem hospital-01_GEN)
(:domain hospital_GEN)
(:init ( athouse))
(:goal (and ( athospital) ( preference p_fast  ( tookhighway)) ( preference p_lied  ( not(lied)))))
(:metric maximize (+
(* (is-violated p_fast) -1)
(* (is-violated p_lied) -1)
))
)
