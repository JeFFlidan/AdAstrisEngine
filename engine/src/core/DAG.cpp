#include "DAG.h"

using namespace ad_astris;

DAG::DAG(uint32_t vertexNumber) : _vertexNumber(vertexNumber)
{
	_adjacencyListsArray.resize(_vertexNumber);
}

void DAG::add_edge(uint32_t from, uint32_t to)
{
	_adjacencyListsArray[from].push_back(to);
}

void DAG::topological_sort(std::vector<uint32_t>& sortedGraph)
{
	std::stack<uint32_t> sortedValues;
	std::vector<bool> visited(_vertexNumber, false);

	for (uint32_t i = 0; i != _vertexNumber; ++i)
	{
		if (visited[i] == false)
		{
			topological_sort_util(i, visited, sortedValues);
		}
	}

	while (!sortedValues.empty())
	{
		sortedGraph.push_back(sortedValues.top());
		sortedValues.pop();
	}
}

void DAG::topological_sort_util(uint32_t val, std::vector<bool>& visited, std::stack<uint32_t>& sortedValues)
{
	visited[val] = true;

	std::list<uint32_t>& list = _adjacencyListsArray[val];
	for (auto i = list.begin(); i != list.end(); ++i)
	{
		if (!visited[*i])
		{
			topological_sort_util(*i, visited, sortedValues);
		}
	}

	sortedValues.push(val);
}
