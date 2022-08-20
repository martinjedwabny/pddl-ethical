#!/usr/bin/env python

import itertools


class EthicalFeature:
    def __init__(self, name, arguments):
        self.name = name
        self.arguments = arguments

    def __str__(self):
        return '{EthicalFeature: ' + self.name + ' ' + str(self.arguments) + '}'

    def __repr__(self):
        return str(self)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class EthicalRank:
    def __init__(self, feature, type, rank):
        self.feature = feature
        self.type = type
        self.rank = rank

    def __str__(self):
        return '{EthicalRank:' + \
            '\n  feature: ' + str(self.feature) + \
            '\n  type: ' + str(self.type) + \
            '\n  rank: ' + str(self.rank) + '}\n'

    def __repr__(self):
        return str(self)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class EthicalRule:
    def __init__(self, name, parameters, positive_preconditions, negative_preconditions, activation, positive_features, negative_features):
        self.name = name
        self.parameters = parameters
        self.positive_preconditions = positive_preconditions
        self.negative_preconditions = negative_preconditions
        self.activation = activation
        self.positive_features = positive_features
        self.negative_features = negative_features

    def __str__(self):
        return '{EthicalRule: ' + self.name + \
            '\n  parameters: ' + str(self.parameters) + \
            '\n  positive_preconditions: ' + str(self.positive_preconditions) + \
            '\n  negative_preconditions: ' + str(self.negative_preconditions) + \
            '\n  activation: ' + str(self.activation) + \
            '\n  positive_features: ' + str(self.positive_features) + \
            '\n  negative_features: ' + str(self.negative_features) + '}\n'

    def __repr__(self):
        return str(self)

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
