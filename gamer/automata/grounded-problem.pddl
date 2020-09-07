(define (problem grounded-hospital-01-gen)
(:domain grounded-hospital-gen)

  (:init 
     (athouse)
  )
  (:goal 
    (and 
      (preference p-fast (tookhighway) )
      (preference p-lied (not (lied) ) )
     (athospital)
     )
  )
  (:metric minimize (+ (* 1 (is-violated-p-fast)) (* 1 (is-violated-p-lied))) ;; ( + (total-cost) )
)
)
