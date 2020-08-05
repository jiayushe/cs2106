#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

struct xs128p {
public:
    using result_type = uint64_t;
    uint64_t max() { return std::numeric_limits<uint64_t>::max(); }
    uint64_t min() { return std::numeric_limits<uint64_t>::min(); }
    uint64_t operator()() {
        uint64_t t = state[0];
        uint64_t s = state[1];
        state[0] = s;
        t ^= t << 23;
        t ^= t >> 17;
        t ^= s ^ (s >> 26);
        state[1] = t;
        return t + s;
    }
    xs128p(uint64_t seed1 = 2346984356564098656ull, uint64_t seed2 = 1234768345892362178ull)
        : state{seed1, seed2} {
    }
private:
    uint64_t state[2];
};

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    int rprob, wprob, mprob, uprob, segvrprob, segvwprob, segvuprob;
    int num_instructions, initial_pages, total_pages, total_frames;
    uint64_t seed1, seed2;
    if (!(std::cin>>rprob>>wprob>>mprob>>uprob>>segvrprob>>segvwprob>>segvuprob>>num_instructions>>initial_pages>>total_pages>>total_frames>>seed1>>seed2)) {
        std::cerr<<"Invalid input!"<<std::endl;
        return 0;
    }
    assert(__builtin_popcount(total_pages)==1 && __builtin_popcount(total_frames)==1);
    assert(initial_pages<=total_pages);
    
    if (argc>=2 && strcmp(argv[1], "flags")==0) {
        // mode to generate grader compilation flags
        std::cout<<"-DGRADER_PAGE_BITS="<<__builtin_ctz(total_pages)<<' '<<"-DGRADER_FRAME_BITS="<<__builtin_ctz(total_frames);
        return 0;
    }
    
    int total = rprob+wprob+mprob+uprob+segvrprob+segvwprob+segvuprob;
    xs128p rng(seed1, seed2);
    std::cout<<initial_pages<<'\n';
    std::vector<int> m_pages, u_pages;
    for(int i=0;i!=initial_pages;++i){
        m_pages.push_back(i);
    }
    for(int i=initial_pages;i!=total_pages;++i){
        u_pages.push_back(i);
    }
    std::uniform_int_distribution<int> inst_dist(0,total-1);
    while (num_instructions-->0) {
        int inst_val = inst_dist(rng);
        if ((inst_val-=rprob)<0) {
            // 'r'
            if(m_pages.empty()){
                ++num_instructions;
                continue;
            }
            std::cout<<"r "<<m_pages[std::uniform_int_distribution<size_t>(0,m_pages.size()-1)(rng)]<<'\n';
        }
        else if ((inst_val-=wprob)<0) {
            // 'w'
            if(m_pages.empty()){
                ++num_instructions;
                continue;
            }
            std::cout<<"w "<<m_pages[std::uniform_int_distribution<size_t>(0,m_pages.size()-1)(rng)]<<'\n';
        }
        else if ((inst_val-=mprob)<0) {
            // 'm'
            if(u_pages.empty()){
                ++num_instructions;
                continue;
            }
            auto uit = u_pages.begin() + std::uniform_int_distribution<size_t>(0,u_pages.size()-1)(rng);
            std::cout<<"m "<<*uit<<'\n';
            m_pages.push_back(*uit);
            *uit=u_pages.back();
            u_pages.pop_back();
        }
        else if ((inst_val-=uprob)<0) {
            // 'u'
            if(m_pages.empty()){
                ++num_instructions;
                continue;
            }
            auto mit = m_pages.begin() + std::uniform_int_distribution<size_t>(0,m_pages.size()-1)(rng);
            std::cout<<"u "<<*mit<<'\n';
            u_pages.push_back(*mit);
            *mit=m_pages.back();
            m_pages.pop_back();
        }
        else if ((inst_val-=segvrprob)<0) {
            // 'r*'
            if(u_pages.empty()){
                ++num_instructions;
                continue;
            }
            std::cout<<"r * "<<u_pages[std::uniform_int_distribution<size_t>(0,u_pages.size()-1)(rng)]<<'\n';
        }
        else if ((inst_val-=segvwprob)<0) {
            // 'w*'
            if(u_pages.empty()){
                ++num_instructions;
                continue;
            }
            std::cout<<"w * "<<u_pages[std::uniform_int_distribution<size_t>(0,u_pages.size()-1)(rng)]<<'\n';
        }
        else if ((inst_val-=segvuprob)<0) {
            // 'u*'
            if(u_pages.empty()){
                ++num_instructions;
                continue;
            }
            std::cout<<"u * "<<u_pages[std::uniform_int_distribution<size_t>(0,u_pages.size()-1)(rng)]<<'\n';
        }
        else {
            assert(false);
        }
    }
}
