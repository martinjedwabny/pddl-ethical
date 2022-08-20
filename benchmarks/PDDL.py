#!/usr/bin/env python
# Reuse large parts of https://github.com/pucrs-automated-planning/pddl-parser
# Let them be thanked

import re
from action import Action
from ethical import EthicalRule, EthicalFeature, EthicalRank
import random


class PDDL_Parser:

    SUPPORTED_REQUIREMENTS = [
        ':strips', ':negative-preconditions', ':equality', ':typing', ":conditional-effects", ':ethical']

    # ------------------------------------------
    # Tokens
    # ------------------------------------------

    def scan_tokens(self, filename):
        with open(filename, 'r') as f:
            # Remove single line comments
            str = re.sub(r';.*$', '', f.read(), flags=re.MULTILINE).lower()
        # Tokenize
        stack = []
        list = []
        for t in re.findall(r'[()]|[^\s()]+', str):
            if t == '(':
                stack.append(list)
                list = []
            elif t == ')':
                if stack:
                    l = list
                    list = stack.pop()
                    list.append(l)
                else:
                    raise Exception('Missing open parentheses')
            else:
                list.append(t)
        if stack:
            raise Exception('Missing close parentheses')
        if len(list) != 1:
            raise Exception('Malformed expression')
        return list[0]

    # -----------------------------------------------
    # Parse domain
    # -----------------------------------------------

    def parse_domain(self, domain_filename, n_rules):
        tokens = self.scan_tokens(domain_filename)
        if type(tokens) is list and tokens.pop(0) == 'define':
            self.domain_name = 'unknown'
            self.requirements = []
            self.types = []
            self.actions = []
            self.predicates = {}
            self.constants = {}
            self.ethical_features = []
            self.ethical_ranks = []
            self.ethical_rules = []
            while tokens:
                group = tokens.pop(0)
                t = group.pop(0)
                if t == 'domain':
                    self.domain_name = group[0]
                elif t == ':requirements':
                    for req in group:
                        if not req in self.SUPPORTED_REQUIREMENTS:
                            raise Exception('Requirement ' +
                                            req + ' not supported')
                    self.requirements = group
                elif t == ':predicates':
                    self.predicates = self.parse_predicates(group)
                elif t == ':types':
                    self.types = group
                elif t == ':action':
                    self.parse_action(group)
                elif t == ':ethical-features':
                    self.ethical_features = self.parse_ethical_features(group)
                elif t == ':ethical-rank':
                    self.parse_ethical_rank(group)
                elif t == ':ethical-rule':
                    self.parse_ethical_rule(group)
                elif t == ':constants':
                    self.constants = self.parse_constants(group)
                else:
                    print(str(t) + ' is not recognized in domain')
            # for i in range(0, int(n_rules)):
            #     self.add_random_ethical_rule('r'+str(i), i, int(n_rules))
        else:
            raise Exception('File ' + domain_filename +
                            ' does not match domain pattern')

    # def add_random_ethical_rule(self, name, r, n):
    #     random.seed(r+2)
    #     num1 = random.randint(0, 1)
    #     num3 = r > (n / 2)
    #     num4 = random.randint(1, 5)
    #     ethical_type = ['+'] if num1 == 0 else ['-']
    #     positive_preconditions = [] if num3 == 1 else [
    #         random.sample(self.predicates.keys(), 1)]
    #     negative_preconditions = []
    #     activation = ['final'] if num3 == 0 else [
    #         random.sample(self.actions, 1)[0].name]
    #     rank = [str(num4)]
    #     # print(name)
    #     # print(ethical_type)
    #     # print(positive_preconditions)
    #     # print(activation)
    #     # print(rank)
    #     self.ethical_rules.append(EthicalRule(
    #         name, ethical_type, positive_preconditions, activation, rank))

    # -----------------------------------------------
    # Parse constants
    # -----------------------------------------------

    def parse_constants(self, group):
        ans = {}
        constant_list = []
        while group:
            if group[0] == '-':
                group.pop(0)
                ans[group.pop(0)] = constant_list
                constant_list = []
            else:
                constant_list.append(group.pop(0))
        if constant_list:
            if not 'object' in ans:
                ans['object'] = []
            ans['object'] += constant_list
        return ans

    # -----------------------------------------------
    # Parse predicates
    # -----------------------------------------------

    def parse_predicates(self, group):
        ans = {}
        for pred in group:
            predicate_name = pred.pop(0)
            if predicate_name in ans:
                raise Exception('Predicate ' + predicate_name + ' redefined')
            arguments = {}
            untyped_variables = []
            while pred:
                t = pred.pop(0)
                if t == '-':
                    if not untyped_variables:
                        raise Exception('Unexpected hyphen in predicates')
                    type = pred.pop(0)
                    while untyped_variables:
                        arguments[untyped_variables.pop(0)] = type
                else:
                    untyped_variables.append(t)
            while untyped_variables:
                arguments[untyped_variables.pop(0)] = 'object'
            ans[predicate_name] = arguments
        return ans

    # -----------------------------------------------
    # Parse action
    # -----------------------------------------------

    def parse_action(self, group):
        name = group.pop(0)
        if not type(name) is str:
            raise Exception('Action without name definition')
        for act in self.actions:
            if act.name == name:
                raise Exception('Action ' + name + ' redefined')
        parameters = []
        preconditions = []
        effects = []
        while group:
            t = group.pop(0)
            if t == ':parameters':
                parameters = self.parse_parameters(group)
            elif t == ':precondition':
                preconditions = group.pop(0)
            elif t == ':effect':
                effects = group.pop(0)
            else:
                print(str(t) + ' is not recognized in action')
        self.actions.append(Action(name, parameters, preconditions, effects))

    def parse_parameters(self, group):
        if not type(group) is list:
            raise Exception('Error with parameters')
        parameters = []
        untyped_parameters = []
        p = group.pop(0)
        while p:
            t = p.pop(0)
            if t == '-':
                if not untyped_parameters:
                    raise Exception(
                        'Unexpected hyphen in parameters')
                ptype = p.pop(0)
                while untyped_parameters:
                    parameters.append(
                        [untyped_parameters.pop(0), ptype])
            else:
                untyped_parameters.append(t)
        while untyped_parameters:
            parameters.append([untyped_parameters.pop(0), 'object'])
        return parameters

    # -----------------------------------------------
    # Parse ethical features
    # -----------------------------------------------

    def parse_ethical_features(self, group):
        ans = []
        names = set()
        for pred in group:
            feature = self.parse_ethical_feature(pred)
            if feature.name in names:
                raise Exception('Ethical feature ' +
                                feature.name + ' redefined')
            else:
                names.add(feature.name)
                ans.append(feature)
        return ans

    def parse_ethical_feature(self, pred):
        predicate_name = pred.pop(0)
        arguments = {}
        untyped_variables = []
        while pred:
            t = pred.pop(0)
            if t == '-':
                if not untyped_variables:
                    raise Exception('Unexpected hyphen in predicates')
                type = pred.pop(0)
                while untyped_variables:
                    arguments[untyped_variables.pop(0)] = type
            else:
                untyped_variables.append(t)
        while untyped_variables:
            arguments[untyped_variables.pop(0)] = 'object'
        return EthicalFeature(predicate_name, arguments)

    # -----------------------------------------------
    # Parse ethical ranks
    # -----------------------------------------------

    def parse_ethical_rank(self, group):
        while group:
            t = group.pop(0)
            if t == ':type':
                type = group.pop(0)
            elif t == ':feature':
                feature = self.parse_ethical_feature(group.pop(0))
            elif t == ':rank':
                rank = int(group.pop(0))
            else:
                print(str(t) + ' is not recognized in ethical rank')
        if type and feature and rank:
            self.ethical_ranks.append(EthicalRank(feature, type, rank))
        else:
            print('Ethical rank missing input')

    # -----------------------------------------------
    # Parse ethical rules
    # -----------------------------------------------

    def parse_ethical_rule(self, group):
        name = group.pop(0)
        if not type(name) is str:
            raise Exception('Ethical rule without name definition')
        for rule in self.ethical_rules:
            if rule.name == name:
                raise Exception('Ethical rule ' + name + ' redefined')
        parameters = []
        preconditions = []
        activation = []
        features = []
        while group:
            t = group.pop(0)
            if t == ':parameters':
                parameters = self.parse_parameters(group)
            elif t == ':activation':
                act = group.pop(0)
                if act == 'null':
                    activation.append(act)
                else:
                    activation.append(act.pop(0))
                    activation.append(self.parse_parameters(act))
            elif t == ':precondition':
                preconditions = group.pop(0)
            elif t == ':features':
                features = group.pop(0)
            else:
                print(str(t) + ' is not recognized in action')
        self.ethical_rules.append(EthicalRule(name, parameters, preconditions,
                                              activation, features))

    # -----------------------------------------------
    # Parse problem
    # -----------------------------------------------

    def parse_problem(self, problem_filename):
        tokens = self.scan_tokens(problem_filename)
        if type(tokens) is list and tokens.pop(0) == 'define':
            self.problem_name = 'unknown'
            self.objects = dict()
            self.state = []
            self.goals = []
            while tokens:
                group = tokens.pop(0)
                t = group[0]
                if t == 'problem':
                    self.problem_name = group[-1]
                elif t == ':domain':
                    if self.domain_name != group[-1]:
                        raise Exception(
                            'Different domain specified in problem file')
                elif t == ':requirements':
                    pass  # Ignore requirements in problem, parse them in the domain
                elif t == ':objects':
                    group.pop(0)
                    object_list = []
                    while group:
                        if group[0] == '-':
                            group.pop(0)
                            self.objects[group.pop(0)] = object_list
                            object_list = []
                        else:
                            object_list.append(group.pop(0))
                    if object_list:
                        if not 'object' in self.objects:
                            self.objects['object'] = []
                        self.objects['object'] += object_list
                elif t == ':init':
                    self.state = group.pop(0)
                elif t == ':goal':
                    self.goals = group.pop(0)
                else:
                    print(str(t) + ' is not recognized in problem')
        else:
            raise Exception('File ' + problem_filename +
                            ' does not match problem pattern')


# ==========================================
# Main
# ==========================================
if __name__ == '__main__':
    import sys
    import pprint
    domain = sys.argv[1]
    problem = sys.argv[2]
    parser = PDDL_Parser()
    print('----------------------------')
    pprint.pprint(parser.scan_tokens(domain))
    print('----------------------------')
    pprint.pprint(parser.scan_tokens(problem))
    print('----------------------------')
    parser.parse_domain(domain, 0)
    parser.parse_problem(problem)

    # print('Domain name: ' + parser.domain_name)
    # print('Constants: ' + str(parser.constants))
    # for act in parser.actions:
    #     print(act)

    print('Ethical features: ' + str(parser.ethical_features))
    print('Ethical ranks: \n' + str(parser.ethical_ranks))
    print('Ethical rules: \n' + str(parser.ethical_rules))
    print('----------------------------')

    # print('Problem name: ' + parser.problem_name)
    # print('Objects: ' + str(parser.objects))
    # print('State: ' + str(parser.state))
    # print('Positive goals: ' + str(parser.positive_goals))
    # print('Negative goals: ' + str(parser.negative_goals))
