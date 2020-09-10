#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <chrono>
#include <omp.h>

using namespace std;

typedef vector<int> Array1D;
typedef vector<Array1D> Array2D;
typedef vector<Array2D> Image;

// Test if point c belong to the Mandelbrot Set
bool mandelbrot(complex<double> c, vector<int>& pixel, int thread_id) {
    int max_iteration = 500, iteration = 0;
    complex<double> z(0, 0);

    while ( abs(z) <= 4 && (iteration < max_iteration) ) {
        z = z * z + c;
        iteration++;
    }

    if (iteration != max_iteration) {
        pixel = {255, 255, 255}; // outside -> white 255,255,255
        return false;
    }
    
    pixel = {80*thread_id,10*thread_id,255-60*thread_id}; //inside colour based on thread_id
    return true;
}

int main(int argc, char **argv)
{
    // height and width of the output image
    // square size assumed for simplicity
    int width = 1200, height = 1200; 

    int i, j, ch, pixels_inside=0;

    // Image data structure: 
    // - for each pixel we need red, green, and blue values (0-255)
    // - we use 3 different matrices for corresponding channels
    int channels = 3; // red, green, blue
    Image image(channels, Array2D(height, Array1D(width)));

    // pixel to be passed to the mandelbrot function
    vector<int> pixel = {0,0,0}; // red,green,blue (each range 0-255)
    complex<double> c; 
   
    int num_threads = 16; // Change value here to change number of threads
    omp_set_num_threads(num_threads);
    
    int choice;
    cout<<"Enter 1 for Variant1 OR 2 for Variant2."<<endl<<"Enter choice: ";
    cin>>choice;
    cout<<endl<<"Executing your choice "<<"..."<<endl<<endl;
    
    auto t1 = omp_get_wtime();
    switch (choice) {
        case 1:
        {
            for (i = 0; i < height; i++) {
            #pragma omp parallel for private (c,pixel) shared(j,i,channels,width,height,image) reduction(+:pixels_inside) schedule(static) // schedule(dynamic) OR schedule(runtime) OR schedule(guided) OR schedule(auto)
                for (j = 0; j < width; j++) {
                    c = complex<double>( 2.0*((double)j/width-0.75), ((double)i/height-0.5)*2.0);
                  if ( mandelbrot(c, pixel, omp_get_thread_num()) ) pixels_inside++;
                    // apply to the image
                    for (int ch = 0; ch < channels; ch++)
                        image[ch][i][j] = pixel[ch];
                }
            }
            break;
        }
        case 2:
        {
         #pragma omp parallel for private (c,j,pixel) shared(i,channels,width,height,image) reduction(+:pixels_inside) schedule(static) // schedule(dynamic) OR schedule(runtime) OR schedule(guided) OR schedule(auto)
         for (i = 0; i < height; i++) {
             for (j = 0; j < width; j++) {
                 c = complex<double>( 2.0*((double)j/width-0.75), ((double)i/height-0.5)*2.0);
               if ( mandelbrot(c, pixel, omp_get_thread_num()) ) pixels_inside++;
                 // apply to the image
                 for (int ch = 0; ch < channels; ch++)
                     image[ch][i][j] = pixel[ch];
             }
         }
            break;
        }
        default:
        {
            cout<<"Invalid choice entered. Try again!"<<endl<<endl;
            return 0;
        }
    }
	 auto t2 = omp_get_wtime();
    
    // save image
    std::ofstream ofs("mandelbrot.ppm", std::ofstream::out);
    ofs << "P3" << std::endl;
    ofs << width << " " << height << std::endl;
    ofs << 255 << std::endl;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ofs << " " << image[0][i][j] << " " << image[1][i][j] << " " << image[2][i][j] << std::endl;
        }
    }
    ofs.close();
    cout<<endl<<"Variant"<<choice<<" executed!"<<endl<<endl;
    cout << "Total pixels inside: " << pixels_inside << endl;
    cout << "Execution time (without disk I/O)): " << chrono::duration<double>(t2 - t1).count() <<endl<<endl;

    return 0;
}
