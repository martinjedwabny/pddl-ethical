(define (domain grounded-STRIPS-HOSPITAL_GEN)
(:requirements
:strips 
:fluents
)
(:predicates
(ATROAD)
(ATTOLL)
(ATHOSPITAL)
(BARRIEROPEN)
(IDA)
(IDB)
(LIED)
(ATHIGHWAY)
(TOOKHIGHWAY)
(ATHOUSE)
)
(:functions
(IS_VIOLATED_P_LIED)
(IS_SATISFIED_P_LIED)
(IS_VIOLATED_P_FAST)
(IS_SATISFIED_P_FAST)
)
(:action GOHIGHWAYHOSPITAL
:parameters ()
:precondition
(and
(ATHIGHWAY)
)
:effect
(and
(ATHOSPITAL)
(not (ATHIGHWAY))
)
)
(:action TAKEHIGHWAY
:parameters ()
:precondition
(and
(BARRIEROPEN)
(ATTOLL)
)
:effect
(and
(ATHIGHWAY)
(TOOKHIGHWAY)
(not (ATTOLL))
(assign ( IS_VIOLATED_P_FAST)  0)
(assign ( IS_SATISFIED_P_FAST)  1)
)
)
(:action PRESENTB
:parameters ()
:precondition
(and
(ATTOLL)
)
:effect
(and
(BARRIEROPEN)
(IDB)
(LIED)
(assign ( IS_VIOLATED_P_LIED)  1)
(assign ( IS_SATISFIED_P_LIED)  0)
)
)
(:action PRESENTA
:parameters ()
:precondition
(and
(ATTOLL)
)
:effect
(and
(BARRIEROPEN)
(IDA)
)
)
(:action GOROADHOSPITAL
:parameters ()
:precondition
(and
(ATROAD)
)
:effect
(and
(ATHOSPITAL)
(not (ATROAD))
)
)
(:action GOTOLL
:parameters ()
:precondition
(and
(ATHOUSE)
)
:effect
(and
(ATTOLL)
(not (ATHOUSE))
)
)
(:action GOROAD
:parameters ()
:precondition
(and
(ATHOUSE)
)
:effect
(and
(ATROAD)
(not (ATHOUSE))
)
)
)
