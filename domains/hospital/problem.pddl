(define (problem hospital-01)
    (:domain hospital)
    (:init
        (at house)
        (connected house toll)
        (connected house sideroad)
        (connected toll highway)
        (connected highway hospital)
        (connected sideroad hospital)
    )
    (:goal
        (at hospital)
    )
)