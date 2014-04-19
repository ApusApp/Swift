#include <iostream>
#include <stdexcept>
#include <thread>
#include <map>
#include <sys/time.h>
#include <swift/disruptor/disruptor.hpp>

#define RING_BUFFER_SIZE 1024

using namespace swift::disruptor;
using namespace std;

int main (int argc, char** argv)
{
    uint64_t iterations = 1000L * 1000L * 100;

    // data source / publisher
    auto source = std::make_shared<RingBuffer<uint64_t, RING_BUFFER_SIZE>> ();
    auto square = std::make_shared<RingBuffer<uint64_t, RING_BUFFER_SIZE>> ();
    auto cube = std::make_shared<RingBuffer<uint64_t, RING_BUFFER_SIZE>> ();
    auto diff = std::make_shared<RingBuffer<uint64_t, RING_BUFFER_SIZE>> ();

    auto a = std::make_shared<ReadCursor> ("a");
    auto b = std::make_shared<ReadCursor> ("b");
    auto c = std::make_shared<ReadCursor> ("c");
    auto p = std::make_shared<WriteCursor> ("write", RING_BUFFER_SIZE);

    a->follows (p);
    b->follows (p);
    c->follows (a);
    c->follows (b);
    p->follows (c);

    // thread publisher
    auto pub_thread = [&] ()
    {
        try {
            auto pos = p->begin ();
            auto end = p->end ();

            for (uint64_t i = 0; i < iterations;) {
                // 1. wait for write slot
                if (pos >= end) {
                    end = p->wait_for (end);
                }

                // 2. write event in batches
                do {
                    source->at (pos) = i;
                    ++pos;
                    ++i;

                } while (pos < end);

                // 3. publish pos
                p->publish (pos - 1);

                // note, publishing in 'batches' is 2x as fast, hitting this
                // memory barrior really slows things down in this trival example.
            }

            p->set_eof ();
        }
        catch (std::exception& e) {
            std::cerr << "publisher caught: " << e.what () << " at pos " << p->pos ().aquire () << "\n";
        }
    };

    // thread a
    auto thread_a = [&] ()
    {
        try {
            auto pos = a->begin ();
            auto end = a->end ();
            while (true) {
                if (pos == end) {
                    a->publish (pos - 1);
                    end = a->wait_for (end);
                }

                square->at (pos) = source->at (pos) * source->at (pos);
                ++pos;
            }
        }
        catch (std::exception& e) {
            std::cerr << "a caught: " << e.what () << "\n";
        }
    };

    // thread b
    auto thread_b = [&] ()
    {
        try {
            auto pos = b->begin ();
            auto end = b->end ();
            while (true) {
                if (pos == end) {
                    b->publish (pos - 1);
                    end = b->wait_for (end);
                }

                cube->at (pos) = source->at (pos) * source->at (pos) * source->at (pos);
                ++pos;
            }

        }
        catch (std::exception& e) {
            std::cerr << "b caught: " << e.what () << "\n";
        }
    };

    // thread c
    auto thread_c = [&] ()
    {
        try {
            auto pos = c->begin ();
            auto end = c->end ();
            while (true) {
                if (pos == end) {
                    c->publish (pos - 1);
                    end = c->wait_for (end);
                }

                diff->at (pos) = cube->at (pos) - square->at (pos);
                ++pos;
            }

        }
        catch (std::exception& e) {
            std::cerr << "c caught: " << e.what () << "\n";
        }
    };

    std::thread pt (pub_thread);
    std::thread at (thread_a);
    std::thread bt (thread_b);
    std::thread ct (thread_c);

    struct timeval start_time, end_time;
    gettimeofday (&start_time, NULL);

    pt.join ();
    at.join ();
    bt.join ();
    ct.join ();

    gettimeofday (&end_time, NULL);

    double start, end;
    start = start_time.tv_sec + ((double)start_time.tv_usec / 1000000);
    end = end_time.tv_sec + ((double)end_time.tv_usec / 1000000);

    std::cout.precision (15);
    std::cout << "1P-3C-UNICAST performance: ";
    std::cout << (iterations * 1.0) / (end - start) << " ops/secs" << std::endl;

    std::cout << "Source: " << cube->at (0) << "  2x: " << square->at (0) << " diff: " << diff->at (0) << std::endl;
    std::cout << "Source: " << cube->at (1) << "  2x: " << square->at (1) << " diff: " << diff->at (1) << std::endl;
    std::cout << "Source: " << cube->at (2) << "  2x: " << square->at (2) << " diff: " << diff->at (2) << std::endl;
    std::cerr << "Exiting " << std::endl;

    return 0;
}
