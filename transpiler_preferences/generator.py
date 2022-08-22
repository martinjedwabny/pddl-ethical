#!/usr/bin/env python

from functools import reduce
from PDDL import PDDL_Parser
from action import Action

SPACE = '    '


class Generator:

    def generate_init(self, atoms):
        ans = ''
        atoms.pop(0)
        for atom in atoms:
            ans += '\n' + self.generate_atom(atom)
        return ans

    def generate_goal(self, goal, pref_goals):
        old = goal[1]
        goal = self.generate_formula(self.merge_and(old, ['check']))
        return "(:goal {} \n)\n\n".format(goal[:-3]+pref_goals[:-1]+'\n)')

    def generate_preference_goals(self, parser):
        ans = ''
        for er in parser.ethical_ranks:
            s = self.ground_feature_string(er.feature)
            f = self.ground_feature_string(er.feature, ' ')
            if er.type == "+":
                ans += "(preference p_{} ({}))\n".format(s, s)
            else:
                ans += "(preference p_{} (not ({})))\n".format(s, f)
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
        if ":preferences" not in parser.requirements:
            text += " " + ":preferences"
        if ":ethical" in parser.requirements:
            parser.requirements.remove(":ethical")
        for req in parser.requirements:
            text += " " + req

        text += ")\n\n"

        text += "(:types\n"
        for k, v in parser.types.items():
            text += SPACE
            for vs in v:
                text += vs+' '
            text += ' - {} \n'.format(k)

        text += ")\n\n"

        text += "(:constants\n"
        for k, v in parser.constants.items():
            text += SPACE
            for vs in v:
                text += vs+' '
            text += ' - {} \n'.format(k)

        text += ")\n\n"

        text += "(:predicates\n"

        parser.predicates['check'] = {}

        for pred, args in parser.predicates.items():
            text += SPACE + "("+pred+' '+self.generate_arguments(args)+")\n"

        for feature in parser.ethical_features:
            text += SPACE + "("+feature.name+' ' + \
                self.generate_arguments(feature.arguments)+")\n"

        text += ")\n\n"

        parser.actions.append(self.generate_check_action(parser))

        for act in parser.actions:
            text += self.generate_transformed_action(parser, act)

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

    def generate_transformed_action(self, parser, act):
        text = "(:action " + act.name + "\n"

        text += SPACE + ":parameters " + \
            self.generate_arguments_list_of_pairs(act.parameters) + "\n"

        text += SPACE + ":precondition "

        if act.name.lower() != 'checkop':
            act.preconditions = self.merge_and(act.preconditions, ['check'])

        text += self.generate_formula(act.preconditions) + "\n"

        text += SPACE + ":effect "

        new_effects = []

        if act.name.lower() != 'checkop':
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

    def generate_check_action(self, parser):
        name = 'checkOp'
        params = []
        precondition = ['not', ['check']]
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
        elif formula[0] == 'when':
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

        text += "(:objects\n"
        for k, v in parser.objects.items():
            text += SPACE
            for vs in v:
                text += vs+' '
            text += ' - {} \n'.format(k)

        text += ")\n\n"

        text += "(:init " + self.generate_init(parser.state) + ")\n\n"

        pref_goals = self.generate_preference_goals(parser)
        goal = self.generate_goal(parser.goal, pref_goals)

        text += goal

        text += self.generate_valuations(parser)

        text += ")\n"

        return text

    def generate_valuations(self, parser):
        ans = "(:metric minimize (+\n"
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
            ans += "(* (is-violated p_" + self.ground_feature_string(r.feature) + ") " + \
                str(val[r.rank]) + ")\n"
        ans += ")\n)\n"
        return ans


if __name__ == '__main__':
    import sys
    import os
    domain = sys.argv[1]
    problem = sys.argv[2]
    n_rules = sys.argv[3]

    new_domain = os.path.dirname(os.path.realpath(
        domain)) + "/" + n_rules + "/" + os.path.basename(domain)
    new_problem = os.path.dirname(os.path.realpath(
        problem)) + "/" + n_rules + "/" + os.path.basename(problem)

    os.makedirs(os.path.dirname(new_domain), exist_ok=True)

    gen = Generator()
    parser = PDDL_Parser()
    parser.parse_domain(domain, n_rules)
    parser.parse_problem(problem)

    with open(new_domain, 'w') as fd:
        fd.write(gen.generate_domain_file(parser))

    with open(new_problem, 'w') as fp:
        fp.write(gen.generate_problem_file(parser))
