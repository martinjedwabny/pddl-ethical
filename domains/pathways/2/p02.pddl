(define (problem grounded-pathways-02_GEN)
(:domain grounded-pathways-propositional_GEN )
(:init ( foo) ( not-chosen_p53p1) ( not-chosen_p130) ( not-chosen_max) ( not-chosen_hdac1-prbp1-e2f4-dp12) ( not-chosen_hdac1-prbp1-e2f13-dp12) ( not-chosen_hdac1-p130-e2f4p1-dp12) ( not-chosen_hbp1) ( not-chosen_ge2) ( not-chosen_e2f6-dp12p1) ( not-chosen_e2f4-dp12p1) ( not-chosen_e2f13p1-dp12) ( not-chosen_cdk1p1p2) ( num-subs_l0))
(:goal (and ( goal2_) ( goal1_) ( preference p_r0   ( chosen_e2f6-dp12p1) ) ( preference p_r1   ( not-chosen_p53p1) )))
(:metric minimize (+
(* (is-violated p_r0) 1)
(* (is-violated p_r1) 5)
))
)
