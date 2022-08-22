(define (problem grounded-pathways-03_GEN)
(:domain grounded-pathways-propositional_GEN )
(:init ( foo) ( not-chosen_wee1) ( not-chosen_sp1) ( not-chosen_rpa) ( not-chosen_prbp2) ( not-chosen_prb) ( not-chosen_p16) ( not-chosen_p130) ( not-chosen_hdac1) ( not-chosen_gp) ( not-chosen_e2f4-dp12p1) ( not-chosen_c-tak1) ( not-chosen_cks1) ( not-chosen_chk1) ( not-chosen_cdk46p3-cycd) ( not-chosen_cdk46p1) ( not-chosen_cdk2p2-cycb) ( not-chosen_cdk1p1p2) ( not-chosen_cdc25c) ( not-chosen_ap2) ( num-subs_l0))
(:goal (and ( goal3_) ( goal2_) ( goal1_) ( preference p_r0   ( chosen_gp) ) ( preference p_r1   ( available_prbp1p2-ap2) ) ( preference p_r2   ( chosen_cks1) ) ( preference p_r3  ( not(not-chosen_c-tak1)) ) ( preference p_r4   ( not-chosen_sp1) ) ( preference p_r5  ( not(available_cdk2p1p2-cycb)) ) ( preference p_r6   ( available_prbp1p2) ) ( preference p_r7  ( not(available_cdk1p1p2-cks1)) ) ( preference p_r8   ( available_c-myc) ) ( preference p_r9  ( not(num-subs_l0)) ) ( preference p_r10  ( not(r10)) ) ( preference p_r11  ( not(r11)) ) ( preference p_r12   ( r12) ) ( preference p_r13   ( r13) ) ( preference p_r14  ( not(r14)) ) ( preference p_r15  ( not(r15)) ) ( preference p_r16   ( r16) ) ( preference p_r17   ( r17) )))
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
(* (is-violated p_r12) 5)
(* (is-violated p_r13) 1)
(* (is-violated p_r14) 4)
(* (is-violated p_r15) 3)
(* (is-violated p_r16) 1)
(* (is-violated p_r17) 5)
))
)
