(define (domain crash-d_GEN)
    (:requirements :preferences :strips :typing :equality :negative-preconditions :conditional-effects)

    (:types
        car xpos ypos direction gravity - object
    )

    (:constants
        agent - car
        left straight right - direction
        low high - gravity
    )

    (:predicates
        (haspos ?c - car ?x1 - xpos ?y1 - ypos)
        (hasdir ?c - car ?d - direction)
        (nextx ?d - direction ?x1 - xpos ?x2 - xpos)
        (nexty ?y1 - ypos ?y2 - ypos)
        (hascrashed ?c1 - car)
        (hasbumped ?c1 - car)
        (updated)
        (danger ?c1 - car ?g1 - gravity)
        (damagerail ?c1 - car)
    )

    (:action setdir
        :parameters (?d1 - direction)
        :precondition (updated)
        :effect (and
            (not (hasdir agent left))
            (not (hasdir agent straight))
            (not (hasdir agent right))
            (hasdir agent ?d1)
        )

    )
    (:action setstop
        :parameters ()
        :precondition (updated)
        :effect (and
            (not (hasdir agent left))
            (not (hasdir agent straight))
            (not (hasdir agent right))
        )

    )
    (:action update
        :parameters ()
        :precondition (not (updated))
        :effect (and
            (updated)
            (forall
                (?c1 - car ?c2 - car ?y1 - ypos ?x1 - xpos)
                (when
                    (and
                        (not (= ?c1 ?c2))
                        (not (= ?c1 agent))
                        (not (= ?c2 agent))
                        (not (hascrashed ?c1))
                        (haspos ?c1 ?x1 ?y1)
                        (haspos ?c2 ?x1 ?y1)
                    )
                    (and
                        (hascrashed ?c1)
                        (hascrashed ?c2)
                    )
                ))
            (forall
                (?c1 - car ?y1 - ypos ?x1 - xpos)
                (when
                    (and
                        (not (= ?c1 agent))
                        (haspos agent ?x1 ?y1)
                        (haspos ?c1 ?x1 ?y1)
                    )
                    (and
                        (hasbumped agent)
                        (hasbumped ?c1)
                        (not (hasdir ?c1 left))
                        (not (hasdir ?c1 straight))
                        (not (hasdir ?c1 right))
                    )
                ))
        )

    )
    (:action go
        :parameters ()
        :precondition (updated)
        :effect (and
            (not (updated))
            (forall
                (?c1 - car ?d1 - direction ?y1 - ypos ?y2 - ypos ?x1 - xpos ?x2 - xpos)
                (when
                    (and
                        (not (hascrashed ?c1))
                        (haspos ?c1 ?x1 ?y1)
                        (hasdir ?c1 ?d1)
                        (nextx ?d1 ?x1 ?x2)
                        (nexty ?y1 ?y2)
                    )
                    (and
                        (not (haspos ?c1 ?x1 ?y1))
                        (haspos ?c1 ?x2 ?y2)
                    )
                ))
        )

    )

)