(define (domain car)
(:requirements :typing :durative-actions :fluents :time :negative-preconditions :duration-inequalities )
(:predicates (landed) (crashed) (in_progress))
(:functions (M) (q) (d) (g) (v) (M_min) (ISP) (d_final) (v_margin) (d_margin))

(:durative-action falling
:parameters ()
:duration (<= ?duration 40)
:condition (and 
		(over all (< (d) (d_final)))
		(at start (not (landed)))
		(at start (not (in_progress)))
		(over all (not (crashed)))
		(at end (< (v) (v_margin)))
		(at end (< (d) (d_final)))
		(at end (> (d) (- (d_final) (d_margin))))
	      )
:effect (and 	
		(at start (in_progress))		
		(increase (d) (* #t (* 0.5 (v)) ) )
		(increase (v) (* #t (g)))
		(at end (not (in_progress)))
		(at end (landed))
	 )
)

(:durative-action thrust
:parameters ()
:duration (<= ?duration (/ (- (M) (M_min)) q) )
:condition (and (over all (in_progress)) (over all (< (d) (d_final))) (over all (not (landed))) (over all  (> (M) (M_min))) (over all (not (crashed))) )
:effect (and (decrease (v) (* #t (* (* (ISP) (g)) (/ (q) (M))))) 
	     (decrease (M) (* #t (q)))	
	)
)

;(:action land
;:parameters ()
;:precondition (and (in_progress) (not (landed)) (< (v) (v_margin)) (< (d) (d_final)) (> (d) (- (d_final) (d_margin))) (not (crashed)) )
;:effect (landed)
;)

(:event crash
:parameters ()
:precondition (and (> (d) (d_final)) )
:effect (crashed)
)

)

