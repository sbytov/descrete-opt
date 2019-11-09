// solver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stack>

struct item {
    int value = { 0 };
    int weight = { 0 };
    int idx = {0};
    float est = {0};

    float value_per_weight_unit() const {
        return float(value)/float(weight);
    }
};

uint64_t g_e, g_v;

class DFSSolver {
public:
    DFSSolver(int cap, const std::vector<item>& items) : m_cap {cap}, m_items{items}
    {
        std::sort(m_items.begin(), m_items.end(), [](const item& lhs, const item& rhs) { return lhs.value_per_weight_unit() > rhs.value_per_weight_unit(); });
    }

    std::pair<int, std::vector<int>> solve() {
        std::stack<int> temp;
        visit(0, 0, 0, temp);
        std::vector<int> solution;
        int value = 0;
        while (!m_taken.empty())
        {
            solution.push_back(m_items[m_taken.top()].idx);
            value += m_items[m_taken.top()].value;
            m_taken.pop();
        }
        return { value, solution };
    }
private:
    int m_cap;
    std::vector<item> m_items;
    int m_bestValue;
    std::stack<int> m_taken;

    void visit(int i, int curr_weight, int curr_val, std::stack<int>& taken) {
        if (curr_val > m_bestValue)
        {
            m_bestValue = curr_val;
            m_taken = taken;
        }
        if (i >= m_items.size())
        {
            return;
        }
        g_v++;
        if (fits(i, curr_weight))
        {
            taken.push(i);
#if _DEBUG
            //dbg(taken);
#endif
            visit(i + 1, curr_weight + m_items[i].weight, curr_val + m_items[i].value, taken);
            taken.pop();
        }
        if (curr_val + estimate(i + 1, m_cap-curr_weight) > m_bestValue)
        {
            visit(i + 1, curr_weight, curr_val, taken);
        }
    }

    void dbg(const std::stack<int>& taken) {
        auto copy = taken;
        while (!copy.empty())
        {
            std::cout << copy.top() << ", ";
            copy.pop();
        }
        std::cout << std::endl;
    }

    bool fits(int i, int curr_cap) {
        return curr_cap + m_items[i].weight <= m_cap;
    }
public:
    float estimate(int n, int cap_left) {
        float e = 0;
        int weight = 0;
        for (int i = n; i< m_items.size(); i++)
        {
            g_e++;
            auto& it = m_items[i];
            weight += it.weight;
            if (weight <= cap_left)
            {
                e += it.value;

            }
            else
            {
                e += it.value_per_weight_unit() * (cap_left - (weight - it.weight));;
                break;
            }
        }
        return e;
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
    int n, cap;
    in >> n >> cap;
    //std::cout << n << " " << cap;

    std::vector<item> items;
    for (auto i = 0; i < n; i++)
    {
        item it;
        in >> it.value >> it.weight;
        it.idx = i;
        items.push_back(std::move(it));
    }


    DFSSolver slv{cap, items};
    auto solution = slv.solve();

    std::cout << solution.first << " " << 1 << std::endl;

    std::vector<short> out(items.size());
#ifdef _DEBUG
    std::cout << "optimistic " << slv.estimate(0, cap) << std::endl;
#endif
    int verify = 0;
    for (auto& i : solution.second)
    {
        out[i] = true;
        verify += items[i].value;
    }
    for (auto& o : out)
    {
        std::cout << o << " ";
    }

#ifdef _DEBUG
    std::cout << " === " << verify << "(" << g_v << "," << g_e << ")";
#endif
}