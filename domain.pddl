(define (domain crash-d_GEN)
(:requirements :action-costs :strips :typing :equality :negative-preconditions :conditional-effects)

(:types
    car xpos ypos direction gravity  - object 
)

(:constants
    agent c1 c2  - car 
    left straight right  - direction 
    low high  - gravity 
    x1 x2  - xpos 
    y1 y2 y3 y4  - ypos 
)

(:predicates
    (haspos ?c - car ?x1 - xpos ?y1 - ypos )
    (hasdir ?c - car ?d - direction )
    (nextx ?d - direction ?x1 - xpos ?x2 - xpos )
    (nexty ?y1 - ypos ?y2 - ypos )
    (hascrashed ?c1 - car )
    (hasbumped ?c1 - car )
    (updated )
    (check )
    (danger ?c1 - car ?g1 - gravity )
    (damagerail ?c1 - car )
    (responsibleagent )
    (final-mode)
    (final-mode-check-damagerail-agent)
    (final-mode-check-damagerail-c1)
    (final-mode-check-damagerail-c2)
    (final-mode-check-danger-agent-low)
    (final-mode-check-danger-c1-low)
    (final-mode-check-danger-c2-low)
    (final-mode-check-danger-agent-high)
    (final-mode-check-danger-c1-high)
    (final-mode-check-danger-c2-high)
    (final-mode-check-responsibleagent)
)

(:action setdir
    :parameters (?d1 - direction )
    :precondition (and
(not (final-mode ))
(check )
(updated )
) 

    :effect (and
(not (check ))
(not (hasdir agent left ))
(not (hasdir agent straight ))
(not (hasdir agent right ))
(hasdir agent ?d1 )
) 


)
(:action setstop
    :parameters ()
    :precondition (and
(not (final-mode ))
(check )
(updated )
) 

    :effect (and
(not (check ))
(not (hasdir agent left ))
(not (hasdir agent straight ))
(not (hasdir agent right ))
) 


)
(:action update
    :parameters ()
    :precondition (and
(not (final-mode ))
(check )
(not (updated ))
) 

    :effect (and
(not (check ))
(updated )
(forall (?c1 - car ?c2 - car ?y1 - ypos ?x1 - xpos )
(when (and
(not (= ?c1 ?c2 ))
(not (= ?c1 agent ))
(not (= ?c2 agent ))
(not (hascrashed ?c1 ))
(haspos ?c1 ?x1 ?y1 )
(haspos ?c2 ?x1 ?y1 )
) 
 (and
(hascrashed ?c1 )
(hascrashed ?c2 )
) 
))
(forall (?c1 - car ?y1 - ypos ?x1 - xpos )
(when (and
(not (= ?c1 agent ))
(haspos agent ?x1 ?y1 )
(haspos ?c1 ?x1 ?y1 )
) 
 (and
(hasbumped agent )
(hasbumped ?c1 )
(not (hasdir ?c1 left ))
(not (hasdir ?c1 straight ))
(not (hasdir ?c1 right ))
) 
))
) 


)
(:action go
    :parameters ()
    :precondition (and
(not (final-mode ))
(check )
(updated )
) 

    :effect (and
(not (check ))
(not (updated ))
(forall (?c1 - car ?d1 - direction ?y1 - ypos ?y2 - ypos ?x1 - xpos ?x2 - xpos )
(when (and
(not (hascrashed ?c1 ))
(haspos ?c1 ?x1 ?y1 )
(hasdir ?c1 ?d1 )
(nextx ?d1 ?x1 ?x2 )
(nexty ?y1 ?y2 )
) 
 (and
(not (haspos ?c1 ?x1 ?y1 ))
(haspos ?c1 ?x2 ?y2 )
) 
))
(forall (?c1 - car ?x1 - xpos ?y1 - ypos )
(when (and
(haspos ?c1 ?x1 ?y1 )
(hasdir ?c1 left )
(nextx left ?x1 ?x1 )
) 
 (damagerail ?c1 )))
(forall (?c1 - car ?x1 - xpos ?y1 - ypos )
(when (and
(haspos ?c1 ?x1 ?y1 )
(hasdir ?c1 right )
(nextx right ?x1 ?x1 )
) 
 (damagerail ?c1 )))
) 


)
(:action check-ethical-features
    :parameters ()
    :precondition (and
(not (final-mode ))
(not (check ))
) 

    :effect (and
(check )
(forall (?c1 - car )
(when (hascrashed ?c1 ) (danger ?c1 high )))
(forall (?c1 - car )
(when (hasbumped ?c1 ) (danger ?c1 low )))
(when (hascrashed agent ) (responsibleagent ))
(when (hasbumped agent ) (responsibleagent ))
) 


)
(:action final-mode-start
    :parameters ()
    :precondition (not (final-mode ))
    :effect (final-mode )

)
(:action final-mode-check-op-False-damagerail-agent
    :parameters ()
    :precondition (and
(final-mode )
(damagerail agent )
(not (final-mode-check-damagerail-agent ))
) 

    :effect (and
(final-mode-check-damagerail-agent )
(increase (total-cost ) 1)
) 


)
(:action final-mode-check-op-True-damagerail-agent
    :parameters ()
    :precondition (and
(final-mode )
(not (damagerail agent ))
(not (final-mode-check-damagerail-agent ))
) 

    :effect (final-mode-check-damagerail-agent )

)
(:action final-mode-check-op-False-damagerail-c1
    :parameters ()
    :precondition (and
(final-mode )
(damagerail c1 )
(not (final-mode-check-damagerail-c1 ))
) 

    :effect (and
(final-mode-check-damagerail-c1 )
(increase (total-cost ) 1)
) 


)
(:action final-mode-check-op-True-damagerail-c1
    :parameters ()
    :precondition (and
(final-mode )
(not (damagerail c1 ))
(not (final-mode-check-damagerail-c1 ))
) 

    :effect (final-mode-check-damagerail-c1 )

)
(:action final-mode-check-op-False-damagerail-c2
    :parameters ()
    :precondition (and
(final-mode )
(damagerail c2 )
(not (final-mode-check-damagerail-c2 ))
) 

    :effect (and
(final-mode-check-damagerail-c2 )
(increase (total-cost ) 1)
) 


)
(:action final-mode-check-op-True-damagerail-c2
    :parameters ()
    :precondition (and
(final-mode )
(not (damagerail c2 ))
(not (final-mode-check-damagerail-c2 ))
) 

    :effect (final-mode-check-damagerail-c2 )

)
(:action final-mode-check-op-False-danger-agent-low
    :parameters ()
    :precondition (and
(final-mode )
(danger agent low )
(not (final-mode-check-danger-agent-low ))
) 

    :effect (and
(final-mode-check-danger-agent-low )
(increase (total-cost ) 4)
) 


)
(:action final-mode-check-op-True-danger-agent-low
    :parameters ()
    :precondition (and
(final-mode )
(not (danger agent low ))
(not (final-mode-check-danger-agent-low ))
) 

    :effect (final-mode-check-danger-agent-low )

)
(:action final-mode-check-op-False-danger-c1-low
    :parameters ()
    :precondition (and
(final-mode )
(danger c1 low )
(not (final-mode-check-danger-c1-low ))
) 

    :effect (and
(final-mode-check-danger-c1-low )
(increase (total-cost ) 4)
) 


)
(:action final-mode-check-op-True-danger-c1-low
    :parameters ()
    :precondition (and
(final-mode )
(not (danger c1 low ))
(not (final-mode-check-danger-c1-low ))
) 

    :effect (final-mode-check-danger-c1-low )

)
(:action final-mode-check-op-False-danger-c2-low
    :parameters ()
    :precondition (and
(final-mode )
(danger c2 low )
(not (final-mode-check-danger-c2-low ))
) 

    :effect (and
(final-mode-check-danger-c2-low )
(increase (total-cost ) 4)
) 


)
(:action final-mode-check-op-True-danger-c2-low
    :parameters ()
    :precondition (and
(final-mode )
(not (danger c2 low ))
(not (final-mode-check-danger-c2-low ))
) 

    :effect (final-mode-check-danger-c2-low )

)
(:action final-mode-check-op-False-danger-agent-high
    :parameters ()
    :precondition (and
(final-mode )
(danger agent high )
(not (final-mode-check-danger-agent-high ))
) 

    :effect (and
(final-mode-check-danger-agent-high )
(increase (total-cost ) 48)
) 


)
(:action final-mode-check-op-True-danger-agent-high
    :parameters ()
    :precondition (and
(final-mode )
(not (danger agent high ))
(not (final-mode-check-danger-agent-high ))
) 

    :effect (final-mode-check-danger-agent-high )

)
(:action final-mode-check-op-False-danger-c1-high
    :parameters ()
    :precondition (and
(final-mode )
(danger c1 high )
(not (final-mode-check-danger-c1-high ))
) 

    :effect (and
(final-mode-check-danger-c1-high )
(increase (total-cost ) 16)
) 


)
(:action final-mode-check-op-True-danger-c1-high
    :parameters ()
    :precondition (and
(final-mode )
(not (danger c1 high ))
(not (final-mode-check-danger-c1-high ))
) 

    :effect (final-mode-check-danger-c1-high )

)
(:action final-mode-check-op-False-danger-c2-high
    :parameters ()
    :precondition (and
(final-mode )
(danger c2 high )
(not (final-mode-check-danger-c2-high ))
) 

    :effect (and
(final-mode-check-danger-c2-high )
(increase (total-cost ) 16)
) 


)
(:action final-mode-check-op-True-danger-c2-high
    :parameters ()
    :precondition (and
(final-mode )
(not (danger c2 high ))
(not (final-mode-check-danger-c2-high ))
) 

    :effect (final-mode-check-danger-c2-high )

)
(:action final-mode-check-op-False-responsibleagent
    :parameters ()
    :precondition (and
(final-mode )
(responsibleagent )
(not (final-mode-check-responsibleagent ))
) 

    :effect (and
(final-mode-check-responsibleagent )
(increase (total-cost ) 48)
) 


)
(:action final-mode-check-op-True-responsibleagent
    :parameters ()
    :precondition (and
(final-mode )
(not (responsibleagent ))
(not (final-mode-check-responsibleagent ))
) 

    :effect (final-mode-check-responsibleagent )

)

)
