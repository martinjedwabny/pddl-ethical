(define (problem grounded-pathways-04_GEN)
(:domain grounded-pathways-propositional_GEN )
(:init ( foo) ( not-chosen_wee1) ( not-chosen_sp1) ( not-chosen_rpa) ( not-chosen_prbp2) ( not-chosen_prbp1-e2f4p1-dp12) ( not-chosen_prb-e2f4p1-dp12) ( not-chosen_pcna) ( not-chosen_m1433) ( not-chosen_hdac1-p130-e2f4p1-dp12) ( not-chosen_hdac1) ( not-chosen_gp) ( not-chosen_ge2) ( not-chosen_e2f6-dp12p1) ( not-chosen_e2f4-dp12p1) ( not-chosen_e2f13) ( not-chosen_dmp1) ( not-chosen_c-tak1) ( not-chosen_cebp) ( not-chosen_cdk2p1) ( not-chosen_cdk2) ( not-chosen_cdc25c) ( not-chosen_c-abl) ( num-subs_l0))
(:goal (and ( goal4_) ( goal3_) ( goal2_) ( goal1_) ( preference p_r0   ( chosen_hdac1-p130-e2f4p1-dp12) ) ( preference p_r1   ( goal1_) ) ( preference p_r2   ( chosen_ge2) ) ( preference p_r3  ( not(not-chosen_cebp)) ) ( preference p_r4   ( not-chosen_c-abl) ) ( preference p_r5  ( not(available_prb-e2f4p1-dp12-ge2)) ) ( preference p_r6   ( available_sp1-e2f13-gp) ) ( preference p_r7  ( not(r7)) ) ( preference p_r8   ( r8) ) ( preference p_r9  ( not(r9)) ) ( preference p_r10  ( not(r10)) ) ( preference p_r11  ( not(r11)) )))
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
