#include "wrapper_py_optical_flow.h"
#include "../benchmarks.h"

#include "Halide.h"
#include "halide_image_io.h"
#include "tiramisu/utils.h"
#include <cstdlib>
#include <iostream>
#include <stdlib.h>


int main(int, char**)
{
    std::vector<std::chrono::duration<double,std::milli>> duration_vector_1;
    std::vector<std::chrono::duration<double,std::milli>> duration_vector_2;

#if SYNTHETIC_INPUT
    Halide::Buffer<uint8_t> im1(SYNTHETIC_INPUT_SIZE, SYNTHETIC_INPUT_SIZE);
    Halide::Buffer<uint8_t> im2(SYNTHETIC_INPUT_SIZE, SYNTHETIC_INPUT_SIZE);

    for (int i = 0; i < SYNTHETIC_INPUT_SIZE; i++)
	    for (int j = 0; j < SYNTHETIC_INPUT_SIZE; j++)
	    {
		    im1(i, j) = (uint8_t) i*i+j*j;
		    im2(i, j) = (uint8_t) i*i+j*j;
	    }
#else
    Halide::Buffer<uint8_t> im1 = Halide::Tools::load_image("./utils/images/rgb.png");
    Halide::Buffer<uint8_t> im2 = Halide::Tools::load_image("./utils/images/rgb.png");
#endif

    Halide::Buffer<float> Ix_m(im1.width(), im1.height(), npyramids);
    Halide::Buffer<float> Iy_m(im1.width(), im1.height(), npyramids);
    Halide::Buffer<float> It_m(im1.width(), im1.height(), npyramids);
    Halide::Buffer<int> C1(_NC);
    Halide::Buffer<int> C2(_NC);
    Halide::Buffer<int> SIZES(2);
    Halide::Buffer<int> u(_NC);
    Halide::Buffer<int> v(_NC);
    Halide::Buffer<float> A(2, 4*w*w);
    Halide::Buffer<float> tA(4*w*w, 2);
    Halide::Buffer<double> pinvA(4*w*w, 2);
    Halide::Buffer<double> det(1);
    Halide::Buffer<float> tAA(2, 2);
    Halide::Buffer<double> X(2, 2);
    Halide::Buffer<uint8_t> pyramids1(im1.width(), im1.height(), npyramids);
    Halide::Buffer<uint8_t> pyramids2(im1.width(), im1.height(), npyramids);

    SIZES(0) = im1.height();
    SIZES(1) = im1.width();
#ifdef SYNTHETIC_INPUT
    C1(0) = 4; C2(0) = 5;
    C1(1) = 6; C2(1) = 6;
#else
    C1(0) = 500; C2(0) = 400;
    C1(1) = 800; C2(1) = 900;
    C1(2) = 200; C2(2) = 400;
    C1(3) = 400; C2(3) = 200;
    C1(4) = 400; C2(4) = 500;
    C1(5) = 800; C2(5) = 200;
    C1(6) = 200; C2(6) = 900;
    C1(7) = 900; C2(7) = 200;
#endif

    det(0) = 0;
    init_buffer(Ix_m, (float) 0);
    init_buffer(Iy_m, (float) 0);
    init_buffer(It_m, (float) 0);
    init_buffer(A, (float) 0);
    init_buffer(tA, (float) 0);
    init_buffer(pinvA, (double) 0);
    init_buffer(tAA, (float) 0);
    init_buffer(X, (double) 0);
    init_buffer(pyramids1, (uint8_t) 0);
    init_buffer(pyramids2, (uint8_t) 0);

    // Warm up
    py_optical_flow_tiramisu(SIZES.raw_buffer(), im1.raw_buffer(), im2.raw_buffer(),
			  Ix_m.raw_buffer(), Iy_m.raw_buffer(), It_m.raw_buffer(),
			  C1.raw_buffer(), C2.raw_buffer(), u.raw_buffer(), v.raw_buffer(),
			  A.raw_buffer(), pinvA.raw_buffer(), det.raw_buffer(), tAA.raw_buffer(),
			  tA.raw_buffer(), X.raw_buffer(), pyramids1.raw_buffer(), pyramids2.raw_buffer());

    // Tiramisu
    for (int i=0; i<NB_TESTS; i++)
    {
        auto start1 = std::chrono::high_resolution_clock::now();
        py_optical_flow_tiramisu(SIZES.raw_buffer(), im1.raw_buffer(), im2.raw_buffer(),
			  Ix_m.raw_buffer(), Iy_m.raw_buffer(), It_m.raw_buffer(),
			  C1.raw_buffer(), C2.raw_buffer(), u.raw_buffer(), v.raw_buffer(),
			  A.raw_buffer(), pinvA.raw_buffer(), det.raw_buffer(), tAA.raw_buffer(),
			  tA.raw_buffer(), X.raw_buffer(), pyramids1.raw_buffer(), pyramids2.raw_buffer());
        auto end1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::milli> duration1 = end1 - start1;
        duration_vector_1.push_back(duration1);
    }

    std::cout << "Time: " << median(duration_vector_1) << std::endl;

#if SYNTHETIC_INPUT
    std::cout << "Im1." << std::endl;
    print_buffer(im1);
    std::cout << "Im2." << std::endl;
    print_buffer(im2);

    std::cout << "pyramids1" << std::endl;
    print_buffer(pyramids1);
    std::cout << "pyramids2" << std::endl;
    print_buffer(pyramids2);

    std::cout << "Ix_m." << std::endl;
    print_buffer(Ix_m);
    std::cout << "Iy_m." << std::endl;
    print_buffer(Iy_m);
    std::cout << "It_m." << std::endl;
    print_buffer(It_m);

    std::cout << "A." << std::endl;
    print_buffer(A);
    std::cout << "tA." << std::endl;
    print_buffer(tA);
    std::cout << "tAA." << std::endl;
    print_buffer(tAA);
    std::cout << "det." << std::endl;
    print_buffer(det);

    std::cout << "X" << std::endl;
    print_buffer(X);

    std::cout << "pinvA" << std::endl;
    print_buffer(pinvA);
#endif

    std::cout << "Output" << std::endl;
    print_buffer(u);
    print_buffer(v);

    return 0;
}
