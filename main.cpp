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

    // Split getline operations on ' '
    while (getline(strm, segment, char(32))) {
        seglist.push_back(segment);
    }

    return seglist;
}

string get_line_n(ifstream &in, int n) {
    string s;
    s.reserve(32);
    in.seekg(0);

    for (int i = 0; i < n; i++) {
        getline(in, s);
    }

    return s;
}

int bayer_filter(int &nx, int &ny, ifstream &src, string &outname) {
    // Create output stream for bayer filtered image
    ofstream ofs;
    ofs.open(outname);

    // P2 is for grayscale
    ofs << "P2\n" << nx << " " << ny << "\n255\n";

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
            // R G
            // G B
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
    ofs.close();
    return 0;
}

int* get_neighbor_pixels(int &nx, int &ny, int &current_pos, ifstream &src) {
    static int pixels[6];

    // Get up + down neightbors
    pixels[0] = std::stoi(get_line_n(src, current_pos - nx));
    pixels[1] = std::stoi(get_line_n(src, current_pos + nx));

    // Get left + right neightbors
    pixels[2] = std::stoi(get_line_n(src, current_pos - 1));
    pixels[3] = std::stoi(get_line_n(src, current_pos + 1));

    // Get diagonal neighbors (top_left + bottom_right)
    pixels[4] = std::stoi(get_line_n(src, current_pos - nx - 1));
    pixels[5] = std::stoi(get_line_n(src, current_pos + nx + 1));

    return pixels;
}

int demosaic(int &nx, int &ny, ifstream &src, string &outname) {
    // Create output stream for bayer filtered image
    ofstream ofs;
    ofs.open(outname);

    // P5 is for color
    ofs << "P5\n" << nx << " " << ny << "\n255\n";

    // For parsing src
    string line;

    // Ignore the first 3 lines of the ppm file
    int offset = 3;

    for (int y = ny-1; y >= 0; y--) {
        // if (y % 5 == 0){
        fprintf(stderr,"\rDemosaicing: %5.2f%%", double(100.0*((ny-y)*nx)/(ny*nx)));
        // }
        int y_pos = ny - y;
        for (int x = 0; x < nx; x++) {
            // Store current position and pixel
            int current_pos = x + (nx * (y_pos - 1)) + offset;
            int current_pixel_val;
            string pos_val;
            pos_val = get_line_n(src, current_pos);
            current_pixel_val = std::stoi(pos_val);

            // TODO Add functionality for edge pixels
            if (x == 0 || x == nx - 1 || y == 0 || y == ny - 1) {
                ofs << "0 0 0\n";
            }
            else {
                // Get neighbor pixels and their averages
                int* np = get_neighbor_pixels(nx, ny, current_pos, src);
                int up_down_avg = static_cast<int>((np[0] + np[1]) / 2);
                int left_right_avg = static_cast<int>((np[2] + np[3]) / 2);
                int diag_avg = static_cast<int>((np[4] + np[5]) / 2);

                // Apply bayer demosaicing
                // R G
                // G B
                if (x % 2 == 1 && y % 2 == 0) {
                    // Bottom left (green)
                    ofs << up_down_avg << " " << current_pixel_val << " " << left_right_avg << "\n";
                } else if (x % 2 == 0 && y % 2 == 1) {
                    // Top right (green)
                    ofs << left_right_avg << " " << current_pixel_val << " " << up_down_avg << "\n";
                } else if (x % 2 == 1 && y % 2 == 1) {
                    // Top left (red)
                    ofs << current_pixel_val << " " << left_right_avg << " " << diag_avg << "\n";
                } else if (x % 2 == 0 && y % 2 == 0) {
                    // Bottom right (blue)
                    ofs << diag_avg << " " << left_right_avg << " " << current_pixel_val << "\n";
                }
            }
        }
    }
    ofs.close();
    return 0;
}

int main(int argc, char** argv) {
    string input_img;
    string output_bayer;
    string output_demosaic;

    output_bayer = "./bayer.ppm";
    output_demosaic = "./demosaic.ppm";

    if (2 == argc) {
        input_img = argv[1];
    } else {
        input_img = "./in.ppm";
    }

    // Input image information
    int nx = 200;
    int ny = 100;

    // Create input stream for source
    ifstream src1(input_img, std::ios::binary);

    bayer_filter(nx, ny, src1, output_bayer);
    src1.close();

    // // Create input stream for intermediate
    ifstream src2(output_bayer, std::ios::binary);

    demosaic(nx, ny, src2, output_demosaic);
    src2.close();

    return 0;
}
