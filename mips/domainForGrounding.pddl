(define (domain hospital)
    (:requirements :strips :negative-preconditions :preferences)

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
        (honesty)
        (lied)
        (compassion)
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
            :effect (and (barrierOpen) (idA) (honesty))
        )

        (:action presentB
            :parameters ()
            :precondition (atToll)
            :effect (and (barrierOpen) (idB) (lied))
        )

        (:action takeHighway
            :parameters ()
            :precondition (and (atToll) (barrierOpen))
            :effect (and (atHighway) (tookHighway) (not (atToll)) (compassion))
        )

        (:action goHighwayHospital
            :parameters ()
            :precondition (atHighway)
            :effect (and (atHospital) (not (atHighway)))
        )

)
