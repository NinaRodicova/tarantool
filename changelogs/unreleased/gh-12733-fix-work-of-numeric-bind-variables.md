## bugfix/sql

* Now the number of a numeric variable corresponds to its position
  in the array of bind variables; the anonymous variable
  corresponds to the next variable; the named bind variables corresponds to
  a variable with the same name in the array of bind variables (gh-12733).
