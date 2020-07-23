  
#include <cmath>
#include <math.h>
#include "utils.h"

#include "BatchLoad.h"
#include "kernels.h"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>
#include <cstdlib>




//#define M_PI 3.1432
int window_size = 100;
auto window =0;
int radius = 1;
pc::Options opts; 

BYTE assign_color(const int d, const uint64_t dmax, char col)
{
    float rel_dist = (float)(d)/dmax;

    switch(col)
    {
        case 'r': return (BYTE)(rel_dist * 255);break;
        case 'g': return (BYTE)(rel_dist * 255);break;
        case 'b': return (BYTE)(rel_dist * 255);break;
        default: std::cout<<"Default case"<<std::endl;
        return (BYTE) 0;break;
    }
}

int compare_ints(const void* a, const void* b)   // comparison function
{
    int arg1 = *reinterpret_cast<const int*>(a);
    int arg2 = *reinterpret_cast<const int*>(b);
    if(arg1 < arg2) return -1;
    if(arg1 > arg2) return 1;
    return 0;
}

uint16_t distance(int z,int y,int x,int counter,pc::InputData const& data)
{
    int cpx = (int)(data.controlPositions[counter].x);
    int cpy = (int)data.controlPositions[counter].y;
    int cpz = (int)data.controlPositions[counter].z;
    int diff_x = pow((cpx-x),2);
    int diff_y = pow((cpy-y),2);
    int diff_z = pow((cpz-z),2);
    float displacement = (float) sqrt(diff_x+diff_y+diff_z);
    return (uint16_t) displacement;
}

void
Worleyfunc(pc::InputData const& obj)
{
  std::vector<int> dist(opts.numControlPoints);
  BYTE red, blue, green;
  char color[] = {'r','g','b'};
  int offset = 0;
  for(int z =0;z<opts.z;z++)
  {
    FIBITMAP* seqImg = FreeImage_AllocateT(FIT_BITMAP, opts.x, opts.y, 24);
    BYTE* OutputImage = FreeImage_GetBits(seqImg);

    for(int i=0;i<opts.y;i++)
    {
      for (int j =0;j<opts.x;j++)
      {
        for (int d = 0;d<opts.numControlPoints;d++)
        {
          dist[d] = distance(z,i,j,d,obj);
          //std::cout<<"ditance is "<<dist[d]<<std::endl;
        }
        //std::qsort(dist, opts.numControlPoints, sizeof(int), compare_ints);
        std::sort(dist.begin(), dist.end());
        /*for (int d = 0;d<opts.numControlPoints;d++)
        { 
          std::cout<<" sorted array is "<<dist[d]<<std::endl;
        }*/
        offset = (i * opts.x * 3) + j * 3;
        //change color according to distance index command line option
        red = assign_color(dist[opts.distanceIndex],opts.dmax,color[0]);
        green = assign_color(dist[opts.distanceIndex],opts.dmax,color[1]);
        blue = assign_color(dist[opts.distanceIndex],opts.dmax,color[2]);
        OutputImage[offset + 0] = (BYTE) blue;
        OutputImage[offset + 1] = (BYTE) green;
        OutputImage[offset + 2] = (BYTE) red;
               

      }

    }
    std::cout<<" Writing Image File"<<std::endl;
    std::string name = "outputseq/plane_z";
    if(z<10)
    {
        name = name+"00"+std::to_string(z)+".png";
    }
    else if(z<100)
    {
        name = name+"0"+std::to_string(z)+".png";
    }
    else
    {
        name = name+std::to_string(z)+".png";
    }

    bool ret = GenericWriter(seqImg, name.c_str(), PNG_DEFAULT);
    std::cout<<" Returned value is "<<ret<<std::endl;
    FreeImage_Unload(seqImg);
  
  }
}

void demo(pc::InputData const& data) {
    std::cout << "= control positions =" << std::endl;
    std::copy(std::begin(data.controlPositions),
        std::end(data.controlPositions) ,
        std::ostream_iterator< pc::Point >(std::cout, "\n")
    );

}

int
main(int argc, char **argv)
{
  pc::Options opts = pc::Options::parseCommandLine(argc, argv);
    std::cout << opts << std::endl;

    // Use this code to get initial values
    // You do not have to use the provided data structure Vec2, but use the
    // generated values!
    pc::InputData data = pc::InputData::generate(opts);

    demo(data);

    // See template code in kernels.h/cu
    // runGPUVariant();

    //return 0;
    switch(opts.mode)
    {
        case pc::Mode::GPUVersion2: std::cout<<" gpu version 2 code running "<<std::endl;
                runGPUVariant(&data,&opts); break;
        case pc::Mode::GPUVersion1: runGPUVariant(&data,&opts);
          std::cout<<" gpu version 1 code running "<<std::endl;
          break;
        case pc::Mode::Sequential: Worleyfunc(data);break;;
        std::cout<<" seq code running "<<std::endl;
          break;
        //case pc::Mode::GPUVersion2: runGPUVariant2();break;
        default: std::cout<<" Invalid option!! See the options "<<std::endl;break;
    }
  //Worleyfunc();


  return 0;             /* ANSI C requires main to return int. */
}
