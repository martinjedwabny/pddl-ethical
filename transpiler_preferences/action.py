#!/usr/bin/env python
# Four spaces as indentation [no tabs]

import itertools


class Action:

    def __init__(self, name, parameters, preconditions, effects):
        self.name = name
        self.parameters = parameters
        self.preconditions = preconditions
        self.effects = effects

    def __str__(self):
        return 'action: ' + self.name + \
            '\n  parameters: ' + str(self.parameters) + \
            '\n  preconditions: ' + str(self.preconditions) + \
            '\n  effects: ' + str(self.effects) + '\n'

    def __eq__(self, other):
        return self.__dict__ == other.__dict__

    def groundify(self, objects):
        if not self.parameters:
            yield self
            return
        type_map = []
        variables = []
        for var, type in self.parameters:
            type_map.append(objects[type])
            variables.append(var)
        for assignment in itertools.product(*type_map):
            preconditions = self.replace(
                self.preconditions, variables, assignment)
            effects = self.replace(self.effects, variables, assignment)
            yield Action(self.name, assignment, preconditions, effects)

    def replace(self, group, variables, assignment):
        g = []
        for pred in group:
            pred = list(pred)
            iv = 0
            for v in variables:
                while v in pred:
                    pred[pred.index(v)] = assignment[iv]
                iv += 1
            g.append(pred)
        return g
