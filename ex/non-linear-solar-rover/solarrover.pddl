(define (domain generator2 )
(:requirements :fluents :durative-actions :typing :negative-preconditions :time :duration-inequalities  :timed-initial-literals)
(:types generalBattery battery )
(:predicates (gboff ?gb - generalBattery) (gbon ?gb - generalBattery) (off ?b - battery) (on ?b - battery) (night) (sunexposure) (solarsupport) (datatosend) (datasent) (roversafe) (day))
(:functions (roverenergy) (SoC ?b - battery))

(:action switchGenBatteryOn
 :parameters (?gb - generalBattery )
 :precondition ( and (gboff ?gb))
 :effect (and (gbon ?gb) (not (gboff ?gb))
	      (roversafe)
	      (increase (roverenergy) 100)
	  )
)


(:durative-action useBattery
 :parameters (?b - battery)
 :duration (>= ?duration 0)
 :condition (and (at start (off ?b)) (over all (roversafe)) (over all (> (SoC ?b) 0)))
 :effect (and (at start (not (off ?b)))
	      (at start (on ?b))	      
	      (decrease (SoC ?b) (* #t 1))
	      (at start (increase (roverenergy) 10))
	      (at end (not (on ?b)))
	  )
)

(:event sunshine
 :parameters ()
 :precondition (and (night) (sunexposure))
 :effect (and (not (night)) (day))
)

(:process charging
:parameters ()
:precondition (and (day))
:effect (and 
		(increase (roverenergy) (* #t (* (* 0.05 (roverenergy)) ( * 0.05 (roverenergy)) ) ) )
	 )
)

(:action sendData
 :parameters ()
 :precondition (and (datatosend) (roversafe) (>= (roverenergy) 1000))
 :effect (and (datasent) (not (datatosend)))
)


)


