	set %letter "A"

	!while &not &sgr %letter "Z"
		insert-string &cat &cat "FN" %letter " = "
		insert-string &cat &bind &cat "FN" %letter "~n"
		update-screen
		set %letter &chr &add &ascii %letter 1
	!endwhile
