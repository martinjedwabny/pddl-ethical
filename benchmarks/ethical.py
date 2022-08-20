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
    def __init__(self, name, parameters, preconditions, activation, features):
        self.name = name
        self.parameters = parameters
        self.preconditions = preconditions
        self.activation = activation
        self.features = features

    def __str__(self):
        return '{EthicalRule: ' + self.name + \
            '\n  parameters: ' + str(self.parameters) + \
            '\n  preconditions: ' + str(self.preconditions) + \
            '\n  activation: ' + str(self.activation) + \
            '\n  features: ' + str(self.features) + '}\n'

    def __repr__(self):
        return str(self)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__
