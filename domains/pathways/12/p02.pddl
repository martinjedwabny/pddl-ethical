(define (problem grounded-pathways-02_GEN)
(:domain grounded-pathways-propositional_GEN )
(:init ( foo) ( not-chosen_p53p1) ( not-chosen_p130) ( not-chosen_max) ( not-chosen_hdac1-prbp1-e2f4-dp12) ( not-chosen_hdac1-prbp1-e2f13-dp12) ( not-chosen_hdac1-p130-e2f4p1-dp12) ( not-chosen_hbp1) ( not-chosen_ge2) ( not-chosen_e2f6-dp12p1) ( not-chosen_e2f4-dp12p1) ( not-chosen_e2f13p1-dp12) ( not-chosen_cdk1p1p2) ( num-subs_l0))
(:goal (and ( goal2_) ( goal1_) ( preference p_r0   ( chosen_e2f6-dp12p1) ) ( preference p_r1   ( not-chosen_p53p1) ) ( preference p_r2   ( chosen_cdk1p1p2) ) ( preference p_r3  ( not(not-chosen_max)) ) ( preference p_r4   ( available_e2f13p1-dp12-ge2) ) ( preference p_r5  ( not(available_p21-gadd45)) ) ( preference p_r6   ( goal2_) ) ( preference p_r7  ( not(r7)) ) ( preference p_r8   ( r8) ) ( preference p_r9  ( not(r9)) ) ( preference p_r10  ( not(r10)) ) ( preference p_r11  ( not(r11)) )))
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
))
)
