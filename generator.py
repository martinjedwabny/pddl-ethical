#!/usr/bin/env python

from PDDL import PDDL_Parser

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
            text = "(and"
            for pos in pos_elems:
                #TODO : awful there
                # text += " ("+pos[0]+")"
                text += " ("
                for p in pos:
                    text += " " + p
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
                #TODO : awful there again
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

    def generate_preference_goals(self, parser):
        pref_list = []
        for rule in parser.ethical_rules:
            #TODO there is something wrong here: what should be the fluent to add? Precondition can be empty and name does not correspond
            # if rule.ethical_type[0] == "obligation":
            #     pref_list.append(["preference", "p_" + rule.name, self.format_conjunction(rule.name, [])])
            # else:
            #     pref_list.append(["preference", "p_" + rule.name, self.format_conjunction([],rule.name)])
            name = []
            #TODO WE NEED NEGATIVE PRECOND FOR ETHICS
            if rule.activation[0] == "final":
                name = rule.preconditions
            else:
                name = [[rule.name]]
            if rule.ethical_type[0] == "obligation":
                pref_list.append(["preference", "p_" + rule.name, self.format_conjunction(name, [])])
            else:
                pref_list.append(["preference", "p_" + rule.name, self.format_conjunction([],name)])
        return pref_list



    def generate_domain_file(self, parser, generator):
        text = "(define (domain "+parser.domain_name+"_GEN)\n"

        text += "(:requirements"
        if ":conditional-effects" not in parser.requirements:
            text += " " + ":conditional-effects" 
        if ":ethical-rules" in parser.requirements:
            parser.requirements.remove(":ethical-rules")
        for req in parser.requirements:
            text += " " + req

        text += ")\n\n(:predicates\n"
        for pred in parser.predicates:
            text += "("+pred+")\n"

        for rule in parser.ethical_rules:
            #Warning: when the action is final, nothing is added
            if rule.activation[0] != "final":
                text += "("+rule.name+")\n"

        text += ")\n\n"

        for act in parser.actions:
            text += "(:action "+act.name+"\n:parameters "
            #TODO not sure it works as intended for parameters, but oh well...
            text += generator.format_conjunction(act.parameters,[])

            text += "\n:precondition "
            text += generator.format_conjunction(act.positive_preconditions, act.negative_preconditions)

            text += "\n:effect "
            rules = generator.find_relevant_rules(parser, act.name)
            for rule in rules:
                if not rule.preconditions:
                    act.add_effects.extend([[rule.name]])
                else:
                    act.add_effects.extend([["when"+generator.format_conjunction(rule.preconditions,[]) + generator.format_conjunction( [[rule.name]],[])]])

            text += generator.format_conjunction(act.add_effects, act.del_effects)

            text += "\n)\n"

        text += "\n)\n"
        return text

    def generate_problem_file(self, parser, generator):
        text = "(define (problem "+parser.problem_name+"_GEN)\n"
        text += "(:domain " + parser.domain_name + ")\n"
        text += "(:init" + generator.format_conjunction(parser.state, []) + ")\n"
        text += "(:goal "
        pref_goals = self.generate_preference_goals(parser)
        pos_goals = []
        pos_goals.extend(parser.positive_goals)
        pos_goals.extend(pref_goals)
        # print(pos_goals)
        text += generator.format_conjunction(pos_goals,parser.negative_goals) + ")\n"
        text += "(:metric maximize (+\n"
        for rule in parser.ethical_rules:
            text += "(* (is-violated p_" + rule.name + ") -" + rule.rank[0] + ")\n"
        text += "))\n"

        text += ")\n"

        return text


if __name__ == '__main__':
    import sys
    import os
    domain = sys.argv[1]
    problem = sys.argv[2]

    new_domain = os.path.dirname(os.path.realpath(domain)) + "/" + os.path.basename(domain).split(".")[0] + "GEN.pddl"
    new_problem = os.path.dirname(os.path.realpath(problem)) + "/" + os.path.basename(problem).split(".")[0] + "GEN.pddl"

    gen = Generator()
    parser = PDDL_Parser()
    parser.parse_domain(domain)
    parser.parse_problem(problem)

    with open(new_domain, 'w') as fd:
        fd.write(gen.generate_domain_file(parser, gen))

    with open(new_problem, 'w') as fp:
        fp.write(gen.generate_problem_file(parser, gen))
