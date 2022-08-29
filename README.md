# Ethical PDDL

Extension of PDDL with ethical rules.

Presented in the FLAIRS conference as a [full paper](https://journals.flvc.org/FLAIRS/article/view/128492/130071).

## Directories:
- domains: domains with ethical rules and transpiled versions.
- transpiler_costs: compiler from ethical rules to PDDL action costs.
- transpiler_preferences: compiler from ethical rules to PDDL preferences.
- downward: FD action cost PDDL planner [Link](https://www.fast-downward.org/).

## Run instructions:

- transpiler_costs: 

        python transpiler_costs/generator.py <DOMAIN-FILE> <PROBLEM-FILE> <OUT-DOMAIN-FILE> <OUT-PROBLEM-FILE>
    
    Example:

        python transpiler_costs/generator.py domains/driver/domain.pddl domains/driver/problem.pddl transpiled/driver/costs/domain.pddl transpiled/driver/costs/problem.pddl

- transpiler_preferences: 

        python transpiler_preferences/generator.py <DOMAIN-FILE> <PROBLEM-FILE> <OUT-DOMAIN-FILE> <OUT-PROBLEM-FILE>
    
    Example:

        python transpiler_preferences/generator.py domains/driver/domain.pddl domains/driver/problem.pddl transpiled/driver/preferences/domain.pddl transpiled/driver/preferences/problem.pddl

- planner:

        ./downward/fast-downward.py --alias seq-opt-fdss-1 --plan-file <OUT-FILE> --search-time-limit <TIME-LIMIT> <DOMAIN-FILE> <PROBLEM-FILE>
    
    Example:

        ./downward/fast-downward.py --alias seq-opt-fdss-1 --plan-file transpiled/driver/out.plan --search-time-limit 1000s transpiled/driver/domain.pddl transpiled/driver/problem.pddl

