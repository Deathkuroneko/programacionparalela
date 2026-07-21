#include <iostream>
#include <fmt/core.h>
#include <omp.h>

int main()
{
    /*#pragma omp parallel num_threads(4)
        {
            int thread_id = omp_get_thread_num();

    #pragma omp master
            {
                int threads_count = omp_get_max_threads();
                fmt::print("Threads disponibles: {}\n", threads_count);
            }

            fmt::print("Hello, World!\n");
            fmt::print("My thread_id is {}\n", thread_id);
        }*/
    /*#pragma omp parallel
    {
        int thead_id =
        std::string msg = "";
        #pragma omp paraller for
        {
            for (int i = 0; i < thead_id; i++)
            {
                msg = msg+"*";
            }

        }
        fmt::println("My thread_id is {}, msg:{}", thead_id,msg);
    }*/
    int num_elementos = 15;
#pragma omp parallel for num_threads(4)
    for (int i = 0; i < 15; i++)
    {
        // fmt::println("i: {}, thead_id {}", i, omp_get_thread_num());
    }

#pragma omp parallel num_threads(4)
    {
        int therad_id = omp_get_thread_num();
        int thread_num = omp_get_num_threads();
        int delta = std::ceil(num_elementos * 1.0 / thread_num);
        int start = therad_id * delta;
        int end = (therad_id + 1) * delta;
        if (therad_id == thread_num - 1)
        {
            end = num_elementos;
        }

        fmt::println("Thread_{}: start={}, end={}", therad_id, start, end);
        for (int i = start; i < end; i++)
        {
        }
    }
fmt::println("----------------------------");
#pragma omp parallel num_threads(4)//division por saltos
    {
        int therad_id = omp_get_thread_num();
        int thread_num = omp_get_num_threads();
        for (int i = therad_id; i < num_elementos; i += 4)
        {
            fmt::println("Thread_id: {},index={}", therad_id, i);
        }
    }

    #pragma omp parallel
    {
        while (true)
        {
            /* code */
        }
        
    }




    return 0;
}