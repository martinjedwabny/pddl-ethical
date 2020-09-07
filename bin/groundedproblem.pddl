(define (problem grounded-STRIPS-HOSPITAL-01_GEN)
(:domain grounded-STRIPS-HOSPITAL_GEN)
(:init
(ATHOUSE)
(=(IS_VIOLATED_P_LIED) 0)
(=(IS_SATISFIED_P_LIED) 1)
(=(IS_VIOLATED_P_FAST) 1)
(=(IS_SATISFIED_P_FAST) 0)
)
(:goal
(and
(ATHOSPITAL)
)
)
(:metric maximize (+ (* (IS_VIOLATED_P_FAST)-1)(* (IS_VIOLATED_P_LIED)-1)))
)
