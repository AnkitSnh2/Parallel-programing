#include "utils.h"

#include <cstring>
/* #include <iomanip> */
#include <iostream>
#include <random>


#include <getopt.h>

namespace pc {

std::ostream&
operator<< (std::ostream& stream, pc::Point const& p)
{
    stream << "Point( "
        << p.x
        << ", "
        << p.y
        << ", "
        << p.z
        << " )";

    return stream;
}

/* static */
InputData InputData::generate(Options const& o)
{
    std::mt19937_64 gen(o.seed);
    std::uniform_int_distribution<> xDist(0, o.x - 1);
    std::uniform_int_distribution<> yDist(0, o.y - 1);
    std::uniform_int_distribution<> zDist(0, o.z - 1);
    InputData d;

    d.controlPositions.resize(o.numControlPoints);
    std::generate(std::begin(d.controlPositions),
        std::end(d.controlPositions),
        [&] { return Point(xDist(gen), yDist(gen), zDist(gen)); });

    return d;
}

void usage(char const* programName)
{
    std::cerr << "Usage: " << programName
        << " [-d d-th shortest distance to pick]"
        << " [-h show this help]"
        << " [-m mode {seq, gpuv1, gpuv2}]"
        << " [-n number of control points]"
        << " [-o output directory]"
        << " [-s random seed]"
        << " [-x x-dimension of cuboid]"
        << " [-y y-dimension of cuboid]"
        << " [-z z-dimension of cuboid]"
        << std::endl;
}

/* static */
Options
Options::parseCommandLine(int argc, char * const argv[])
{
    Options result;

    int opt;
    while ((opt = getopt(argc, argv, "d:hm:n:o:s:x:y:z:")) != -1) {
        switch (opt) {
        case 'd':
            result.distanceIndex = std::stoul(optarg);
            break;
        case 'h':
            usage(argv[0]);
            exit(EXIT_FAILURE);
        case 'm':
            if (strncmp(optarg, "seq", 3) == 0) {
                result.mode = Mode::Sequential;
            } else if (strncmp(optarg, "gpuv1", 5) == 0) {
                result.mode = Mode::GPUVersion1;
            } else if (strncmp(optarg, "gpuv2", 5) == 0) {
                result.mode = Mode::GPUVersion2;
                std::cout<<"GPU V2 called"<<std::endl;
            } else {
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'n':
            result.numControlPoints = std::stoul(optarg);
            break;
        case 'o':
            result.outputDirectory = optarg;
            break;
        case 's':
            result.seed = std::stoul(optarg);
            break;
        case 'x':
            result.x = std::stoul(optarg);
            break;
        case 'y':
            result.y = std::stoul(optarg);
            break;
        case 'z':
            result.z = std::stoul(optarg);
            break;
        default: /* '?' */
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        std::cerr << "Unknown argument " << argv[optind] << std::endl;
        exit(EXIT_FAILURE);
    }

   return result;
}


} // namespace pc


std::ostream& operator<< (std::ostream& stream, pc::Options const& o)
{
    stream
        << "distance index: " << o.distanceIndex
        << ", mode: " << o.mode
        << ", n-control points: " << o.numControlPoints
        << ", output directory: " << o.outputDirectory
        << ", seed: " << o.seed
        << ", x: " << o.x
        << ", y: " << o.y
        << ", z: " << o.z
        ;

    return stream;
}
