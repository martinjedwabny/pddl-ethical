(define (domain grounded-pathways-propositional_GEN)
(:requirements :conditional-effects :preferences :strips)

(:predicates
(foo)
(chosen_p53p1)
(num-subs_l1)
(chosen_p130)
(chosen_max)
(chosen_hdac1-prbp1-e2f4-dp12)
(chosen_hdac1-prbp1-e2f13-dp12)
(chosen_hdac1-p130-e2f4p1-dp12)
(chosen_hbp1)
(chosen_ge2)
(chosen_e2f6-dp12p1)
(chosen_e2f4-dp12p1)
(chosen_e2f13p1-dp12)
(chosen_cdk1p1p2)
(available_p53p1)
(available_p130)
(available_max)
(available_hdac1-prbp1-e2f4-dp12)
(available_hdac1-prbp1-e2f13-dp12)
(available_hdac1-p130-e2f4p1-dp12)
(available_hbp1)
(available_ge2)
(available_e2f6-dp12p1)
(available_e2f4-dp12p1)
(available_e2f13p1-dp12)
(available_cdk1p1p2)
(available_p130-e2f4-dp12p1)
(available_p130-e2f4-dp12p1-ge2)
(available_hdac1-prbp1-e2f4-dp12-ge2)
(available_hdac1-prbp1-e2f13-dp12-ge2)
(available_hdac1-p130-e2f4p1-dp12-ge2)
(available_hbp1-p130)
(available_e2f6-dp12p1-ge2)
(available_e2f13p1-dp12-ge2)
(available_p21)
(available_mdm2)
(available_gadd45)
(available_c-fos)
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
(available_p21-gadd45)
(available_p107-e2f4-dp12p1)
(available_p107-e2f4-dp12p1-ge2)
(available_mdm2-e2f13p1-dp12)
(available_c-myc-max)
(available_cdk1p1p2-gadd45)
(goal1_)
(num-subs_l3)
(not-chosen_cdk1p1p2)
(not-chosen_e2f13p1-dp12)
(not-chosen_e2f4-dp12p1)
(not-chosen_e2f6-dp12p1)
(not-chosen_ge2)
(not-chosen_hbp1)
(not-chosen_hdac1-p130-e2f4p1-dp12)
(not-chosen_hdac1-prbp1-e2f13-dp12)
(not-chosen_hdac1-prbp1-e2f4-dp12)
(not-chosen_max)
(not-chosen_p130)
(not-chosen_p53p1)
(num-subs_l0)
(r6)
(r7)
(r8)
(r9)
)

(:action choose_cdk1p1p2_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_cdk1p1p2))
:effect (and ( chosen_cdk1p1p2) ( num-subs_l3) ( not(not-chosen_cdk1p1p2)) ( not(num-subs_l2)))
)
(:action choose_e2f13p1-dp12_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_e2f13p1-dp12))
:effect (and ( chosen_e2f13p1-dp12) ( num-subs_l3) ( not(not-chosen_e2f13p1-dp12)) ( not(num-subs_l2)))
)
(:action choose_e2f4-dp12p1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_e2f4-dp12p1))
:effect (and ( chosen_e2f4-dp12p1) ( num-subs_l3) ( not(not-chosen_e2f4-dp12p1)) ( not(num-subs_l2)))
)
(:action choose_e2f6-dp12p1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_e2f6-dp12p1))
:effect (and ( chosen_e2f6-dp12p1) ( num-subs_l3) ( not(not-chosen_e2f6-dp12p1)) ( not(num-subs_l2)))
)
(:action choose_ge2_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_ge2))
:effect (and ( chosen_ge2) ( num-subs_l3) ( not(not-chosen_ge2)) ( not(num-subs_l2)))
)
(:action choose_hbp1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_hbp1))
:effect (and ( chosen_hbp1) ( num-subs_l3) ( not(not-chosen_hbp1)) ( not(num-subs_l2)))
)
(:action choose_hdac1-p130-e2f4p1-dp12_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_hdac1-p130-e2f4p1-dp12))
:effect (and ( chosen_hdac1-p130-e2f4p1-dp12) ( num-subs_l3) ( not(not-chosen_hdac1-p130-e2f4p1-dp12)) ( not(num-subs_l2)))
)
(:action choose_hdac1-prbp1-e2f13-dp12_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_hdac1-prbp1-e2f13-dp12))
:effect (and ( chosen_hdac1-prbp1-e2f13-dp12) ( num-subs_l3) ( not(not-chosen_hdac1-prbp1-e2f13-dp12)) ( not(num-subs_l2)))
)
(:action choose_hdac1-prbp1-e2f4-dp12_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_hdac1-prbp1-e2f4-dp12))
:effect (and ( chosen_hdac1-prbp1-e2f4-dp12) ( num-subs_l3) ( not(not-chosen_hdac1-prbp1-e2f4-dp12)) ( not(num-subs_l2)))
)
(:action choose_max_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_max))
:effect (and ( chosen_max) ( num-subs_l3) ( not(not-chosen_max)) ( not(num-subs_l2)))
)
(:action choose_p130_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_p130))
:effect (and ( chosen_p130) ( num-subs_l3) ( not(not-chosen_p130)) ( not(num-subs_l2)))
)
(:action choose_p53p1_l3_l2
:parameters ()
:precondition (and ( num-subs_l2) ( not-chosen_p53p1))
:effect (and ( chosen_p53p1) ( num-subs_l3) ( not(not-chosen_p53p1)) ( not(num-subs_l2)))
)
(:action dummy-action-2-1
:parameters ()
:precondition  ( available_c-myc-max)
:effect  ( goal2_)
)
(:action dummy-action-1-1
:parameters ()
:precondition  ( available_p107-e2f4-dp12p1)
:effect  ( goal1_)
)
(:action dummy-action-1-2
:parameters ()
:precondition  ( available_p107-e2f4-dp12p1-ge2)
:effect  ( goal1_)
)
(:action associate_cdk1p1p2_gadd45_cdk1p1p2-gadd45
:parameters ()
:precondition (and ( available_gadd45) ( available_cdk1p1p2))
:effect (and ( available_cdk1p1p2-gadd45) ( not(available_cdk1p1p2)) ( not(available_gadd45)))
)
(:action associate_c-myc_max_c-myc-max
:parameters ()
:precondition (and ( available_max) ( available_c-myc))
:effect (and ( available_c-myc-max) ( not(available_c-myc)) ( not(available_max)))
)
(:action associate_mdm2_e2f13p1-dp12_mdm2-e2f13p1-dp12
:parameters ()
:precondition (and ( available_e2f13p1-dp12) ( available_mdm2))
:effect (and ( available_mdm2-e2f13p1-dp12) ( not(available_mdm2)) ( not(available_e2f13p1-dp12)))
)
(:action associate_p107-e2f4-dp12p1_ge2_p107-e2f4-dp12p1-ge2
:parameters ()
:precondition (and ( available_ge2) ( available_p107-e2f4-dp12p1))
:effect (and ( available_p107-e2f4-dp12p1-ge2) ( not(available_p107-e2f4-dp12p1)) ( not(available_ge2)))
)
(:action associate_p107_e2f4-dp12p1_p107-e2f4-dp12p1
:parameters ()
:precondition (and ( available_e2f4-dp12p1) ( available_p107))
:effect (and ( available_p107-e2f4-dp12p1) ( not(available_p107)) ( not(available_e2f4-dp12p1)))
)
(:action associate_p21_gadd45_p21-gadd45
:parameters ()
:precondition (and ( available_gadd45) ( available_p21))
:effect (and ( available_p21-gadd45) ( not(available_p21)) ( not(available_gadd45)))
)
(:action choose_cdk1p1p2_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_cdk1p1p2))
:effect (and ( chosen_cdk1p1p2) ( num-subs_l2) ( not(not-chosen_cdk1p1p2)) ( not(num-subs_l1)))
)
(:action choose_e2f13p1-dp12_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_e2f13p1-dp12))
:effect (and ( chosen_e2f13p1-dp12) ( num-subs_l2) ( not(not-chosen_e2f13p1-dp12)) ( not(num-subs_l1)))
)
(:action choose_e2f4-dp12p1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_e2f4-dp12p1))
:effect (and ( chosen_e2f4-dp12p1) ( num-subs_l2) ( not(not-chosen_e2f4-dp12p1)) ( not(num-subs_l1)))
)
(:action choose_e2f6-dp12p1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_e2f6-dp12p1))
:effect (and ( chosen_e2f6-dp12p1) ( num-subs_l2) ( not(not-chosen_e2f6-dp12p1)) ( not(num-subs_l1)))
)
(:action choose_ge2_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_ge2))
:effect (and ( chosen_ge2) ( num-subs_l2) ( not(not-chosen_ge2)) ( not(num-subs_l1)))
)
(:action choose_hbp1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_hbp1))
:effect (and ( chosen_hbp1) ( num-subs_l2) ( not(not-chosen_hbp1)) ( not(num-subs_l1)))
)
(:action choose_hdac1-p130-e2f4p1-dp12_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_hdac1-p130-e2f4p1-dp12))
:effect (and ( chosen_hdac1-p130-e2f4p1-dp12) ( num-subs_l2) ( not(not-chosen_hdac1-p130-e2f4p1-dp12)) ( not(num-subs_l1)))
)
(:action choose_hdac1-prbp1-e2f13-dp12_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_hdac1-prbp1-e2f13-dp12))
:effect (and ( chosen_hdac1-prbp1-e2f13-dp12) ( num-subs_l2) ( not(not-chosen_hdac1-prbp1-e2f13-dp12)) ( not(num-subs_l1)))
)
(:action choose_hdac1-prbp1-e2f4-dp12_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_hdac1-prbp1-e2f4-dp12))
:effect (and ( chosen_hdac1-prbp1-e2f4-dp12) ( num-subs_l2) ( not(not-chosen_hdac1-prbp1-e2f4-dp12)) ( not(num-subs_l1)))
)
(:action choose_max_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_max))
:effect (and ( chosen_max) ( num-subs_l2) ( not(not-chosen_max)) ( not(num-subs_l1)))
)
(:action choose_p130_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_p130))
:effect (and ( chosen_p130) ( num-subs_l2) ( not(not-chosen_p130)) ( not(num-subs_l1)))
)
(:action choose_p53p1_l2_l1
:parameters ()
:precondition (and ( num-subs_l1) ( not-chosen_p53p1))
:effect (and ( chosen_p53p1) ( num-subs_l2) ( not(not-chosen_p53p1)) ( not(num-subs_l1)))
)
(:action dummy-action-2-2
:parameters ()
:precondition  ( available_cycdp1)
:effect  ( goal2_)
)
(:action synthesize_e2f13p1-dp12-ge2_c-myc
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_c-myc)
)
(:action synthesize_e2f13p1-dp12-ge2_cyca
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_cyca)
)
(:action synthesize_e2f13p1-dp12-ge2_cycd
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_cycd)
)
(:action synthesize_e2f13p1-dp12-ge2_cycdp1
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_cycdp1)
)
(:action synthesize_e2f13p1-dp12-ge2_cyce
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_cyce)
)
(:action synthesize_e2f13p1-dp12-ge2_cycep1
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_cycep1)
)
(:action synthesize_e2f13p1-dp12-ge2_p107
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_p107)
)
(:action synthesize_e2f13p1-dp12-ge2_p107p1
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_p107p1)
)
(:action synthesize_e2f13p1-dp12-ge2_p19arf
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_p19arf)
)
(:action synthesize_e2f13p1-dp12-ge2_pol
:parameters ()
:precondition  ( available_e2f13p1-dp12-ge2)
:effect  ( available_pol)
)
(:action synthesize_p53p1_c-fos
:parameters ()
:precondition  ( available_p53p1)
:effect  ( available_c-fos)
)
(:action synthesize_p53p1_gadd45
:parameters ()
:precondition  ( available_p53p1)
:effect  ( available_gadd45)
)
(:action synthesize_p53p1_mdm2
:parameters ()
:precondition  ( available_p53p1)
:effect  ( available_mdm2)
)
(:action synthesize_p53p1_p21
:parameters ()
:precondition  ( available_p53p1)
:effect (and ( available_p21) ( r7))
)
(:action associate_e2f13p1-dp12_ge2_e2f13p1-dp12-ge2
:parameters ()
:precondition (and ( available_ge2) ( available_e2f13p1-dp12))
:effect (and ( available_e2f13p1-dp12-ge2) ( r6) ( not(available_e2f13p1-dp12)) ( not(available_ge2)))
)
(:action associate_e2f6-dp12p1_ge2_e2f6-dp12p1-ge2
:parameters ()
:precondition (and ( available_ge2) ( available_e2f6-dp12p1))
:effect (and ( available_e2f6-dp12p1-ge2) ( not(available_e2f6-dp12p1)) ( not(available_ge2)))
)
(:action associate_hbp1_p130_hbp1-p130
:parameters ()
:precondition (and ( available_p130) ( available_hbp1))
:effect (and ( available_hbp1-p130) ( not(available_hbp1)) ( not(available_p130)))
)
(:action associate_hdac1-p130-e2f4p1-dp12_ge2_hdac1-p130-e2f4p1-dp12-ge2
:parameters ()
:precondition (and ( available_ge2) ( available_hdac1-p130-e2f4p1-dp12))
:effect (and ( available_hdac1-p130-e2f4p1-dp12-ge2) ( not(available_hdac1-p130-e2f4p1-dp12)) ( not(available_ge2)))
)
(:action associate_hdac1-prbp1-e2f13-dp12_ge2_hdac1-prbp1-e2f13-dp12-ge2
:parameters ()
:precondition (and ( available_ge2) ( available_hdac1-prbp1-e2f13-dp12))
:effect (and ( available_hdac1-prbp1-e2f13-dp12-ge2) ( not(available_hdac1-prbp1-e2f13-dp12)) ( not(available_ge2)))
)
(:action associate_hdac1-prbp1-e2f4-dp12_ge2_hdac1-prbp1-e2f4-dp12-ge2
:parameters ()
:precondition (and ( available_ge2) ( available_hdac1-prbp1-e2f4-dp12))
:effect (and ( available_hdac1-prbp1-e2f4-dp12-ge2) ( not(available_hdac1-prbp1-e2f4-dp12)) ( not(available_ge2)))
)
(:action associate_p130-e2f4-dp12p1_ge2_p130-e2f4-dp12p1-ge2
:parameters ()
:precondition (and ( available_ge2) ( available_p130-e2f4-dp12p1))
:effect (and ( available_p130-e2f4-dp12p1-ge2) ( not(available_p130-e2f4-dp12p1)) ( not(available_ge2)))
)
(:action associate_p130_e2f4-dp12p1_p130-e2f4-dp12p1
:parameters ()
:precondition (and ( available_e2f4-dp12p1) ( available_p130))
:effect (and ( available_p130-e2f4-dp12p1) ( not(available_p130)) ( not(available_e2f4-dp12p1)))
)
(:action initialize_cdk1p1p2
:parameters ()
:precondition  ( chosen_cdk1p1p2)
:effect  ( available_cdk1p1p2)
)
(:action initialize_e2f13p1-dp12
:parameters ()
:precondition  ( chosen_e2f13p1-dp12)
:effect  ( available_e2f13p1-dp12)
)
(:action initialize_e2f4-dp12p1
:parameters ()
:precondition  ( chosen_e2f4-dp12p1)
:effect  ( available_e2f4-dp12p1)
)
(:action initialize_e2f6-dp12p1
:parameters ()
:precondition  ( chosen_e2f6-dp12p1)
:effect (and ( available_e2f6-dp12p1) ( r9))
)
(:action initialize_ge2
:parameters ()
:precondition  ( chosen_ge2)
:effect  ( available_ge2)
)
(:action initialize_hbp1
:parameters ()
:precondition  ( chosen_hbp1)
:effect (and ( available_hbp1) ( r8))
)
(:action initialize_hdac1-p130-e2f4p1-dp12
:parameters ()
:precondition  ( chosen_hdac1-p130-e2f4p1-dp12)
:effect  ( available_hdac1-p130-e2f4p1-dp12)
)
(:action initialize_hdac1-prbp1-e2f13-dp12
:parameters ()
:precondition  ( chosen_hdac1-prbp1-e2f13-dp12)
:effect  ( available_hdac1-prbp1-e2f13-dp12)
)
(:action initialize_hdac1-prbp1-e2f4-dp12
:parameters ()
:precondition  ( chosen_hdac1-prbp1-e2f4-dp12)
:effect  ( available_hdac1-prbp1-e2f4-dp12)
)
(:action initialize_max
:parameters ()
:precondition  ( chosen_max)
:effect  ( available_max)
)
(:action initialize_p130
:parameters ()
:precondition  ( chosen_p130)
:effect  ( available_p130)
)
(:action initialize_p53p1
:parameters ()
:precondition  ( chosen_p53p1)
:effect  ( available_p53p1)
)
(:action choose_cdk1p1p2_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_cdk1p1p2))
:effect (and ( chosen_cdk1p1p2) ( num-subs_l1) ( not(not-chosen_cdk1p1p2)) ( not(num-subs_l0)))
)
(:action choose_e2f13p1-dp12_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_e2f13p1-dp12))
:effect (and ( chosen_e2f13p1-dp12) ( num-subs_l1) ( not(not-chosen_e2f13p1-dp12)) ( not(num-subs_l0)))
)
(:action choose_e2f4-dp12p1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_e2f4-dp12p1))
:effect (and ( chosen_e2f4-dp12p1) ( num-subs_l1) ( not(not-chosen_e2f4-dp12p1)) ( not(num-subs_l0)))
)
(:action choose_e2f6-dp12p1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_e2f6-dp12p1))
:effect (and ( chosen_e2f6-dp12p1) ( num-subs_l1) ( not(not-chosen_e2f6-dp12p1)) ( not(num-subs_l0)))
)
(:action choose_ge2_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_ge2))
:effect (and ( chosen_ge2) ( num-subs_l1) ( not(not-chosen_ge2)) ( not(num-subs_l0)))
)
(:action choose_hbp1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_hbp1))
:effect (and ( chosen_hbp1) ( num-subs_l1) ( not(not-chosen_hbp1)) ( not(num-subs_l0)))
)
(:action choose_hdac1-p130-e2f4p1-dp12_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_hdac1-p130-e2f4p1-dp12))
:effect (and ( chosen_hdac1-p130-e2f4p1-dp12) ( num-subs_l1) ( not(not-chosen_hdac1-p130-e2f4p1-dp12)) ( not(num-subs_l0)))
)
(:action choose_hdac1-prbp1-e2f13-dp12_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_hdac1-prbp1-e2f13-dp12))
:effect (and ( chosen_hdac1-prbp1-e2f13-dp12) ( num-subs_l1) ( not(not-chosen_hdac1-prbp1-e2f13-dp12)) ( not(num-subs_l0)))
)
(:action choose_hdac1-prbp1-e2f4-dp12_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_hdac1-prbp1-e2f4-dp12))
:effect (and ( chosen_hdac1-prbp1-e2f4-dp12) ( num-subs_l1) ( not(not-chosen_hdac1-prbp1-e2f4-dp12)) ( not(num-subs_l0)))
)
(:action choose_max_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_max))
:effect (and ( chosen_max) ( num-subs_l1) ( not(not-chosen_max)) ( not(num-subs_l0)))
)
(:action choose_p130_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_p130))
:effect (and ( chosen_p130) ( num-subs_l1) ( not(not-chosen_p130)) ( not(num-subs_l0)))
)
(:action choose_p53p1_l1_l0
:parameters ()
:precondition (and ( num-subs_l0) ( not-chosen_p53p1))
:effect (and ( chosen_p53p1) ( num-subs_l1) ( not(not-chosen_p53p1)) ( not(num-subs_l0)))
)

)
