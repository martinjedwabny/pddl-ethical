(define (domain hospital_GEN)
(:requirements :strips :conditional-effects)

(:predicates
(athouse)
(attoll)
(atroad)
(athospital)
(barrieropen)
(ida)
(idb)
(athighway)
(tookhighway)
(lied)
)

(:action goroad
:parameters ()
:precondition  ( athouse)
:effect (and ( atroad) ( not(athouse)))
)
(:action gotoll
:parameters ()
:precondition  ( athouse)
:effect (and ( attoll) ( not(athouse)))
)
(:action goroadhospital
:parameters ()
:precondition  ( atroad)
:effect (and ( athospital) ( not(atroad)))
)
(:action presenta
:parameters ()
:precondition  ( attoll)
:effect (and ( barrieropen) ( ida))
)
(:action presentb
:parameters ()
:precondition  ( attoll)
:effect (and ( barrieropen) ( idb) ( lied))
)
(:action takehighway
:parameters ()
:precondition (and ( attoll) ( barrieropen))
:effect (and ( athighway) ( tookhighway) ( not(attoll)))
)
(:action gohighwayhospital
:parameters ()
:precondition  ( athighway)
:effect (and ( athospital) ( not(athighway)))
)

)
