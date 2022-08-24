# Ethical PDDL

Extension of PDDL with ethical rules.

## Directories:
- domains: domains with ethical rules and transpiled versions.
- transpiler_costs: compiler from ethical rules to PDDL action costs.
- transpiler_preferences: compiler from ethical rules to PDDL preferences.
- downward: FD action cost PDDL planner [Link](https://www.fast-downward.org/).

## Run instructions:

- transpiler_costs: 

        python transpiler_costs/generator.py <DOMAIN-FILE> <PROBLEM-FILE> <OUT-DOMAIN-FILE> <OUT-PROBLEM-FILE>
    
    Example:

        python transpiler_costs/generator.py domains/custom/domain.pddl domains/custom/problem.pddl transpiled/custom/costs/domain.pddl transpiled/custom/costs/problem.pddl

- transpiler_preferences: 

        python transpiler_preferences/generator.py <DOMAIN-FILE> <PROBLEM-FILE> <OUT-DOMAIN-FILE> <OUT-PROBLEM-FILE>
    
    Example:

        python transpiler_preferences/generator.py domains/custom/domain.pddl domains/custom/problem.pddl transpiled/custom/preferences/domain.pddl transpiled/custom/preferences/problem.pddl

- planner:

        ./downward/fast-downward.py --alias seq-opt-fdss-1 --plan-file <OUT-FILE> --search-time-limit <TIME-LIMIT> <DOMAIN-FILE> <PROBLEM-FILE>
    
    Example:

        ./downward/fast-downward.py --alias seq-opt-fdss-1 --plan-file transpiled/custom/out.plan --search-time-limit 1000s transpiled/custom/domain.pddl transpiled/custom/problem.pddl

