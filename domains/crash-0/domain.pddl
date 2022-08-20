(define (domain crash-d)
    (:requirements :strips :typing :equality :negative-preconditions :conditional-effects :ethical)

    (:types
        car xPos yPos direction gravity - object
    )

    (:constants
        agent - car
        left straight right - direction
        low high - gravity
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

    (:ethical-features
        (danger ?C1 - car ?G1 - gravity)
        (damageRail ?C1 - car)
    )

    (:ethical-rank
        :feature
        (damageRail agent)
        :type -
        :rank 1
    )
    (:ethical-rank
        :feature
        (damageRail c1)
        :type -
        :rank 1
    )
    (:ethical-rank
        :feature
        (damageRail c2)
        :type -
        :rank 1
    )
    (:ethical-rank
        :feature
        (danger agent low)
        :type -
        :rank 2
    )
    (:ethical-rank
        :feature
        (danger c1 low)
        :type -
        :rank 2
    )
    (:ethical-rank
        :feature
        (danger c2 low)
        :type -
        :rank 2
    )
    (:ethical-rank
        :feature
        (danger agent high)
        :type -
        :rank 4
    )
    (:ethical-rank
        :feature
        (danger c1 high)
        :type -
        :rank 3
    )
    (:ethical-rank
        :feature
        (danger c2 high)
        :type -
        :rank 3
    )
    (:ethical-rank
        :feature
        (responsibleCollision agent)
        :type -
        :rank 4
    )

    (:ethical-rule crashRule
        :parameters (?C1 - car)
        :precondition (hasCrashed ?C1)
        :activation null
        :features
        (danger ?C1 high)
    )
    (:ethical-rule bumpRule
        :parameters (?C1 - car)
        :precondition (hasBumped ?C1)
        :activation null
        :features
        (danger ?C1 low)
    )
    (:ethical-rule responsibleCrashRule
        :parameters ()
        :precondition (hasCrashed agent)
        :activation null
        :features
        (resposibleCollision agent)
    )
    (:ethical-rule responsibleBumpRule
        :parameters ()
        :precondition (hasBumped agent)
        :activation null
        :features
        (resposibleCollision agent)
    )
    (:ethical-rule railLeftRule
        :parameters (?C1 - car ?X1 - xPos ?Y1 - yPos)
        :precondition (and
            (position ?C1 ?X1 ?Y1)
            (direction ?C1 left)
            (nextX left ?X1 ?X1))
        :activation
        (go())
        :features
        (damageRail ?C1)
    )
    (:ethical-rule railRightRule
        :parameters (?C1 - car ?X1 - xPos ?Y1 - yPos)
        :precondition (and
            (position ?C1 ?X1 ?Y1)
            (direction ?C1 right)
            (nextX right ?X1 ?X1))
        :activation
        (go())
        :features
        (damageRail ?C1)
    )
)