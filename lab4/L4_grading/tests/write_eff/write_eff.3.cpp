// a fixed-size circular queue
#include <iostream>
#include <cstring>
constexpr int N = 1<<13; // num pages
constexpr int F = 1<<11; // num frames
constexpr int Q = 1<<13;
constexpr int INITIAL = N-10;
constexpr int QUEUE_SIZE = INITIAL/2;
int main(int argc, char** argv){
    if (argc>=2 && std::strcmp(argv[1], "flags")==0) {
        // mode to generate grader compilation flags
        std::cout<<"-DGRADER_PAGE_BITS="<<__builtin_ctz(N)<<' '<<"-DGRADER_FRAME_BITS="<<__builtin_ctz(F)<<'\n';
        return 0;
    }
    std::cout<<INITIAL<<'\n';
    int read_head = 0;
    int write_head = QUEUE_SIZE;
    for(int i=0; i<Q; ++i){
        std::cout<<"w "<<write_head++<<'\n';
        std::cout<<"r "<<read_head++<<'\n';
        if(write_head==INITIAL)write_head=0;
        if(read_head==INITIAL)read_head=0;
    }
}
