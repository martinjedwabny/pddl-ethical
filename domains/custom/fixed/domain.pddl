(define (domain crash-d_GEN)
    (:requirements :strips :typing :equality :negative-preconditions :conditional-effects :action-costs)

    (:types
        car xpos ypos direction gravity - object
    )

    (:constants
        agent c1 c2 - car
        left straight right - direction
        low high - gravity
        x1 x2 - xpos
        y1 y2 y3 y4 - ypos
    )

    (:predicates
        (haspos ?c - car ?x1 - xpos ?y1 - ypos)
        (hasdir ?c - car ?d - direction)
        (nextx ?d - direction ?x1 - xpos ?x2 - xpos)
        (nexty ?y1 - ypos ?y2 - ypos)
        (hascrashed ?c1 - car)
        (hasbumped ?c1 - car)
        (updated)
        (check)
        (checkFinal)
        (danger ?c1 - car ?g1 - gravity)
        (damagerail ?c1 - car)
        (responsibleagent)
        (checked-damagerail-agent)
        (checked-damagerail-c1)
        (checked-damagerail-c2)
        (checked-danger-agent-low)
        (checked-danger-c1-low)
        (checked-danger-c2-low)
        (checked-danger-agent-high)
        (checked-danger-c1-high)
        (checked-danger-c2-high)
        (checked-responsibleagent)
    )

    (:functions
        (total-cost) - number
    )

    (:action finalCheck
        :parameters ()
        :precondition (not (checkFinal))
        :effect (checkFinal)
    )

    ; PART 1

    (:action check-damagerail-agent
        :parameters ()
        :precondition (and (checkFinal) (damagerail agent))
        :effect (and (checked-damagerail-agent) (increase (total-cost) 1))
    )

    (:action check-damagerail-c1
        :parameters ()
        :precondition (and (checkFinal) (damagerail c1))
        :effect (and (checked-damagerail-c1) (increase (total-cost) 1))
    )

    (:action check-damagerail-c2
        :parameters ()
        :precondition (and (checkFinal) (damagerail c2))
        :effect (and (checked-damagerail-c2) (increase (total-cost) 1))
    )

    (:action check-danger-agent-low
        :parameters ()
        :precondition (and (checkFinal) (danger agent low))
        :effect (and (checked-danger-agent-low) (increase (total-cost) 4))
    )

    (:action check-danger-c1-low
        :parameters ()
        :precondition (and (checkFinal) (danger c1 low))
        :effect (and (checked-danger-c1-low) (increase (total-cost) 4))
    )

    (:action check-danger-c2-low
        :parameters ()
        :precondition (and (checkFinal) (danger c2 low))
        :effect (and (checked-danger-c2-low) (increase (total-cost) 4))
    )

    (:action check-danger-agent-high
        :parameters ()
        :precondition (and (checkFinal) (danger agent high))
        :effect (and (checked-danger-agent-high) (increase (total-cost) 48))
    )

    (:action check-danger-c1-high
        :parameters ()
        :precondition (and (checkFinal) (danger c1 high))
        :effect (and (checked-danger-c1-high) (increase (total-cost) 16))
    )

    (:action check-danger-c2-high
        :parameters ()
        :precondition (and (checkFinal) (danger c2 high))
        :effect (and (checked-danger-c2-high) (increase (total-cost) 16))
    )

    (:action check-responsibleagent
        :parameters ()
        :precondition (and (checkFinal) (responsibleagent))
        :effect (and (checked-responsibleagent) (increase (total-cost) 48))
    )

    ; PART 2

    (:action check-damagerail-agent-neg
        :parameters ()
        :precondition (and (checkFinal) (not (damagerail agent)))
        :effect (checked-damagerail-agent)
    )

    (:action check-damagerail-c1-neg
        :parameters ()
        :precondition (and (checkFinal) (not (damagerail c1)))
        :effect (checked-damagerail-c1)
    )

    (:action check-damagerail-c2-neg
        :parameters ()
        :precondition (and (checkFinal) (not (damagerail c2)))
        :effect (checked-damagerail-c2)
    )

    (:action check-danger-agent-low-neg
        :parameters ()
        :precondition (and (checkFinal) (not (danger agent low)))
        :effect (checked-danger-agent-low)
    )

    (:action check-danger-c1-low-neg
        :parameters ()
        :precondition (and (checkFinal) (not (danger c1 low)))
        :effect (checked-danger-c1-low)
    )

    (:action check-danger-c2-low-neg
        :parameters ()
        :precondition (and (checkFinal) (not (danger c2 low)))
        :effect (checked-danger-c2-low)
    )

    (:action check-danger-agent-high-neg
        :parameters ()
        :precondition (and (checkFinal) (not (danger agent high)))
        :effect (checked-danger-agent-high)
    )

    (:action check-danger-c1-high-neg
        :parameters ()
        :precondition (and (checkFinal) (not (danger c1 high)))
        :effect (checked-danger-c1-high)
    )

    (:action check-danger-c2-high-neg
        :parameters ()
        :precondition (and (checkFinal) (not (danger c2 high)))
        :effect (checked-danger-c2-high)
    )

    (:action check-responsibleagent-neg
        :parameters ()
        :precondition (and (checkFinal) (not (responsibleagent)))
        :effect (checked-responsibleagent)
    )

    (:action setdir
        :parameters (?d1 - direction)
        :precondition (and
            (updated)
            (check)
            (not (checkFinal))
        )

        :effect (and
            (not (check))
            (not (hasdir agent left))
            (not (hasdir agent straight))
            (not (hasdir agent right))
            (hasdir agent ?d1)
        )

    )
    (:action setstop
        :parameters ()
        :precondition (and
            (updated)
            (check)
            (not (checkFinal))
        )

        :effect (and
            (not (check))
            (not (hasdir agent left))
            (not (hasdir agent straight))
            (not (hasdir agent right))
        )

    )
    (:action update
        :parameters ()
        :precondition (and
            (not (updated))
            (check)
            (not (checkFinal))
        )

        :effect (and
            (not (check))
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
        :precondition (and
            (updated)
            (check)
            (not (checkFinal))
        )

        :effect (and
            (not (check))
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
            (forall
                (?c1 - car ?x1 - xpos ?y1 - ypos)
                (when
                    (and
                        (haspos ?c1 ?x1 ?y1)
                        (hasdir ?c1 left)
                        (nextx left ?x1 ?x1)
                    )
                    (damagerail ?c1)))
            (forall
                (?c1 - car ?x1 - xpos ?y1 - ypos)
                (when
                    (and
                        (haspos ?c1 ?x1 ?y1)
                        (hasdir ?c1 right)
                        (nextx right ?x1 ?x1)
                    )
                    (damagerail ?c1)))
        )

    )
    (:action checkOp
        :parameters ()
        :precondition (and
            (not (check))
            (not (checkFinal))
        )
        :effect (and
            (check)
            (forall
                (?c1 - car)
                (when
                    (hascrashed ?c1)
                    (danger ?c1 high)))
            (forall
                (?c1 - car)
                (when
                    (hasbumped ?c1)
                    (danger ?c1 low)))
            (when
                (hascrashed agent)
                (responsibleagent))
            (when
                (hasbumped agent)
                (responsibleagent))
        )

    )

)