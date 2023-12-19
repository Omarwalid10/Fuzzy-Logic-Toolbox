# Fuzzy-Logic-Toolbox
* A “Fuzzy Logic Toolbox” provides functions and apps for designing and simulating fuzzy logic systems. It lets the user specify and configure input variables, output variables, membership functions, rules and defuzzification methods.

## How the algorithm acts 
* It takes variables, fuzzy sets to an existing variable, rules, and gets the crisp values that we will make a simulation on it and return the crisp output for the user.
* It can take multiple rules and multiple inputs in the same rule.
* It can take fuzzy set shapes either triangular or trapezoidal.
* The “run simulation” step performs fuzzification, inference, and defuzzification using the weighted average method.

## How to use it 
* 1- Add variables.<br>
    ```
    Ex:
      proj_funding IN 0 100
      exp_level IN 0 60
      risk OUT 0 100
      x
    ```
* 2- Add fuzzy sets to an existing variable.
   ```
    Ex:
      proj_funding
  
      very_low TRAP 0 0 10 30
      low TRAP 10 30 40 60
      medium TRAP 40 60 70 90
      high TRAP 70 90 100 100
      x
    ```
* 3- Add rules.
  ```
    Ex:
      proj_funding high or exp_level expert => risk low
      proj_funding medium and exp_level intermediate => risk normal
      proj_funding medium and exp_level beginner => risk normal
      proj_funding low and exp_level beginner => risk high
      proj_funding very_low and_not exp_level expert => risk high
      x
   
  ```
in the same order<br> 
* 4- Run the simulation on crisp values.
