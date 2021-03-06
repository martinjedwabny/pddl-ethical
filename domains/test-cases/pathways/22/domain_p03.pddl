(define (domain grounded-pathways-propositional_GEN)
(:requirements :conditional-effects :preferences :strips)

(:predicates
(foo)
(chosen_wee1)
(num-subs_l1)
(chosen_sp1)
(chosen_rpa)
(chosen_prbp2)
(chosen_prb)
(chosen_p16)
(chosen_p130)
(chosen_hdac1)
(chosen_gp)
(chosen_e2f4-dp12p1)
(chosen_c-tak1)
(chosen_cks1)
(chosen_chk1)
(chosen_cdk46p3-cycd)
(chosen_cdk46p1)
(chosen_cdk2p2-cycb)
(chosen_cdk1p1p2)
(chosen_cdc25c)
(chosen_ap2)
(available_wee1)
(available_sp1)
(available_rpa)
(available_prbp2)
(available_prb)
(available_p16)
(available_p130)
(available_hdac1)
(available_gp)
(available_e2f4-dp12p1)
(available_c-tak1)
(available_cks1)
(available_chk1)
(available_cdk46p3-cycd)
(available_cdk46p1)
(available_cdk2p2-cycb)
(available_cdk1p1p2)
(available_cdc25c)
(available_ap2)
(available_sp1-gp)
(available_prbp2-ap2)
(available_prb-e2f4-dp12p1)
(available_prb-ap2)
(available_p16-cdk46p1)
(available_p130-e2f4-dp12p1)
(available_hdac1-p130-e2f4-dp12p1)
(available_cdk1p1p2-cks1)
(available_prbp1p2)
(available_prbp1)
(available_cdk2p1p2-cycb)
(available_cdc25cp2)
(available_pol)
(available_p19arf)
(available_p107p1)
(available_p107)
(available_cycep1)
(available_cyce)
(available_cycdp1)
(available_cycd)
(available_cyca)
(available_c-myc)
(goal2_)
(num-subs_l2)
(available_sp1-p107)
(available_sp1-p107p1)
(available_sp1-p107p1-gp)
(available_sp1-p107-gp)
(available_rpa-cyca)
(available_prbp1p2-ap2)
(available_prbp1-e2f4-dp12p1)
(available_prbp1-ap2)
(available_p107-e2f4-dp12p1)
(available_hdac1-p107-e2f4-dp12p1)
(available_c-myc-ap2)
(available_cdk46p1-cycdp1)
(available_cdk46p1-cycd)
(goal1_)
(goal3_)
(num-subs_l3)
(not-chosen_ap2)
(not-chosen_cdc25c)
(not-chosen_cdk1p1p2)
(not-chosen_cdk2p2-cycb)
(not-chosen_cdk46p1)
(not-chosen_cdk46p3-cycd)
(not-chosen_chk1)
(not-chosen_cks1)
(not-chosen_c-tak1)
(not-chosen_e2f4-dp12p1)
(not-chosen_gp)
(not-chosen_hdac1)
(not-chosen_p130)
(not-chosen_p16)
(not-chosen_prb)
(not-chosen_prbp2)
(not-chosen_rpa)
(not-chosen_sp1)
(not-chosen_wee1)
(num-subs_l0)
(r12)
(r13)
(r14)
(r15)
(r16)
(r17)
(r18)
(r19)
(r20)
(r21)
)

(:action choose_ap2_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_ap2))
:effect (and ( chosen_ap2) ( num-subs_l3) ( not(not-chosen_ap2)) ( not(num-subs_l2)))
)
(:action choose_cdc25c_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_cdc25c))
:effect (and ( chosen_cdc25c) ( num-subs_l3) ( not(not-chosen_cdc25c)) ( not(num-subs_l2)))
)
(:action choose_cdk1p1p2_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_cdk1p1p2))
:effect (and ( chosen_cdk1p1p2) ( num-subs_l3) ( r21) ( not(not-chosen_cdk1p1p2)) ( not(num-subs_l2)))
)
(:action choose_cdk2p2-cycb_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_cdk2p2-cycb))
:effect (and ( chosen_cdk2p2-cycb) ( num-subs_l3) ( r20) ( not(not-chosen_cdk2p2-cycb)) ( not(num-subs_l2)))
)
(:action choose_cdk46p1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_cdk46p1))
:effect (and ( chosen_cdk46p1) ( num-subs_l3) ( not(not-chosen_cdk46p1)) ( not(num-subs_l2)))
)
(:action choose_cdk46p3-cycd_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_cdk46p3-cycd))
:effect (and ( chosen_cdk46p3-cycd) ( num-subs_l3) ( not(not-chosen_cdk46p3-cycd)) ( not(num-subs_l2)))
)
(:action choose_chk1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_chk1))
:effect (and ( chosen_chk1) ( num-subs_l3) ( not(not-chosen_chk1)) ( not(num-subs_l2)))
)
(:action choose_cks1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_cks1))
:effect (and ( chosen_cks1) ( num-subs_l3) ( not(not-chosen_cks1)) ( not(num-subs_l2)))
)
(:action choose_c-tak1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_c-tak1))
:effect (and ( chosen_c-tak1) ( num-subs_l3) ( not(not-chosen_c-tak1)) ( not(num-subs_l2)))
)
(:action choose_e2f4-dp12p1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_e2f4-dp12p1))
:effect (and ( chosen_e2f4-dp12p1) ( num-subs_l3) ( not(not-chosen_e2f4-dp12p1)) ( not(num-subs_l2)))
)
(:action choose_gp_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_gp))
:effect (and ( chosen_gp) ( num-subs_l3) ( not(not-chosen_gp)) ( not(num-subs_l2)))
)
(:action choose_hdac1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_hdac1))
:effect (and ( chosen_hdac1) ( num-subs_l3) ( not(not-chosen_hdac1)) ( not(num-subs_l2)))
)
(:action choose_p130_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_p130))
:effect (and ( chosen_p130) ( num-subs_l3) ( not(not-chosen_p130)) ( not(num-subs_l2)))
)
(:action choose_p16_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_p16))
:effect (and ( chosen_p16) ( num-subs_l3) ( not(not-chosen_p16)) ( not(num-subs_l2)))
)
(:action choose_prb_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_prb))
:effect (and ( chosen_prb) ( num-subs_l3) ( not(not-chosen_prb)) ( not(num-subs_l2)))
)
(:action choose_prbp2_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_prbp2))
:effect (and ( chosen_prbp2) ( num-subs_l3) ( r17) ( not(not-chosen_prbp2)) ( not(num-subs_l2)))
)
(:action choose_rpa_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_rpa))
:effect (and ( chosen_rpa) ( num-subs_l3) ( not(not-chosen_rpa)) ( not(num-subs_l2)))
)
(:action choose_sp1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_sp1))
:effect (and ( chosen_sp1) ( num-subs_l3) ( not(not-chosen_sp1)) ( not(num-subs_l2)))
)
(:action choose_wee1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_wee1))
:effect (and ( chosen_wee1) ( num-subs_l3) ( not(not-chosen_wee1)) ( not(num-subs_l2)))
)
(:action dummy-action-3-1
:parameters ()
:precondition  ( available_hdac1-p107-e2f4-dp12p1)
:effect  ( goal3_)
)
(:action dummy-action-3-2
:parameters ()
:precondition  ( available_cdk46p1-cycd)
:effect  ( goal3_)
)
(:action dummy-action-2-1
:parameters ()
:precondition  ( available_sp1-p107p1-gp)
:effect  ( goal2_)
)
(:action dummy-action-1-1
:parameters ()
:precondition  ( available_sp1-p107-gp)
:effect  ( goal1_)
)
(:action dummy-action-1-2
:parameters ()
:precondition  ( available_prbp1-e2f4-dp12p1)
:effect  ( goal1_)
)
(:action associate_cdk46p1_cycd_cdk46p1-cycd
:parameters ()
:precondition (and ( available_cycd) ( available_cdk46p1))
:effect (and ( available_cdk46p1-cycd) ( not(available_cdk46p1)) ( not(available_cycd)))
)
(:action associate_cdk46p1_cycdp1_cdk46p1-cycdp1
:parameters ()
:precondition (and ( available_cycdp1) ( available_cdk46p1))
:effect (and ( available_cdk46p1-cycdp1) ( not(available_cdk46p1)) ( not(available_cycdp1)))
)
(:action associate_c-myc_ap2_c-myc-ap2
:parameters ()
:precondition (and ( available_ap2) ( available_c-myc))
:effect (and ( available_c-myc-ap2) ( not(available_c-myc)) ( not(available_ap2)))
)
(:action associate_hdac1_p107-e2f4-dp12p1_hdac1-p107-e2f4-dp12p1
:parameters ()
:precondition (and ( available_p107-e2f4-dp12p1) ( available_hdac1))
:effect (and ( available_hdac1-p107-e2f4-dp12p1) ( not(available_hdac1)) ( not(available_p107-e2f4-dp12p1)))
)
(:action associate_p107_e2f4-dp12p1_p107-e2f4-dp12p1
:parameters ()
:precondition (and ( available_e2f4-dp12p1) ( available_p107))
:effect (and ( available_p107-e2f4-dp12p1) ( not(available_p107)) ( not(available_e2f4-dp12p1)))
)
(:action associate_prbp1_ap2_prbp1-ap2
:parameters ()
:precondition (and ( available_ap2) ( available_prbp1))
:effect (and ( available_prbp1-ap2) ( not(available_prbp1)) ( not(available_ap2)))
)
(:action associate_prbp1_e2f4-dp12p1_prbp1-e2f4-dp12p1
:parameters ()
:precondition (and ( available_e2f4-dp12p1) ( available_prbp1))
:effect (and ( available_prbp1-e2f4-dp12p1) ( not(available_prbp1)) ( not(available_e2f4-dp12p1)))
)
(:action associate_prbp1p2_ap2_prbp1p2-ap2
:parameters ()
:precondition (and ( available_ap2) ( available_prbp1p2))
:effect (and ( available_prbp1p2-ap2) ( not(available_prbp1p2)) ( not(available_ap2)))
)
(:action associate_rpa_cyca_rpa-cyca
:parameters ()
:precondition (and ( available_cyca) ( available_rpa))
:effect (and ( available_rpa-cyca) ( not(available_rpa)) ( not(available_cyca)))
)
(:action associate_sp1-p107_gp_sp1-p107-gp
:parameters ()
:precondition (and ( available_gp) ( available_sp1-p107))
:effect (and ( available_sp1-p107-gp) ( not(available_sp1-p107)) ( not(available_gp)))
)
(:action associate_sp1-p107p1_gp_sp1-p107p1-gp
:parameters ()
:precondition (and ( available_gp) ( available_sp1-p107p1))
:effect (and ( available_sp1-p107p1-gp) ( not(available_sp1-p107p1)) ( not(available_gp)))
)
(:action associate_sp1_p107p1_sp1-p107p1
:parameters ()
:precondition (and ( available_p107p1) ( available_sp1))
:effect (and ( available_sp1-p107p1) ( not(available_sp1)) ( not(available_p107p1)))
)
(:action associate_sp1_p107_sp1-p107
:parameters ()
:precondition (and ( available_p107) ( available_sp1))
:effect (and ( available_sp1-p107) ( not(available_sp1)) ( not(available_p107)))
)
(:action choose_ap2_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_ap2))
:effect (and ( chosen_ap2) ( num-subs_l2) ( not(not-chosen_ap2)) ( not(num-subs_l1)))
)
(:action choose_cdc25c_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_cdc25c))
:effect (and ( chosen_cdc25c) ( num-subs_l2) ( not(not-chosen_cdc25c)) ( not(num-subs_l1)))
)
(:action choose_cdk1p1p2_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_cdk1p1p2))
:effect (and ( chosen_cdk1p1p2) ( num-subs_l2) ( not(not-chosen_cdk1p1p2)) ( not(num-subs_l1)))
)
(:action choose_cdk2p2-cycb_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_cdk2p2-cycb))
:effect (and ( chosen_cdk2p2-cycb) ( num-subs_l2) ( not(not-chosen_cdk2p2-cycb)) ( not(num-subs_l1)))
)
(:action choose_cdk46p1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_cdk46p1))
:effect (and ( chosen_cdk46p1) ( num-subs_l2) ( not(not-chosen_cdk46p1)) ( not(num-subs_l1)))
)
(:action choose_cdk46p3-cycd_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_cdk46p3-cycd))
:effect (and ( chosen_cdk46p3-cycd) ( num-subs_l2) ( not(not-chosen_cdk46p3-cycd)) ( not(num-subs_l1)))
)
(:action choose_chk1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_chk1))
:effect (and ( chosen_chk1) ( num-subs_l2) ( not(not-chosen_chk1)) ( not(num-subs_l1)))
)
(:action choose_cks1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_cks1))
:effect (and ( chosen_cks1) ( num-subs_l2) ( not(not-chosen_cks1)) ( not(num-subs_l1)))
)
(:action choose_c-tak1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_c-tak1))
:effect (and ( chosen_c-tak1) ( num-subs_l2) ( not(not-chosen_c-tak1)) ( not(num-subs_l1)))
)
(:action choose_e2f4-dp12p1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_e2f4-dp12p1))
:effect (and ( chosen_e2f4-dp12p1) ( num-subs_l2) ( r15) ( not(not-chosen_e2f4-dp12p1)) ( not(num-subs_l1)))
)
(:action choose_gp_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_gp))
:effect (and ( chosen_gp) ( num-subs_l2) ( not(not-chosen_gp)) ( not(num-subs_l1)))
)
(:action choose_hdac1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_hdac1))
:effect (and ( chosen_hdac1) ( num-subs_l2) ( not(not-chosen_hdac1)) ( not(num-subs_l1)))
)
(:action choose_p130_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_p130))
:effect (and ( chosen_p130) ( num-subs_l2) ( not(not-chosen_p130)) ( not(num-subs_l1)))
)
(:action choose_p16_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_p16))
:effect (and ( chosen_p16) ( num-subs_l2) ( not(not-chosen_p16)) ( not(num-subs_l1)))
)
(:action choose_prb_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_prb))
:effect (and ( chosen_prb) ( num-subs_l2) ( not(not-chosen_prb)) ( not(num-subs_l1)))
)
(:action choose_prbp2_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_prbp2))
:effect (and ( chosen_prbp2) ( num-subs_l2) ( not(not-chosen_prbp2)) ( not(num-subs_l1)))
)
(:action choose_rpa_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_rpa))
:effect (and ( chosen_rpa) ( num-subs_l2) ( not(not-chosen_rpa)) ( not(num-subs_l1)))
)
(:action choose_sp1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_sp1))
:effect (and ( chosen_sp1) ( num-subs_l2) ( not(not-chosen_sp1)) ( not(num-subs_l1)))
)
(:action choose_wee1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_wee1))
:effect (and ( chosen_wee1) ( num-subs_l2) ( not(not-chosen_wee1)) ( not(num-subs_l1)))
)
(:action dummy-action-2-2
:parameters ()
:precondition  ( available_cyca)
:effect  ( goal2_)
)
(:action synthesize_sp1-gp_c-myc
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_c-myc)
)
(:action synthesize_sp1-gp_cyca
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_cyca)
)
(:action synthesize_sp1-gp_cycd
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_cycd)
)
(:action synthesize_sp1-gp_cycdp1
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_cycdp1)
)
(:action synthesize_sp1-gp_cyce
:parameters ()
:precondition  ( available_sp1-gp)
:effect (and ( available_cyce) ( r14))
)
(:action synthesize_sp1-gp_cycep1
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_cycep1)
)
(:action synthesize_sp1-gp_p107
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_p107)
)
(:action synthesize_sp1-gp_p107p1
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_p107p1)
)
(:action synthesize_sp1-gp_p19arf
:parameters ()
:precondition  ( available_sp1-gp)
:effect  ( available_p19arf)
)
(:action synthesize_sp1-gp_pol
:parameters ()
:precondition  ( available_sp1-gp)
:effect (and ( available_pol) ( r13))
)
(:action associate-with-catalyze_cdc25c_chk1_cdc25cp2
:parameters ()
:precondition (and ( available_chk1) ( available_cdc25c))
:effect (and ( available_cdc25cp2) ( not(available_cdc25c)))
)
(:action associate-with-catalyze_cdc25c_c-tak1_cdc25cp2
:parameters ()
:precondition (and ( available_c-tak1) ( available_cdc25c))
:effect (and ( available_cdc25cp2) ( not(available_cdc25c)))
)
(:action associate-with-catalyze_cdk2p2-cycb_wee1_cdk2p1p2-cycb
:parameters ()
:precondition (and ( available_wee1) ( available_cdk2p2-cycb))
:effect (and ( available_cdk2p1p2-cycb) ( not(available_cdk2p2-cycb)))
)
(:action associate-with-catalyze_prb_cdk46p3-cycd_prbp1
:parameters ()
:precondition (and ( available_cdk46p3-cycd) ( available_prb))
:effect (and ( available_prbp1) ( not(available_prb)))
)
(:action associate-with-catalyze_prbp2_cdk46p3-cycd_prbp1p2
:parameters ()
:precondition (and ( available_cdk46p3-cycd) ( available_prbp2))
:effect (and ( available_prbp1p2) ( not(available_prbp2)))
)
(:action associate_cdk1p1p2_cks1_cdk1p1p2-cks1
:parameters ()
:precondition (and ( available_cks1) ( available_cdk1p1p2))
:effect (and ( available_cdk1p1p2-cks1) ( not(available_cdk1p1p2)) ( not(available_cks1)))
)
(:action associate_hdac1_p130-e2f4-dp12p1_hdac1-p130-e2f4-dp12p1
:parameters ()
:precondition (and ( available_p130-e2f4-dp12p1) ( available_hdac1))
:effect (and ( available_hdac1-p130-e2f4-dp12p1) ( not(available_hdac1)) ( not(available_p130-e2f4-dp12p1)))
)
(:action associate_p130_e2f4-dp12p1_p130-e2f4-dp12p1
:parameters ()
:precondition (and ( available_e2f4-dp12p1) ( available_p130))
:effect (and ( available_p130-e2f4-dp12p1) ( not(available_p130)) ( not(available_e2f4-dp12p1)))
)
(:action associate_p16_cdk46p1_p16-cdk46p1
:parameters ()
:precondition (and ( available_cdk46p1) ( available_p16))
:effect (and ( available_p16-cdk46p1) ( not(available_p16)) ( not(available_cdk46p1)))
)
(:action associate_prb_ap2_prb-ap2
:parameters ()
:precondition (and ( available_ap2) ( available_prb))
:effect (and ( available_prb-ap2) ( not(available_prb)) ( not(available_ap2)))
)
(:action associate_prb_e2f4-dp12p1_prb-e2f4-dp12p1
:parameters ()
:precondition (and ( available_e2f4-dp12p1) ( available_prb))
:effect (and ( available_prb-e2f4-dp12p1) ( not(available_prb)) ( not(available_e2f4-dp12p1)))
)
(:action associate_prbp2_ap2_prbp2-ap2
:parameters ()
:precondition (and ( available_ap2) ( available_prbp2))
:effect (and ( available_prbp2-ap2) ( not(available_prbp2)) ( not(available_ap2)))
)
(:action associate_sp1_gp_sp1-gp
:parameters ()
:precondition (and ( available_gp) ( available_sp1))
:effect (and ( available_sp1-gp) ( not(available_sp1)) ( not(available_gp)))
)
(:action initialize_ap2
:parameters ()
:precondition  ( chosen_ap2)
:effect  ( available_ap2)
)
(:action initialize_cdc25c
:parameters ()
:precondition  ( chosen_cdc25c)
:effect  ( available_cdc25c)
)
(:action initialize_cdk1p1p2
:parameters ()
:precondition  ( chosen_cdk1p1p2)
:effect  ( available_cdk1p1p2)
)
(:action initialize_cdk2p2-cycb
:parameters ()
:precondition  ( chosen_cdk2p2-cycb)
:effect  ( available_cdk2p2-cycb)
)
(:action initialize_cdk46p1
:parameters ()
:precondition  ( chosen_cdk46p1)
:effect (and ( available_cdk46p1) ( r16))
)
(:action initialize_cdk46p3-cycd
:parameters ()
:precondition  ( chosen_cdk46p3-cycd)
:effect  ( available_cdk46p3-cycd)
)
(:action initialize_chk1
:parameters ()
:precondition  ( chosen_chk1)
:effect (and ( available_chk1) ( r18))
)
(:action initialize_cks1
:parameters ()
:precondition  ( chosen_cks1)
:effect  ( available_cks1)
)
(:action initialize_c-tak1
:parameters ()
:precondition  ( chosen_c-tak1)
:effect (and ( available_c-tak1) ( r19))
)
(:action initialize_e2f4-dp12p1
:parameters ()
:precondition  ( chosen_e2f4-dp12p1)
:effect (and ( available_e2f4-dp12p1) ( r12))
)
(:action initialize_gp
:parameters ()
:precondition  ( chosen_gp)
:effect  ( available_gp)
)
(:action initialize_hdac1
:parameters ()
:precondition  ( chosen_hdac1)
:effect  ( available_hdac1)
)
(:action initialize_p130
:parameters ()
:precondition  ( chosen_p130)
:effect  ( available_p130)
)
(:action initialize_p16
:parameters ()
:precondition  ( chosen_p16)
:effect  ( available_p16)
)
(:action initialize_prb
:parameters ()
:precondition  ( chosen_prb)
:effect  ( available_prb)
)
(:action initialize_prbp2
:parameters ()
:precondition  ( chosen_prbp2)
:effect  ( available_prbp2)
)
(:action initialize_rpa
:parameters ()
:precondition  ( chosen_rpa)
:effect  ( available_rpa)
)
(:action initialize_sp1
:parameters ()
:precondition  ( chosen_sp1)
:effect  ( available_sp1)
)
(:action initialize_wee1
:parameters ()
:precondition  ( chosen_wee1)
:effect  ( available_wee1)
)
(:action choose_ap2_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_ap2))
:effect (and ( chosen_ap2) ( num-subs_l1) ( not(not-chosen_ap2)) ( not(num-subs_l0)))
)
(:action choose_cdc25c_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_cdc25c))
:effect (and ( chosen_cdc25c) ( num-subs_l1) ( not(not-chosen_cdc25c)) ( not(num-subs_l0)))
)
(:action choose_cdk1p1p2_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_cdk1p1p2))
:effect (and ( chosen_cdk1p1p2) ( num-subs_l1) ( not(not-chosen_cdk1p1p2)) ( not(num-subs_l0)))
)
(:action choose_cdk2p2-cycb_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_cdk2p2-cycb))
:effect (and ( chosen_cdk2p2-cycb) ( num-subs_l1) ( not(not-chosen_cdk2p2-cycb)) ( not(num-subs_l0)))
)
(:action choose_cdk46p1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_cdk46p1))
:effect (and ( chosen_cdk46p1) ( num-subs_l1) ( not(not-chosen_cdk46p1)) ( not(num-subs_l0)))
)
(:action choose_cdk46p3-cycd_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_cdk46p3-cycd))
:effect (and ( chosen_cdk46p3-cycd) ( num-subs_l1) ( not(not-chosen_cdk46p3-cycd)) ( not(num-subs_l0)))
)
(:action choose_chk1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_chk1))
:effect (and ( chosen_chk1) ( num-subs_l1) ( not(not-chosen_chk1)) ( not(num-subs_l0)))
)
(:action choose_cks1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_cks1))
:effect (and ( chosen_cks1) ( num-subs_l1) ( not(not-chosen_cks1)) ( not(num-subs_l0)))
)
(:action choose_c-tak1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_c-tak1))
:effect (and ( chosen_c-tak1) ( num-subs_l1) ( not(not-chosen_c-tak1)) ( not(num-subs_l0)))
)
(:action choose_e2f4-dp12p1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_e2f4-dp12p1))
:effect (and ( chosen_e2f4-dp12p1) ( num-subs_l1) ( not(not-chosen_e2f4-dp12p1)) ( not(num-subs_l0)))
)
(:action choose_gp_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_gp))
:effect (and ( chosen_gp) ( num-subs_l1) ( not(not-chosen_gp)) ( not(num-subs_l0)))
)
(:action choose_hdac1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_hdac1))
:effect (and ( chosen_hdac1) ( num-subs_l1) ( not(not-chosen_hdac1)) ( not(num-subs_l0)))
)
(:action choose_p130_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_p130))
:effect (and ( chosen_p130) ( num-subs_l1) ( not(not-chosen_p130)) ( not(num-subs_l0)))
)
(:action choose_p16_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_p16))
:effect (and ( chosen_p16) ( num-subs_l1) ( not(not-chosen_p16)) ( not(num-subs_l0)))
)
(:action choose_prb_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_prb))
:effect (and ( chosen_prb) ( num-subs_l1) ( not(not-chosen_prb)) ( not(num-subs_l0)))
)
(:action choose_prbp2_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_prbp2))
:effect (and ( chosen_prbp2) ( num-subs_l1) ( not(not-chosen_prbp2)) ( not(num-subs_l0)))
)
(:action choose_rpa_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_rpa))
:effect (and ( chosen_rpa) ( num-subs_l1) ( not(not-chosen_rpa)) ( not(num-subs_l0)))
)
(:action choose_sp1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_sp1))
:effect (and ( chosen_sp1) ( num-subs_l1) ( not(not-chosen_sp1)) ( not(num-subs_l0)))
)
(:action choose_wee1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_wee1))
:effect (and ( chosen_wee1) ( num-subs_l1) ( not(not-chosen_wee1)) ( not(num-subs_l0)))
)

)
