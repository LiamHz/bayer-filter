// Read a ppm image file
// Run the Bayer filter over the image
// Demosaic the bayer filtered image

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

using std::cout;
using std::string;
using std::ofstream;
using std::ifstream;
using std::stringstream;

// int bayer_filter(int &x, int &y) {

// }

// Split a string into its words
std::vector<string> split_string(string &str) {
    stringstream strm(str);
    string segment;
    std::vector<string> seglist;

    // Split getline operations on '_'
    while (getline(strm, segment, char(32))) {
        seglist.push_back(segment);
    }
    // cout << seglist.at(1) << std::endl;

    return seglist;
}

int main(int argc, char** argv) {
    string filename;

    if (2 == argc) {
        filename = argv[1];
    } else {
        filename = "./in.ppm";
    }

    // Input image information
    int nx = 1920;
    int ny = 1080;

    // Create input stream for source
    ifstream src(filename, std::ios::binary);

    // Create output stream for bayer filtered image
    ofstream ofs;
    ofs.open("./bayer.ppm");

    // P5 is for grayscale
    ofs << "P5\n" << nx << " " << ny << "\n255\n";

    // For parsing src
    string line;
    std::vector<string> parsed_line;

    // Ignore the first 3 lines of the ppm file
    getline(src, line);
    getline(src, line);
    getline(src, line);

    for (int y = ny-1; y >= 0; y--) {
        if (y % 5 == 0){
            fprintf(stderr,"\rFiltering: %5.2f%%", double(100.0*((ny-y)*nx)/(ny*nx)));
        }
        for (int x = 0; x < nx; x++) {
            // Store current line of src in "line"
            getline(src, line);
            parsed_line = split_string(line);

            // Apply bayer filter
            if ((x % 2 == 1 && y % 2 == 0) || (x % 2 == 0 && y % 2 == 1)){
                // Keep green value
                ofs << parsed_line.at(1) << std::endl;
            } else if (x % 2 == 1 && y % 2 == 1) {
                // Keep red value
                ofs << parsed_line.at(0) << std::endl;
            } else if (x % 2 == 0 && y % 2 == 0) {
                // Keep blue value
                ofs << parsed_line.at(2) << std::endl;
            }
        }
    }

    src.close();
    ofs.close();
}
