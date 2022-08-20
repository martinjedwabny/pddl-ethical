#!/usr/bin/env python

from PDDL import PDDL_Parser

SPACE = '    '


class Generator:

    def find_relevant_rules(self, parser, action):
        rules = []
        for rule in parser.ethical_rules:
            if rule.activation[0] == action:
                rules.append(rule)
        return rules

    def format_conjunction(self, pos_elems, neg_elems):
        text = ""
        if not (pos_elems or neg_elems):
            text = "()"
        elif len(pos_elems) + len(neg_elems) > 1:
            print(pos_elems)
            text = "(and"
            for pos in pos_elems:
                # TODO : awful there
                # text += " ("+pos[0]+")"
                text += " ("
                for p in pos:
                    text += " " + str(p)
                text += ")"
            for neg in neg_elems:
                # text += " (not("+neg[0]+"))"
                text += " ("
                for n in neg:
                    text += " not(" + n + ")"
                text += ")"
            text += ")"
        else:
            if not pos_elems:
                # TODO : awful there again
                # text += "(not("+neg_elems[0][0]+"))"
                text += " ("
                for n in neg_elems[0]:
                    text += " not(" + n + ")"
                text += ")"
            else:
                # text += "("+pos_elems[0][0]+")"
                text += " ("
                for p in pos_elems[0]:
                    text += " " + p
                text += ")"

        return text

    def generate_init(self, atoms):
        ans = ''
        atoms.pop(0)
        for atom in atoms:
            ans += '\n' + self.generate_atom(atom)
        return ans

    def generate_goal(self, goal):
        goal.pop(0)
        return self.generate_formula(goal.pop(0))

    def generate_preference_goals(self, parser):
        pref_list = []
        # print(parser.ethical_rules)
        for rule in parser.ethical_rules:
            # TODO there is something wrong here: what should be the fluent to add? Precondition can be empty and name does not correspond
            # if rule.ethical_type[0] == "+":
            #     pref_list.append(["preference", "p_" + rule.name, self.format_conjunction(rule.name, [])])
            # else:
            #     pref_list.append(["preference", "p_" + rule.name, self.format_conjunction([],rule.name)])
            name = []
            # TODO WE NEED NEGATIVE PRECOND FOR ETHICS
            if rule.activation[0] == "final":
                name = rule.preconditions
            else:
                name = [[rule.name]]
            if rule.ethical_type[0] == "+":
                pref_list.append(
                    ["preference", "p_" + rule.name, " "+self.format_conjunction(name, [])+" "])
            else:
                pref_list.append(
                    ["preference", "p_" + rule.name, ""+self.format_conjunction([], name)+" "])
        return pref_list

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
        for pred, args in parser.predicates.items():
            text += SPACE + "("+pred+' '+self.generate_arguments(args)+")\n"

        for feature in parser.ethical_features:
            text += SPACE + "("+feature.name+' ' + \
                self.generate_arguments(feature.arguments)+")\n"

        text += ")\n\n"

        for act in parser.actions:
            text += "(:action " + act.name + "\n"

            text += SPACE + ":parameters " + \
                self.generate_arguments_list_of_pairs(act.parameters) + "\n"

            text += SPACE + ":precondition "
            text += self.generate_formula(act.preconditions) + "\n"

            text += SPACE + ":effect "
            text += self.generate_formula(act.effects) + "\n"
            # rules = self.find_relevant_rules(parser, act.name)
            # for rule in rules:
            #     if not rule.preconditions:
            #         act.add_effects.extend([[rule.name]])
            #     else:
            #         act.add_effects.extend([["when"+self.format_conjunction(
            #             rule.preconditions, []) + self.format_conjunction([[rule.name]], [])]])

            text += "\n)\n"

        text += "\n)\n"
        return text

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
        while len(params) > 0:
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

        text += "(:goal " + self.generate_goal(parser.goal) + ")\n\n"
        # pref_goals = self.generate_preference_goals(parser)
        # pos_goals = []
        # pos_goals.extend(parser.positive_goals)
        # pos_goals.extend(pref_goals)
        # # print(pos_goals)
        # text += self.format_conjunction(pos_goals,
        #                                 parser.negative_goals) + ")\n"
        # text += "(:metric minimize (+\n"
        # for rule in parser.ethical_rules:
        #     text += "(* (is-violated p_" + rule.name + ") " + \
        #         rule.rank[0] + ")\n"
        # text += "))\n"

        text += ")\n"

        return text


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
