
#include "plane_delaunay.hpp"

#include <fstream>
#include <iostream>


namespace mesh {
    namespace Delaunay {


        void markDomain(CDT& cdt) {
            for (auto f : cdt.all_face_handles()) f->info().nesting_level = -1;

            std::deque<FH> q;

            auto mark_component = [&](FH start, int index) {
                if (start->info().nesting_level != -1) return;
                q.clear(); q.push_back(start);
                while (!q.empty()) {
                    FH f = q.front(); q.pop_front();
                    if (f->info().nesting_level != -1) continue;
                    f->info().nesting_level = index;
                    for (int i = 0; i < 3; i++) {
                        FH n = f->neighbor(i);
                        if (cdt.is_constrained(std::make_pair(f, i))) continue;
                        if (n->info().nesting_level == -1) q.push_back(n);
                    }
                }
            };

            mark_component(cdt.infinite_face(), 0);

            int index = 1;
            for (auto f : cdt.all_face_handles())
                if (f->info().nesting_level == -1)
                    mark_component(f, index++);
        }


        void insertPolygonEdgeConstraint(CDT& cdt, const std::vector<P2>& outer) {
            if (outer.size() < 2) return;

            std::vector<VH> vhs; 
            vhs.reserve(outer.size());

            for (auto& p : outer) 
                vhs.push_back(cdt.insert(p));

            for (size_t i = 0, n = outer.size(); i < n; ++i)
                cdt.insert_constraint(vhs[i], vhs[(i + 1) % n]);

        }


        std::vector<int> extractOuterBoundaryCCW(CDT& cdt, const std::unordered_map<VH, int>& vh2idx) {
            markDomain(cdt);

            std::unordered_map<VH, VH> next_of;
            std::unordered_map<VH, bool> on_boundary;

            for (auto e = cdt.edges_begin(); e != cdt.edges_end(); ++e) {
                FH  f = e->first;
                int i = e->second;
                FH  g = f->neighbor(i);

                bool cons = cdt.is_constrained(*e);
                bool finf = cdt.is_infinite(f);
                bool ginf = cdt.is_infinite(g);

                bool fin = (!finf && f->info().in_domain());
                bool gin = (!ginf && g->info().in_domain());

                if (!cons) continue;

                if (fin != gin) {
                    VH v1 = f->vertex((i + 1) % 3);
                    VH v2 = f->vertex((i + 2) % 3);

                    VH src = fin ? v1 : v2;
                    VH dst = fin ? v2 : v1;

                    next_of[src] = dst;
                    on_boundary[src] = on_boundary[dst] = true;
                }
            }

            if (next_of.empty()) return {};

            auto less_xy = [](const P2& a, const P2& b) {
                if (a.x() < b.x()) return true;
                if (a.x() > b.x()) return false;
                return a.y() < b.y();
            };

            VH start = nullptr;
            P2 best(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
            for (auto& kv : on_boundary) if (kv.second) {
                VH v = kv.first;
                const P2& p = v->point();
                if (less_xy(p, best)) { best = p; start = v; }
            }

            std::vector<VH> ring;
            VH cur = start;
            do {
                ring.push_back(cur);
                auto it = next_of.find(cur);
                if (it == next_of.end()) break;
                cur = it->second;
            } while (cur != start && ring.size() <= next_of.size() + 1);

            std::vector<int> idx;
            idx.reserve(ring.size());
            for (VH v : ring) idx.push_back(vh2idx.at(v));

            auto signed_area = [&](const std::vector<VH>& vs) {
                long double A = 0;
                for (size_t i = 0, n = vs.size(); i < n; ++i) {
                    const auto& a = vs[i]->point();
                    const auto& b = vs[(i + 1) % n]->point();
                    A += (long double)a.x() * b.y() - (long double)b.x() * a.y();
                }
                return A * 0.5L;
            };
            if (ring.size() >= 3 && signed_area(ring) < 0) {
                std::reverse(idx.begin(), idx.end());
            }

            return idx;
        }


    }
}