(define (problem crash-p-01)
    (:domain crash-d)
    (:objects
        c1 c2 - car
        x1 x2 - xPos
        y1 y2 y3 y4 - yPos
    )
    (:init
        (updated)

        (hasPos agent x1 y1)
        (hasPos c1 x2 y1)
        (hasPos c2 x2 y3)

        (hasDir agent straight)
        (hasDir c1 straight)

        (nextX straight x1 x1)
        (nextX straight x2 x2)
        (nextX right x1 x2)
        (nextX right x2 x2)
        (nextX left x1 x1)
        (nextX left x2 x1)

        (nextY y1 y2)
        (nextY y2 y3)
        (nextY y3 y4)
        (nextY y4 y4)
    )

    (:goal
        (and (updated) (hasPos agent x2 y4) (not (hasCrashed agent)))
    )
)