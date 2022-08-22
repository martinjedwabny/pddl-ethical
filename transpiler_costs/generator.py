#!/usr/bin/env python

from functools import reduce
from pathlib import Path
from PDDL import PDDL_Parser
from action import Action

class Generator:

    def generate_init(self, atoms):
        ans = '\n(= (total-cost) 0)'
        atoms.pop(0)
        for atom in atoms:
            ans += '\n' + self.generate_atom(atom)
        return ans

    def generate_goal(self, goal, pref_goals):
        old = goal[1]
        goal = self.generate_formula(self.merge_and(old, ['check']))
        return "(:goal {} \n)\n\n".format(goal[:-3]+pref_goals[:-1]+'\n)')

    def generate_final_mode_goals(self, parser):
        ans = '(final-mode)\n'
        for er in parser.ethical_ranks:
            s = self.ground_feature_string(er.feature)
            ans += "(final-mode-check-{})\n".format(s)
        return ans

    def ground_feature_string(self, feature, sep='-'):
        ans = feature.name
        for arg, targ in feature.arguments.items():
            ans += sep + arg
        return ans

    def generate_domain_file(self, parser):
        text = "(define (domain "+parser.domain_name+"_GEN)\n"

        text += "(:requirements"
        if ":conditional-effects" not in parser.requirements:
            text += " " + ":conditional-effects"
        if ":typing" not in parser.requirements:
            text += " " + ":typing"
        if ":action-costs" not in parser.requirements:
            text += " " + ":action-costs"
        if ":preferences" in parser.requirements:
            parser.requirements.remove(":preferences")
        if ":ethical" in parser.requirements:
            parser.requirements.remove(":ethical")
        for req in parser.requirements:
            text += " " + req

        text += ")\n\n"

        text += "(:types\n"
        for k, v in parser.types.items():
            for vs in v:
                text += vs+' '
            text += ' - {} \n'.format(k)

        text += ")\n\n"

        text += "(:constants\n"
        for k, v in parser.constants.items():
            for vs in v:
                text += vs+' '
            text += ' - {} \n'.format(k)

        text += ")\n\n"

        text += "(:functions (total-cost))\n\n"

        text += "(:predicates\n"

        parser.predicates['check'] = {}

        for pred, args in parser.predicates.items():
            text += "("+pred+' '+self.generate_arguments(args)+")\n"

        for feature in parser.ethical_features:
            text += "("+feature.name+' ' + \
                self.generate_arguments(feature.arguments)+")\n"

        text += "(final-mode)\n"
        for er in parser.ethical_ranks:
            text += "(final-mode-check-" + \
                self.ground_feature_string(er.feature)+")\n"

        text += ")\n\n"

        parser.actions.append(self.generate_check_action(parser))

        for act in parser.actions:
            text += self.generate_transformed_action(parser, act)

        for act in self.generate_final_mode_actions(parser):
            text += self.generate_action(parser, act)

        text += "\n)\n"
        return text

    def merge_and(self, f1, f2):
        if len(f1) > 1 and f1[0] == 'and':
            if len(f2) > 1 and f2[0] == 'and':
                f1.pop(0)
                f2.pop(0)
                return ['and'] + f1 + f2
            else:
                return f1 + [f2]
        else:
            if len(f2) > 1 and f2[0] == 'and':
                return f2 + [f1]
            else:
                return ['and'] + [f1] + [f2]

    def generate_action(self, parser, act):
        text = "(:action " + act.name + "\n"

        text += ":parameters " + \
            self.generate_arguments_list_of_pairs(act.parameters) + "\n"

        text += ":precondition "

        text += self.generate_formula(act.preconditions) + "\n"

        text += ":effect "

        text += self.generate_formula(act.effects) + "\n"

        text += "\n)\n"

        return text

    def generate_transformed_action(self, parser, act):
        text = "(:action " + act.name + "\n"

        text += ":parameters " + \
            self.generate_arguments_list_of_pairs(act.parameters) + "\n"

        text += ":precondition "

        if act.name.lower() != 'check-ethical-features':
            act.preconditions = self.merge_and(act.preconditions, ['and', ['not', ['final-mode']], ['check']])

        text += self.generate_formula(act.preconditions) + "\n"

        text += ":effect "

        new_effects = []

        if act.name.lower() != 'check-ethical-features':
            relevant_rules = list(filter(lambda r: r.activation and len(r.activation) > 0 and r.activation[0].lower(
            ) == act.name.lower(), parser.ethical_rules))
            for er in relevant_rules:
                forall_params = []
                for item in er.parameters:
                    if len(item) > 1:
                        forall_params.extend([item[0], '-', item[1]])
                    else:
                        forall_params.append(item[0])
                forall_precondition = er.preconditions
                forall_features = er.features
                if len(forall_params) > 0:
                    new_effects.append(['forall', forall_params, [
                        'when', forall_precondition, forall_features]])
                else:
                    new_effects.append(
                        ['when', forall_precondition, forall_features])
            if len(new_effects) > 0:
                act.effects = self.merge_and(
                    act.effects, ['and'] + new_effects)
            act.effects = self.merge_and(
                ['and', ['not', ['check']]], act.effects)

        text += self.generate_formula(act.effects) + "\n"

        text += "\n)\n"

        return text

    def generate_final_mode_actions(self, parser):
        ans = [Action('final-mode-start', [], ['not', ['final-mode']], ['final-mode'])]
        max_rank = reduce(lambda m, r: m if m > r.rank else r.rank,
                          parser.ethical_ranks, 0)
        amount = [0 for _ in range(max_rank+1)]
        for r in parser.ethical_ranks:
            amount[r.rank] += 1
        max_val = [0 for _ in range(max_rank+1)]
        val = [0 for _ in range(max_rank+1)]
        for i in range(1, max_rank+1):
            val[i] = max_val[i-1] + 1
            max_val[i] = amount[i] * val[i] + max_val[i-1]
        for r in parser.ethical_ranks:
            g_feature = self.ground_feature_string(r.feature)
            p_feature = self.ground_feature_string(r.feature, ' ')
            if r.type == '-':
                a1 = Action('final-mode-check-op-False-'+g_feature, [],
                            ['and', ['final-mode'], [p_feature], ['not', ['final-mode-check-'+g_feature]]], ['and', ['final-mode-check-'+g_feature], ['increase', ['total-cost'], val[r.rank]]])
                a2 = Action('final-mode-check-op-True-'+g_feature, [],
                            ['and', ['final-mode'], ['not', [p_feature]], ['not', ['final-mode-check-'+g_feature]]], ['final-mode-check-'+g_feature])
            elif r.type == '+':
                a1 = Action('final-mode-check-op-False-'+g_feature, [],
                            ['and', ['final-mode'], [p_feature], ['not', ['final-mode-check-'+g_feature]]], ['final-mode-check-'+g_feature])
                a2 = Action('final-mode-check-op-True-'+g_feature, [],
                            ['and', ['final-mode'], ['not', [p_feature]], ['not', ['final-mode-check-'+g_feature]]], ['and', ['final-mode-check-'+g_feature], ['increase', ['total-cost'], val[r.rank]]])
            ans.append(a1)
            ans.append(a2)
        return ans

    def generate_check_action(self, parser):
        name = 'check-ethical-features'
        params = []
        precondition = ['and', ['not', ['final-mode']], ['not', ['check']]]
        effect = ['and', ['check']]
        for er in parser.ethical_rules:
            if er.activation and er.activation[0] == 'null':
                forall_params = []
                for item in er.parameters:
                    if len(item) > 1:
                        forall_params.extend([item[0], '-', item[1]])
                    else:
                        forall_params.append(item[0])
                forall_precondition = er.preconditions
                forall_features = er.features
                if len(forall_params) > 0:
                    effect.append(['forall', forall_params, [
                        'when', forall_precondition, forall_features]])
                else:
                    effect.append(
                        ['when', forall_precondition, forall_features])
        return Action(name, params, precondition, effect)

    def generate_formula(self, formula):
        ans = ''
        if isinstance(formula, int):
            return str(formula)
        if len(formula) == 0:
            ans = '()'
        elif formula[0] == 'not':
            ans = '(not {})'.format(self.generate_formula(formula[1]))
        elif formula[0] == 'and' or formula[0] == 'or':
            ans = '({}'.format(formula.pop(0))+'\n'
            for sub in formula:
                ans += self.generate_formula(sub) + '\n'
            ans += ') \n'
        elif formula[0] == 'forall' or formula[0] == 'exists':
            op = formula.pop(0)
            args = self.generate_arguments_forall(formula.pop(0))
            sub = self.generate_formula(formula.pop(0))
            ans += '({} {}\n{})'.format(op, args, sub)
        elif formula[0] == 'when' or formula[0] == 'increase':
            op = formula.pop(0)
            cond = self.generate_formula(formula.pop(0))
            sub = self.generate_formula(formula.pop(0))
            ans += '({} {} {})'.format(op, cond, sub)
        else:
            ans = self.generate_atom(formula)
        return ans

    def generate_atom(self, atom):
        pred = atom.pop(0)
        args = self.generate_arguments_list(atom)
        return '({} {})'.format(pred, args)

    def generate_arguments_forall(self, params):
        ans = '('
        while params and len(params) > 0:
            p = params.pop(0)
            if len(params) == 0 or params[0] != '-':
                ans += p + ' '
            else:
                params.pop(0)
                ptype = params.pop(0)
                ans += '{} - {} '.format(p, ptype)
        return ans + ')'

    # list of pairs [obj,type]
    def generate_arguments_list_of_pairs(self, params):
        ans = '('
        while len(params) > 0:
            param = params.pop(0)
            p = param.pop(0)
            ptype = param.pop(0)
            ans += '{} - {} '.format(p, ptype)
        return ans + ')'

    # dict <obj,type>
    def generate_arguments(self, args):
        ans = ''
        for k, v in args.items():
            ans += k + ' - ' + v + ' '
        return ans

    # list of obj
    def generate_arguments_list(self, params):
        ans = ''
        for p in params:
            ans += p + ' '
        return ans

    def generate_problem_file(self, parser):
        text = "(define (problem "+parser.problem_name+"_GEN)\n"

        text += "(:domain " + parser.domain_name+"_GEN )\n"

        text += "(:init " + self.generate_init(parser.state) + ")\n\n"

        pref_goals = self.generate_final_mode_goals(parser)
        goal = self.generate_goal(parser.goal, pref_goals)

        text += goal

        text += self.generate_metric()

        text += ")\n"

        return text

    def generate_metric(self):
        return "(:metric minimize (total-cost))\n"

if __name__ == '__main__':
    import sys
    import os
    domain = sys.argv[1]
    problem = sys.argv[2]
    domain_out = sys.argv[3]
    problem_out = sys.argv[4]

    new_domain = Path(str(Path().parent.resolve()) + \
        "/" + domain_out)
    new_problem = Path(str(Path().parent.resolve()) + \
        "/" + problem_out)

    os.makedirs(os.path.dirname(new_domain), exist_ok=True)
    os.makedirs(os.path.dirname(new_problem), exist_ok=True)

    gen = Generator()
    parser = PDDL_Parser()
    parser.parse_domain(domain)
    parser.parse_problem(problem)

    with open(new_domain, 'w') as fd:
        fd.write(gen.generate_domain_file(parser))

    with open(new_problem, 'w') as fp:
        fp.write(gen.generate_problem_file(parser))
