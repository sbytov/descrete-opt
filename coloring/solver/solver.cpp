// solver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stack>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <chrono>
#include <sstream>

struct Graph {
    Graph(int n, std::vector< std::pair<int, int> > edges) {
        adj.resize(n);
        for (auto& e : edges)
        {
            adj[e.first].push_back(e.second);
            adj[e.second].push_back(e.first);
        }
    }
    std::vector<std::vector<int> > adj;
};


int g_c = 0;

struct Solver {
private:
    Graph m_g;

    struct comparator {
        Graph& m_g;
        std::vector<std::unordered_map<int, int> >& m_constraints;

        comparator(
            Graph& g,
            std::vector<std::unordered_map<int, int> >& constraints)
            : m_constraints{ constraints }, m_g{ g }
        {
        }

        bool operator() (int n1, int n2) const
        {
            //std::cout << "cmp " << n1 << " and " << n2 << " = ";
            if (m_constraints[n1].size() != m_constraints[n2].size())
            {
                //std::cout << true << std::endl;
                return m_constraints[n1].size() > m_constraints[n2].size();
            }
            if (m_g.adj[n1].size() != m_g.adj[n2].size())
            {
                //std::cout << true << std::endl;
                return m_g.adj[n1].size() > m_g.adj[n2].size();
            }
            //std::cout << (n1>n2) << std::endl;
            return n1 > n2;
        }
    };

    std::unique_ptr<std::set<int, comparator > > m_nodes;
    std::vector<std::unordered_map<int, int> > m_constraints;
    std::vector<int> m_colors;
    std::vector<int> m_solution;
    int m_cmax = std::numeric_limits<int>::max();

    int color(int n, int cmax, int& counter, int level) 
    {
        if (n == -1)
        {
#ifdef _DEBUG
            std::cout << "done coloring with cmax " << cmax << std::endl; 
#endif
            if (m_cmax > cmax)
            {
                m_cmax = cmax;
                m_solution = m_colors;
            }
            g_c++;
            return cmax;
        }
        auto affected = m_nodes->erase(n);
        int prev_c = -1;
        int cres = -1;
        std::string indent = std::string(level, '.');
        for (int c = cmax; c >= 1; c--)
        {
            if (m_constraints[n].count(c) > 0)
            {
                continue;
            }
#ifdef _DEBUG
            std::cout << indent.c_str() << "color " << n << " into " << c << std::endl;
#endif
            update_constraint(n, c, prev_c);
            prev_c = c;
            int depth = 0;
            cres = color(next(), cmax, ++depth, level + 1);
            counter += depth;
            log_cres(c, cres, cmax, n, indent, false, constr);
            if (cres == cmax)
            {
                break;
            }


            //if (depth > 1000000) {
            //    counter = 0;
            //    break;
            //}
            //remove_constraint(n, c);
        }
        if (cmax + 1 < m_cmax)
        {
#ifdef _DEBUG
            std::cout << indent.c_str() << "color " << n << " into " << cmax + 1 << " (max)" << std::endl;
#endif
            update_constraint(n, cmax + 1, prev_c);
            prev_c = cmax + 1;
            int depth = 0;
            auto nx = next();
            std::unordered_map<int, int> constr;
            if (nx != -1) constr = m_constraints[nx];
            cres = color(nx, cmax + 1, ++depth, level+1);
            log_cres(cmax+1, cres, cmax, n, indent, true, constr);
            counter += depth;
        }
        if (prev_c != -1) remove_constraint(n, prev_c);
        m_nodes->insert(n);
        return cres;
    }

    void log_cres(int c, int cres, int cmax, int n, std::string& indent, bool last, std::unordered_map<int,int>& constraints) {
        std::ostringstream oss;
        oss << "[";
        for (auto& c : constraints)
        {
            oss << c.first << ",";
        }
        oss << "]";
        if (cres == cmax)
        {
            std::cout << indent.c_str() << "found best possible coloring: node " << n << ", color " << c << ", skip the rest" << std::endl;
        }
        else if (cres == -1)
        {
            std::cout << indent.c_str() << "couldn't find any possible coloring: node " << n << ", color " << c << (last ? "(last)" : "") << " " << oss.str().c_str() << std::endl;
        }
        else
        {
            std::cout << indent.c_str() << "found possible coloring: node " << n << ", color " << c << (last ? "(last)" : ", look further ") << std::endl;
        }
    }

    int next() {
        if (m_nodes->empty())
        {
            return -1;
        }
        return *m_nodes->begin();
    }

    void update_constraint(int n, int c, int prev_c) {
        m_colors[n] = c;
        //assert(affected == 1);
        for (auto& adj : m_g.adj[n])
        {
            if (m_colors[adj] > 0) continue;
            bool erasePrevConstraint = false;
            auto prev = m_constraints[adj].end();
            if (prev_c != -1)
            {
                prev = m_constraints[adj].find(prev_c);
                if (prev != m_constraints[adj].end())
                {
                    prev->second--;
                    if (prev->second == 0)
                    {
                        erasePrevConstraint = true;
                    }
                }
            }

            bool insertItConstraint = false;
            auto it = m_constraints[adj].find(c);
            if (it != m_constraints[adj].end())
            {
                it->second++;
            }
            else
            {
                insertItConstraint = true;
            }
            if (
                (!erasePrevConstraint && !insertItConstraint)
                || (erasePrevConstraint && insertItConstraint)
                )
            {
                if (erasePrevConstraint) m_constraints[adj].erase(prev);
                if (insertItConstraint) m_constraints[adj][c] = 1;
            }
            else
            {
                auto affected = m_nodes->erase(adj);
                if (erasePrevConstraint) m_constraints[adj].erase(prev);
                if (insertItConstraint) m_constraints[adj][c] = 1;

                m_nodes->insert(adj);
            }
        }
    }

    void remove_constraint(int n, int c) {
        m_colors[n] = 0;
        for (auto& adj : m_g.adj[n])
        {
            if (m_colors[adj] > 0) continue;
            auto it = m_constraints[adj].find(c);
            if (it != m_constraints[adj].end())
            {
                it->second--;
                if (it->second == 0)
                {
                    auto affected = m_nodes->erase(adj);
                    m_constraints[adj].erase(it);
                    if (affected == 1)
                    {
                        m_nodes->insert(adj);
                    }

                }
            }
        }
    }


public:
    Solver(const Graph& g) : m_g{ g }
    {
        m_constraints.resize(g.adj.size());
        m_colors.resize(g.adj.size());
        m_nodes = std::make_unique<std::set<int, comparator >>(comparator{m_g, m_constraints});

        int start = 0;
        int maxDegree = 0;
        for (int n = 0; n < g.adj.size(); n++)
        {
            if (g.adj[n].size() > maxDegree)
            {
                maxDegree = g.adj[n].size();
                start = n;
            }
        }

        int t = 0;
        color(start, 0, t, 0);
    }

    std::pair<int, std::vector<int> > solution() {
        return {m_cmax, m_solution};
    }
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "incorrect input";
        return -1;
    }

    std::ifstream in(argv[1]);
    if (in.fail())
    {
        std::cout << argv[1] << " not found.";
        return -1;
    }
    int n, e;
    in >> n >> e;
    std::vector< std::pair<int, int> > edges;
    for (auto i = 0; i < e; i++)
    {
        int b, e;
        in >> b >> e;
        edges.emplace_back(b, e);
    }

    auto begin = std::chrono::system_clock::now();
    Graph g{n, edges};
    std::cout << "=== BEGIN ===\n ";
    Solver slv{ g };
    auto end = std::chrono::system_clock::now();
    auto sol = slv.solution();
    std::cout << sol.first << " " << 1 << std::endl;
    for (auto& c : sol.second)
    {
        std::cout << c << " ";
    }
#ifdef _DEBUG
#endif
    std::cout << " === " << g_c << " in " << std::chrono::duration_cast<std::chrono::seconds>(end-begin).count() << " sec.";
}