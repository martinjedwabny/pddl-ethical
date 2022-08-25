(define (domain hospital)
    (:requirements :strips :typing :conditional-effects :negative-preconditions :equality)

    (:types
        position id - object
    )

    (:constants
        house toll sideroad highway hospital - position
        a b - id
    )

    (:predicates
        (at ?P1 - position)
        (connected ?P1 - position ?P2 - position)
        (openBarrier)
        (presentedId ?I1 - id)
    )

    (:action go
        :parameters (?P1 - position ?P2 - position)
        :precondition (and (at ?P1) (connected ?P1 ?P2))
        :effect (and
            (when
                (and (= ?P1 toll) (openBarrier))
                (and (not (at ?P1)) (at ?P2)))
            (when
                (not (= ?P1 toll))
                (and (not (at ?P1)) (at ?P2))))
    )

    (:action presentId
        :parameters (?I1 - id)
        :precondition (and (at toll) (not (openBarrier)))
        :effect (and (openBarrier) (presentedId ?I1))
    )

)