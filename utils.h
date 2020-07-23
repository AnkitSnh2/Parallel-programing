#ifndef PC_UTILS_H
#define PC_UTILS_H

#include <cmath>
#include <ostream>
#include <string>
#include <valarray>

namespace pc
{

struct Point {
    std::size_t x;
    std::size_t y;
    std::size_t z;

    Point() : x(0), y(0), z(0) {}
    explicit Point(std::size_t val) : x(val), y(val), z(val) {}
    Point(std::size_t xVal, std::size_t yVal, std::size_t zVal) : x(xVal), y(yVal), z(zVal) {}
};

std::ostream& operator<< (std::ostream& stream, pc::Point const& v);

enum Mode {
    Sequential,
    GPUVersion1,
    GPUVersion2
};

struct Options {
    std::size_t distanceIndex = 1;
    Mode mode = Mode::Sequential;
    std::size_t numControlPoints = 15;
    std::string outputDirectory = "output";
    std::uint64_t seed = 1234;
    std::size_t x = 800;
    std::size_t y = 600;
    std::size_t z = 400;
    uint64_t dmax = 100;

    static Options parseCommandLine(int argc, char * const argv[]);
};

struct InputData {
    std::valarray< Point > controlPositions;

    static InputData generate(Options const& o);

private:
    InputData() {}
};

} // namespace pc

std::ostream& operator<< (std::ostream& stream, pc::Options const& o);

#endif // PC_UTILS_H
