(define (problem crash-p-01_GEN)
    (:domain crash-d_GEN)
    (:objects
        c1 c2 - car
        x1 x2 - xpos
        y1 y2 y3 y4 - ypos
    )

    (:init
        (updated)
        (haspos agent x1 y1)
        (haspos c1 x2 y1)
        (haspos c2 x2 y3)
        (hasdir agent straight)
        (hasdir c1 straight)
        (nextx straight x1 x1)
        (nextx straight x2 x2)
        (nextx right x1 x2)
        (nextx right x2 x2)
        (nextx left x1 x1)
        (nextx left x2 x1)
        (nexty y1 y2)
        (nexty y2 y3)
        (nexty y3 y4)
        (nexty y4 y4)
    )

    (:goal
        (and
            (updated)
            (haspos agent x2 y4)
            (not (hascrashed agent))
        )
    )

)