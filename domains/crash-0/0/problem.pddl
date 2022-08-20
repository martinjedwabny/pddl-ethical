(define (problem crash-p-01_GEN)
(:domain crash-d_GEN )
(:objects
    c1 c2  - car 
    x1 x2  - xpos 
    y1 y2 y3 y4  - ypos 
)

(:init 
(updated )
(haspos agent x1 y1 )
(haspos c1 x2 y1 )
(haspos c2 x2 y3 )
(hasdir agent straight )
(hasdir c1 straight )
(nextx straight x1 x1 )
(nextx straight x2 x2 )
(nextx right x1 x2 )
(nextx right x2 x2 )
(nextx left x1 x1 )
(nextx left x2 x1 )
(nexty y1 y2 )
(nexty y2 y3 )
(nexty y3 y4 )
(nexty y4 y4 ))

(:goal (and 
 (and
(updated )
(haspos agent x2 y4 )
(not (hascrashed agent ))
) 
 
 (and
(preference p_damagerail-agent (not (damagerail-agent)))
(preference p_damagerail-c1 (not (damagerail-c1)))
(preference p_damagerail-c2 (not (damagerail-c2)))
(preference p_danger-agent-low (not (danger-agent-low)))
(preference p_danger-c1-low (not (danger-c1-low)))
(preference p_danger-c2-low (not (danger-c2-low)))
(preference p_danger-agent-high (not (danger-agent-high)))
(preference p_danger-c1-high (not (danger-c1-high)))
(preference p_danger-c2-high (not (danger-c2-high)))
(preference p_responsiblecollision-agent (not (responsiblecollision-agent)))
) 
) 
)

(:metric minimize (+
(* (is-violated p_damagerail-agent) 1)
(* (is-violated p_damagerail-c1) 1)
(* (is-violated p_damagerail-c2) 1)
(* (is-violated p_danger-agent-low) 4)
(* (is-violated p_danger-c1-low) 4)
(* (is-violated p_danger-c2-low) 4)
(* (is-violated p_danger-agent-high) 48)
(* (is-violated p_danger-c1-high) 16)
(* (is-violated p_danger-c2-high) 16)
(* (is-violated p_responsiblecollision-agent) 48)
)
)
