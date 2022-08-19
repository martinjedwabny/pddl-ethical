(define (domain crash-d)
    (:requirements :strips :typing :equality :negative-preconditions :conditional-effects)

    (:types
        car xPos yPos direction - object
    )

    (:constants
        agent - car
        left straight right - direction
    )

    (:predicates
        (hasPos ?C - car ?X1 - xPos ?Y1 - yPos)
        (hasDir ?C - car ?D - direction)
        (nextX ?D - direction ?X1 - xPos ?X2 - xPos)
        (nextY ?Y1 - yPos ?Y2 - yPos)
        (hasCrashed ?C1 - car)
        (hasBumped ?C1 - car)
        (updated)
    )

    (:action setDir
        :parameters (?D1 - direction)
        :precondition (updated)
        :effect (and
            (not (hasDir agent left))
            (not (hasDir agent straight))
            (not (hasDir agent right))
            (hasDir agent ?D1))
    )

    (:action setStop
        :parameters ()
        :precondition (updated)
        :effect (and
            (not (hasDir agent left))
            (not (hasDir agent straight))
            (not (hasDir agent right)))
    )

    (:action update
        :parameters ()
        :precondition (not (updated))
        :effect (and
            (updated)
            (forall
                (?C1 - car ?C2 - car ?Y1 - yPos ?X1 - xPos)
                (when
                    (and
                        (not (= ?C1 ?C2))
                        (not (= ?C1 agent))
                        (not (= ?C2 agent))
                        (not (hasCrashed ?C1))
                        (hasPos ?C1 ?X1 ?Y1)
                        (hasPos ?C2 ?X1 ?Y1))
                    (and
                        (hasCrashed ?C1)
                        (hasCrashed ?C2)
                    )))
            (forall
                (?C1 - car ?Y1 - yPos ?X1 - xPos)
                (when
                    (and
                        (not (= ?C1 agent))
                        (hasPos agent ?X1 ?Y1)
                        (hasPos ?C1 ?X1 ?Y1))
                    (and
                        (hasBumped agent)
                        (hasBumped ?C1)
                        (not (hasDir ?C1 left))
                        (not (hasDir ?C1 straight))
                        (not (hasDir ?C1 right))
                    ))))
    )

    (:action go
        :parameters ()
        :precondition (updated)
        :effect (and
            (not (updated))
            (forall
                (?C1 - car ?D1 - direction ?Y1 - yPos ?Y2 - yPos ?X1 - xPos ?X2 - xPos)
                (when
                    (and
                        (not (hasCrashed ?C1))
                        (hasPos ?C1 ?X1 ?Y1)
                        (hasDir ?C1 ?D1)
                        (nextX ?D1 ?X1 ?X2)
                        (nextY ?Y1 ?Y2))
                    (and
                        (not (hasPos ?C1 ?X1 ?Y1))
                        (hasPos ?C1 ?X2 ?Y2))))
        )
    )
)