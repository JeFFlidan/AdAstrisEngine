#pragma once

#include <vector>
#include <list>
#include <stack>
#include <cstdint>

namespace ad_astris
{
	// Directed Acyclic Graph. Realization from: https://www.geeksforgeeks.org/topological-sorting/
	class DAG
	{
		public:
			DAG(uint32_t vertexNumber);

			void add_edge(uint32_t from, uint32_t to);

			void topological_sort(std::vector<uint32_t>& sortedGraph);

		private:
			uint32_t _vertexNumber;
			std::vector<std::list<uint32_t>> _adjacencyListsArray;

			void topological_sort_util(
				uint32_t val,
				std::vector<bool>& visited,
				std::stack<uint32_t>& sortedValues);
	};
}