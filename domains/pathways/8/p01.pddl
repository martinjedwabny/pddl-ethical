(define (problem grounded-pathways-01_GEN)
(:domain grounded-pathways-propositional_GEN )
(:init ( foo) ( not-chosen_sp1) ( not-chosen_raf1) ( not-chosen_prbp2) ( not-chosen_prb-e2f4p1-dp12) ( not-chosen_pcaf) ( not-chosen_p300) ( not-chosen_p16) ( not-chosen_p130-e2f5p1-dp12) ( not-chosen_e2f13) ( not-chosen_dmp1) ( not-chosen_chk1) ( not-chosen_cdk7) ( not-chosen_cdk46p3-cycdp1) ( not-chosen_cdk46p3-cycd) ( not-chosen_cdc25c) ( not-chosen_ap2) ( num-subs_l0))
(:goal (and ( goal1_) ( preference p_r0   ( chosen_e2f13) ) ( preference p_r1   ( chosen_cdc25c) ) ( preference p_r2   ( chosen_cdk7) ) ( preference p_r3  ( not(chosen_raf1)) ) ( preference p_r4   ( available_ap2) ) ( preference p_r5  ( not(r5)) ) ( preference p_r6   ( r6) ) ( preference p_r7  ( not(r7)) )))
(:metric minimize (+
(* (is-violated p_r0) 1)
(* (is-violated p_r1) 5)
(* (is-violated p_r2) 3)
(* (is-violated p_r3) 3)
(* (is-violated p_r4) 4)
(* (is-violated p_r5) 2)
(* (is-violated p_r6) 3)
(* (is-violated p_r7) 5)
))
)
