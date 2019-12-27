// solver.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>
#include <algorithm>

struct point {
	float x;
	float y;
	int i;
	point* next = nullptr;
	point* prev = nullptr;
	float length;
	std::vector<std::pair<uint16_t, float>> distTo;
	std::vector<std::pair<uint16_t, float>> closest;

	void set_next(point* next) {
		this->next = next;
		next->prev = this;
		this->length = distTo[next->i].second;
	}
};

float distance(point* route) {
	auto begin = route;
	float dist = 0;
	while (route->next != begin) {
		dist += route->length;
		route = route->next;
	}
	dist += route->length;
	return dist;
}

struct point_comparator {
	bool operator()(point* const a, point* const b) const {
		if (a->length != b->length) return a->length > b->length;
		return a->i < b->i;
	}
};
std::set<point*, point_comparator > pq;
std::vector<point> points; 

bool kopt();


std::vector<int> getRoute(point* route)
{
    std::vector<int> ret;
    auto n = route;
    while (n->next != route) {
    	ret.push_back(n->i);
    	n = n->next;
    }
    ret.push_back(n->i);
    return ret;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "wrong number of arguments";
		return -1;
	}
	std::string input = argv[1];
	std::ifstream in(input);
	int N;
	in >> N;

	for (int i = 0; i < N; i++) {
		point p;
		in >> p.x >> p.y;
		p.i = i;
		p.distTo.resize(N);
		points.push_back(p);
	}
	for (int i = 0; i < N; i++) {
		for (int j = i + 1; j < N; j++) {
			float d = std::sqrt((points[j].x - points[i].x)*(points[j].x - points[i].x) + (points[j].y - points[i].y)*(points[j].y - points[i].y));
			points[i].distTo[j] = std::make_pair(j, d);
			points[j].distTo[i] = std::make_pair(i, d);
		}
		points[i].distTo[i] = std::make_pair(i, 0);
	}
	for (int i = 0; i < N; i++) {
		points[i].closest.resize(points[i].distTo.size()/2);
		std::partial_sort_copy(points[i].distTo.begin(), points[i].distTo.end(),
			points[i].closest.begin(), points[i].closest.end(),
			[](const std::pair<uint16_t, float>& a, const std::pair<uint16_t, float>& b) {
			if (a.second < b.second) return true;
			if (a.second > b.second) return false;
			return a.first < b.first;
		});
	}
	point* route = &points[0];
	while (true) {
		bool nomore = true;
		for (auto c : route->closest) {
			if (points[c.first].prev == nullptr && c.first != 0) {
				route->set_next(&points[c.first]);
				pq.insert(route);
				goto goon;
			}
		}
		for (auto c : route->distTo) {
			if (points[c.first].prev == nullptr && c.first != 0 ) {
				route->set_next(&points[c.first]);
				pq.insert(route);
				goto goon;
			}
		}
		route->set_next(&points[0]);
		pq.insert(route);
		route = route->next;
		break;
goon:
		route = route->next;
	}


   auto best = getRoute(route);

	while (!pq.empty()) {
		if (kopt()) {
			std::cout << distance(route) << " " << 0 << std::endl;

            best = getRoute(route);
			//auto n = route;
			//while (n->next != route) {
			//	std::cout << n->i << " ";
			//	n = n->next;
			//}
			//std::cout << n->i << std::endl;
		}
	}
	//std::cout << "---------------------------------" << std::endl;
	std::cout << distance(route) << " " << 0 << std::endl;
    for (auto i : best)
    {
       std::cout << i << " ";
    }
    return 0;
}

bool kopt() {
	auto p = *pq.begin();
	point* n = p->next;
	std::vector<int> candidates;
	int i = 0;
	while (i < n->closest.size() && n->closest[i].second < p->length) {
		if (n->closest[i].first != n->i && n->closest[i].first != n->next->i) {
			candidates.push_back(n->closest[i].first);
		}
		i++;
	}
	float delta = -1;
	for (auto ci : candidates) {
		auto c = &points[ci];
		auto o = c->prev;
		delta = p->distTo[n->i].second + o->distTo[c->i].second - p->distTo[o->i].second - n->distTo[c->i].second;
		if (delta <= 1) continue;

		pq.erase(o);
		pq.erase(p);
		pq.erase(n);
		pq.erase(c);

		auto rev = o;
		while (rev != n){
			auto oldnext = rev->next;
			pq.erase(rev);
			rev->next = rev->prev;
			rev->prev = oldnext;
			rev->length = rev->distTo[rev->next->i].second;
			pq.insert(rev);
			rev = rev->next;

		}
		n->prev = n->next;
		n->set_next(c);
		p->set_next(o);
		pq.insert(p);
		pq.insert(n);
		pq.insert(o);
		pq.insert(c);
		break;
	}
	if (delta <= 1 )
		pq.erase(p);
	return delta > 1;
}
