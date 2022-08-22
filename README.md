# Ethical PDDL

Extension of PDDL with ethical rules.

## Directories:
- benchmarks: experimentation scripts.
- domains: experimentation domains with ethical rules and transformed versions.
- transpiler_preferences: compiler from ethical rules to PDDL preferences.
- transpiler_costs: compiler from ethical rules to PDDL action costs.
- downward: FD action cost PDDL planner [Link](https://www.fast-downward.org/).

## Run instructions:

- transpiler: 

        python transpiler_costs/generator.py <DOMAIN-FILE> <PROBLEM-FILE> <OUT-DOMAIN-FILE> <OUT-PROBLEM-FILE>
    
    Example:

        python transpiler_costs/generator.py domains/custom/domain.pddl domains/custom/problem.pddl transpiled/custom/domain.pddl transpiled/custom/problem.pddl

- planner:

        ./downward/fast-downward.py --alias seq-opt-fdss-1 --plan-file <OUT-FILE> --search-time-limit <TIME-LIMIT> <DOMAIN-FILE> <PROBLEM-FILE>
    
    Example:

        ./downward/fast-downward.py --alias seq-opt-fdss-1 --plan-file plans/custom/0/sas_plan --search-time-limit 1000s transpiled/custom/domain.pddl transpiled/custom/problem.pddl

