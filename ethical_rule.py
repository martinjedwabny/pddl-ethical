#!/usr/bin/env python

import itertools

class Ethical_rule:

    def __init__(self, name, ethical_type, preconditions, activation, rank):
        self.name = name
        self.ethical_type = ethical_type
        self.preconditions = preconditions
        self.activation = activation
        self.rank = rank

    def __str__(self):
        return 'ethical rule: ' + self.name + \
        '\n  ethical_type: ' + str(self.ethical_type) + \
        '\n  preconditions: ' + str(self.preconditions) + \
        '\n  activation: ' + str(self.activation) + \
        '\n  rank: ' + str(self.rank) + '\n'

    def __eq__(self, other): 
        return self.__dict__ == other.__dict__

    # def groundify(self, objects):
    #     if not self.parameters:
    #         yield self
    #         return
    #     type_map = []
    #     variables = []
    #     for var, type in self.parameters:
    #         type_map.append(objects[type])
    #         variables.append(var)
    #     for assignment in itertools.product(*type_map):
    #         positive_preconditions = self.replace(self.positive_preconditions, variables, assignment)
    #         negative_preconditions = self.replace(self.negative_preconditions, variables, assignment)
    #         add_effects = self.replace(self.add_effects, variables, assignment)
    #         del_effects = self.replace(self.del_effects, variables, assignment)
    #         yield Action(self.name, assignment, positive_preconditions, negative_preconditions, add_effects, del_effects)

    # def replace(self, group, variables, assignment):
    #     g = []
    #     for pred in group:
    #         pred = list(pred)
    #         iv = 0
    #         for v in variables:
    #             while v in pred:
    #                 pred[pred.index(v)] = assignment[iv]
    #             iv += 1
    #         g.append(pred)
    #     return g

# if __name__ == '__main__':
#     a = Action('move', [['?ag', 'agent'], ['?from', 'pos'], ['?to', 'pos']],
#         [['at', '?ag', '?from'], ['adjacent', '?from', '?to']],
#         [['at', '?ag', '?to']],
#         [['at', '?ag', '?to']],
#         [['at', '?ag', '?from']]
#     )
#     print(a)

#     objects = {
#         'agent': ['ana','bob'],
#         'pos': ['p1','p2']
#     }
#     for act in a.groundify(objects):
#         print(act)
