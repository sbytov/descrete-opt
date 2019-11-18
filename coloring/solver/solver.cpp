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
#include <future>

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
std::atomic<bool> g_abort = false;

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
            if (m_constraints[n1].size() != m_constraints[n2].size())
            {
                return m_constraints[n1].size() > m_constraints[n2].size();
            }
            if (m_g.adj[n1].size() != m_g.adj[n2].size())
            {
                return m_g.adj[n1].size() > m_g.adj[n2].size();
            }
            return n1 > n2;
        }
    };

    std::unique_ptr<std::set<int, comparator > > m_nodes;
    std::vector<std::unordered_map<int, int> > m_constraints;
    std::vector<int> m_colors;
    std::vector<int> m_solution;
    int m_cmax = std::numeric_limits<int>::max();

    int color(int n, int cmax, int level) 
    {
        if (g_abort) return -1;

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
#ifdef _DEBUG
            exit(1);
#endif
            return cmax;
        }
        auto affected = m_nodes->erase(n);
        int prev_c = -1;
        int cres = -1;
        std::string indent = std::string(level, '.');

        for (int c = 1; c <= cmax; c++)
        {
            if (m_constraints[n].count(c) > 0)
            {
                continue;
            }
#ifdef _DEBUG
            std::cout << indent.c_str() << "color " << n << " into " << c << log_constraints(n) << std::endl;
#endif
            update_constraint(n, c, prev_c);
            prev_c = c;
            int depth = 0;
            auto nn = next();
            cres = color(nn, cmax, level + 1);
            log_cres(c, cres, cmax, n, indent, false);
            if (cres == cmax)
            {
                break;
            }
            //remove_constraint(n, c);
        }
        if (cmax + 1 < m_cmax
           /* && s <= quota*/)
        {
#ifdef _DEBUG
            std::cout << indent.c_str() << "color " << n << " into " << cmax + 1 << " (max)" << log_constraints(n) << std::endl;
#endif
            update_constraint(n, cmax + 1, prev_c);
            prev_c = cmax + 1;
            int depth = 0;
            int nn = next();
            cres = color(nn, cmax + 1, level + 1);
            log_cres(cmax+1, cres, cmax, n, indent, true);
        }
        if (prev_c != -1) remove_constraint(n, prev_c);
        m_nodes->insert(n);
        return cres;
    }

    std::string log_constraints(int n) {
        std::ostringstream oss;
        oss << " [";
        for (auto& c : m_constraints[n])
        {
            oss << c.first << ",";
        }
        oss << "]";
        return oss.str();
    }

    void log_cres(int c, int cres, int cmax, int n, std::string& indent, bool last) {
#ifdef _DEBUG
        if (cres == cmax)
        {
            std::cout << indent.c_str() << "found best possible coloring: node " << n << ", color " << c << ", skip the rest" << std::endl;
        }
        else if (cres == -1)
        {
            std::cout << indent.c_str() << "couldn't find any possible coloring: node " << n << ", color " << c << (last ? "(last)" : "") << " " << std::endl;
        }
        else
        {
            std::cout << indent.c_str() << "found possible coloring: node " << n << ", color " << c << (last ? "(last)" : ", look further ") << std::endl;
        }
#endif
    }

    int next() {
        if (m_nodes->empty())
        {
            return -1;
        }
        auto n = *m_nodes->begin();
        if (m_colors[n] != 0)
        {
            std::cout << "OHHHHHH SOMETHING BAD" << std::endl;
        }
        //if (m_nodes->size() > 1)
        //{
        //    auto nn = *(++m_nodes->begin());
        //    return nn;
        //}
        return n;
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
    Solver(const Graph& g, uint64_t quota) : m_g{ g }
    {
        m_constraints.resize(g.adj.size());
        m_colors.resize(g.adj.size());
        m_nodes = std::make_unique<std::set<int, comparator >>(comparator{m_g, m_constraints});
        for (int n = 0; n < g.adj.size(); n++)
        {
            m_nodes->insert(n);
        }

        std::vector<int> coloring(m_g.adj.size(), 0);
        color(*m_nodes->begin(), 0, 0);
    }

    std::pair<int, std::vector<int> > solution() {
        return {m_cmax, m_solution};
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "incorrect input";
        return -1;
    }

    uint64_t quota = 100000000;
    if (argc == 3)
    {
        quota = atoi(argv[2]);
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
   // std::cout << "=== BEGIN ===\n ";
    auto fut = std::async(std::launch::async, [&]() {
        Solver slv{ g, quota };
        auto sol = slv.solution();
        return sol;
    });
    auto status = fut.wait_for(std::chrono::seconds(60));
    if (status == std::future_status::timeout)
    {
        g_abort = true;
    }
    auto sol = fut.get();
    auto end = std::chrono::system_clock::now();
    std::cout << sol.first << " " << (g_abort ? 0 : 1)  << std::endl;
    for (auto& c : sol.second)
    {
        std::cout << c << " ";
    }
#ifdef _DEBUG
    std::cout << " === " << g_c << " in " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " sec.";
#endif
    }