# !/usr/bin/env python
# Author: Everett Harding
# Creation Date: 7/28/2017
# Algorithm for determining stark service areas
from random import randrange, choices

class ZipCode:
	numCreatedZips = 0
	def __init__(self, neighbors, patientPercentage):
		self.neighbors = neighbors
		self.id = ZipCode.numCreatedZips
		self.patientPercentage = patientPercentage
		ZipCode.numCreatedZips = ZipCode.numCreatedZips + 1 

	def __str__(self):
		nstr = ''
		for n in self.neighbors:
			nstr = nstr + str(n.id) + ', '
		return str(self.id) + ': ' + str(self.patientPercentage) + '% ; ' + nstr
		



class Graph: 

	def __init__(self, listOfZips):
		self.listOfZips  = listOfZips
		self.graph = {}
		self.initializeGraph(self.listOfZips)

	def initializeGraph(self, listOfZips):
		for zipCode in listOfZips:
			if zipCode not in self.graph:
				self.graph[zipCode] = zipCode.neighbors

	def hasPath(self, fromHere, toThere, visited):
		visited.append(fromHere)
		if toThere in fromHere.neighbors:
			return True

		for n in fromHere.neighbors:
			if n in visited:
				continue
			if self.hasPath(n, toThere, visited):
				return True

		return False

	def bfs(self, fromHere, toThere, visited):



def generateZipCodes(numZips):
	totalPercent = 0
	listOfZips = []

	# create a bunch of zip codes, assign each a random percent (no neighbors yet)
	for x in range(numZips):
		thisPercent = randrange(100 * numZips)
		newZip = ZipCode([], thisPercent)
		listOfZips.append(ZipCode([], thisPercent))
		totalPercent = totalPercent + thisPercent

	for zipCode in listOfZips:
		zipCode.patientPercentage = float(zipCode.patientPercentage)/totalPercent * 100	
	return listOfZips

def addNeighbors(listOfZips):
	for zipCode in listOfZips:
		myk = randrange(5) + 1
		neighbors = choices(listOfZips, k=myk)
		if zipCode in neighbors:
			neighbors.remove(zipCode)
		zipCode.neighbors = neighbors
			


loz = generateZipCodes(50)
addNeighbors(loz)
g = Graph(loz)

print('Graph:')
for key in g.graph:
	print(key, end = ':: ')
	for n in g.graph[key]:
		print(str(n.id), end=', ')
	print()

print(g.hasPath(loz[1], loz[20], []));
