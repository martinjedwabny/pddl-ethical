(define (domain hospital)
    (:requirements :strips :conditional-effects :ethical-rules)

    (:predicates  
        (atHouse)
        (atToll)
        (atRoad)
        (atHospital)
        (barrierOpen)
        (idA) 
        (idB)
        (atHighway)
        (tookHighway)
        )

        (:action goRoad
            :parameters ()
            :precondition (atHouse)
            :effect (and (atRoad) (not (atHouse)))
        )

        (:action goToll
            :parameters ()
            :precondition (atHouse)
            :effect (and (atToll) (not (atHouse)))
        )

        (:action goRoadHospital
            :parameters ()
            :precondition (atRoad)
            :effect (and (atHospital) (not (atRoad)))
        )

        (:action presentA
            :parameters ()
            :precondition (atToll)
            :effect (and (barrierOpen) (idA))
        )

        (:action presentb
            :parameters ()
            :precondition (atToll)
            :effect (and (barrierOpen) (idB))
        )

        (:action takeHighway
            :parameters ()
            :precondition (and (atToll) (barrierOpen))
            :effect (and (atHighway) (tookHighway) (not (atToll)))
        )

        (:action goHighwayHospital
            :parameters ()
            :precondition (atHighway)
            :effect (and (atHospital) (not (atHighway)))
        )
        
        (:ethical-rule fast 
            :type (+)
            :precondition (tookHighway)
            :activation final
            :rank 1
        )
        
        (:ethical-rule lied
            :type (-)
            :precondition ()
            :activation presentB
            :rank 1
        )

)
