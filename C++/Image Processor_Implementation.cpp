#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>

const size_t FSIZE = 54;
const long double MAXX = 255.0;
const long double MINN = 0.0;
const double THOUS = 1000.0;
const double FFIRST = 299.0;
const double FSECOND = 587.;
const double FTHIRD = 114.;
const int SIX = 6;
const long double FIVE = 5.0;
const long double FOUR = 4.0;
const uint16_t NAME = 0x4D42;

using BITMAPFILEHEADER = struct __attribute__((packed)) BITMAPFILEHEADER {
    uint16_t type;
    uint32_t size;
    uint16_t reserv1;
    uint16_t reserv2;
    uint32_t offset;
};

using BITMAPINFOHEADER = struct __attribute__((packed)) BITMAPINFOHEADER {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t copmress;
    uint32_t image_size;
    int32_t horiz;
    int32_t vert;
    uint32_t color_num;
    uint32_t color_imp;
};

struct BGR {
    long double b = 0, g = 0, r = 0;
    BGR() = default;
    BGR(long double b, long double g, long double r) : b(b), g(g), r(r) {
    }
};

class Filter;

class Image {
private:
    std::vector<std::vector<BGR>> img_;
    unsigned char* from_rgb_;

public:
    BITMAPFILEHEADER file_header_;
    BITMAPINFOHEADER info_header_;
    friend class Filter;
    friend class GreyScale;
    friend class Negative;
    friend class Sharp;
    friend class GaussianBlur;
    friend class SinusDistortion;
    friend class GlassDistortion;
    friend class EdgeDetection;
    friend class Crop;
    Image(const unsigned char* massive, BITMAPFILEHEADER file_header, BITMAPINFOHEADER info_header)
            : file_header_(file_header), info_header_(info_header) {
        int rw = RealWidth();
        img_ = std::vector<std::vector<BGR>>(info_header_.height, std::vector<BGR>(info_header_.width));
        for (int32_t i = 0; i < info_header_.height; ++i) {
            for (int32_t j = 0; j < info_header_.width; ++j) {
                img_[i][j].b = static_cast<long double>(massive[i * rw + 3 * j + 0]);
                img_[i][j].g = static_cast<long double>(massive[i * rw + 3 * j + 1]);
                img_[i][j].r = static_cast<long double>(massive[i * rw + 3 * j + 2]);
            }
        }
    }

    void ApplyFilter(Filter& filter, double first, double second);

    unsigned char* CreateBinSequence() {
        from_rgb_ = static_cast<unsigned char*>(malloc(file_header_.size - FSIZE));
        for (uint32_t i = 0; i < (file_header_.size - FSIZE); ++i) {
            from_rgb_[i] = 0;
        }
        int rw = RealWidth();
        for (int32_t i = 0; i < info_header_.height; ++i) {
            for (int32_t j = 0; j < info_header_.width; ++j) {
                from_rgb_[i * rw + 3 * j + 0] = static_cast<int32_t>(img_[i][j].b);
                from_rgb_[i * rw + 3 * j + 1] = static_cast<int32_t>(img_[i][j].g);
                from_rgb_[i * rw + 3 * j + 2] = static_cast<int32_t>(img_[i][j].r);
            }
        }
        return from_rgb_;
    }

    int RealWidth() const {
        return info_header_.width * 3 + (4 - info_header_.width * 3 % 4) % 4;
    }

    ~Image() {
        free(from_rgb_);
    }
};

class Filter {
public:
    virtual void Apply(Image& image, double first, double second) = 0;
};

class Negative : public Filter {
public:
    void Apply(Image& image, double first, double second) override {
        for (int32_t i = 0; i < image.info_header_.height; ++i) {
            for (int32_t j = 0; j < image.info_header_.width; ++j) {
                image.img_[i][j].b = MAXX - image.img_[i][j].b;
                image.img_[i][j].g = MAXX - image.img_[i][j].g;
                image.img_[i][j].r = MAXX - image.img_[i][j].r;
            }
        }
    }
};

class GreyScale : public Filter {
public:
    void Apply(Image& image, double first, double second) override {
        for (int32_t i = 0; i < image.info_header_.height; ++i) {
            for (int32_t j = 0; j < image.info_header_.width; ++j) {
                long double color = image.img_[i][j].r * FFIRST / THOUS + image.img_[i][j].g * FSECOND / THOUS +
                                    image.img_[i][j].b * FTHIRD / THOUS;
                image.img_[i][j].b = std::min(MAXX, color);
                image.img_[i][j].g = std::min(MAXX, color);
                image.img_[i][j].r = std::min(MAXX, color);
            }
        }
    }
};

class Crop : public Filter {
public:
    void Apply(Image& image, double to_x, double to_y) override {
        auto x = static_cast<int32_t>(to_x);
        auto y = static_cast<int32_t>(to_y);
        int32_t height = image.info_header_.height;
        int32_t width = image.info_header_.width;
        if (x >= width && y >= height) {
            std::cout << "No changes" << std::endl;
        } else {
            std::vector<std::vector<BGR>> tmp(std::min(height, y), std::vector<BGR>(std::min(x, width)));
            for (int32_t i = 0; i < std::min(height, y); ++i) {
                for (int32_t j = 0; j < std::min(width, x); ++j) {
                    tmp[y - 1 - i][j] = image.img_[height - 1 - i][j];
                }
            }
            image.info_header_.width = static_cast<int32_t>(std::min(width, x));
            image.info_header_.height = static_cast<int32_t>(std::min(height, y));
            int rw = std::min(width, x) * 3 + (4 - std::min(width, x) * 3 % 4) % 4;
            image.file_header_.size = static_cast<uint32_t>(rw * std::min(height, y) + FSIZE);
            image.img_ = tmp;
        }
    }
};

class Sharp : public Filter {
public:
    void Apply(Image& image, double first, double second) override {
        std::vector<std::vector<BGR>> tmp(image.info_header_.height, std::vector<BGR>(image.info_header_.width));
        for (int32_t i = 0; i < image.info_header_.height; ++i) {
            for (int32_t j = 0; j < image.info_header_.width; ++j) {
                long double red = -image.img_[i][std::max(0, j - 1)].r - image.img_[std::max(0, i - 1)][j].r +
                                  FIVE * image.img_[i][j].r -
                                  image.img_[std::min(i + 1, abs(image.info_header_.height) - 1)][j].r -
                                  image.img_[i][std::min(j + 1, abs(image.info_header_.width) - 1)].r;
                long double blue = -image.img_[i][std::max(0, j - 1)].b - image.img_[std::max(0, i - 1)][j].b +
                                   FIVE * image.img_[i][j].b -
                                   image.img_[std::min(i + 1, abs(image.info_header_.height) - 1)][j].b -
                                   image.img_[i][std::min(j + 1, abs(image.info_header_.width) - 1)].b;
                long double green = -image.img_[i][std::max(0, j - 1)].g - image.img_[std::max(0, i - 1)][j].g +
                                    FIVE * image.img_[i][j].g -
                                    image.img_[std::min(i + 1, abs(image.info_header_.height) - 1)][j].g -
                                    image.img_[i][std::min(j + 1, abs(image.info_header_.width) - 1)].g;
                tmp[i][j].r = std::min(MAXX, std::max(red, MINN));
                tmp[i][j].b = std::min(MAXX, std::max(blue, MINN));
                tmp[i][j].g = std::min(MAXX, std::max(green, MINN));
            }
        }
        image.img_ = tmp;
    }
};

class EdgeDetection : public Filter {
public:
    void Apply(Image& image, double threshold, double second) override {
        for (int32_t i = 0; i < image.info_header_.height; ++i) {
            for (int32_t j = 0; j < image.info_header_.width; ++j) {
                long double color = image.img_[i][j].r * FFIRST / THOUS + image.img_[i][j].g * FSECOND / THOUS +
                                    image.img_[i][j].b * FTHIRD / THOUS;
                image.img_[i][j].b = std::min(MAXX, color);
                image.img_[i][j].g = std::min(MAXX, color);
                image.img_[i][j].r = std::min(MAXX, color);
            }
        }
        std::vector<std::vector<BGR>> tmp(image.info_header_.height, std::vector<BGR>(image.info_header_.width));
        for (int32_t i = 0; i < image.info_header_.height; ++i) {
            for (int32_t j = 0; j < image.info_header_.width; ++j) {
                long double new_grey = -image.img_[i][std::max(0, j - 1)].r - image.img_[std::max(0, i - 1)][j].r +
                                       FOUR * image.img_[i][j].r -
                                       image.img_[std::min(i + 1, abs(image.info_header_.height) - 1)][j].r -
                                       image.img_[i][std::min(j + 1, abs(image.info_header_.width) - 1)].r;
                new_grey = std::min(MAXX, std::max(new_grey, MINN));
                if (std::isgreater(new_grey, MAXX * threshold)) {
                    new_grey = MAXX;
                } else {
                    new_grey = MINN;
                }
                tmp[i][j].b = new_grey;
                tmp[i][j].g = new_grey;
                tmp[i][j].r = new_grey;
            }
        }
        image.img_ = tmp;
    }
};

class SinusDistortion : public Filter {
public:
    void Apply(Image& image, double wave, double second) override {
        wave = std::abs(wave);
        int32_t height = image.info_header_.height;
        int32_t width = image.info_header_.width;
        std::vector<std::vector<BGR>> tmp(height, std::vector<BGR>(width));
        for (int32_t i = 0; i < height; ++i) {
            for (int32_t j = 0; j < width; ++j) {
                int new_i = (static_cast<int>(i + wave * (sin(i / wave) * sin(j / wave)))) % height;
                int new_j = (static_cast<int>(j + wave * (sin(i / wave) * sin(j / wave)))) % width;
                tmp[i][j] = image.img_[new_i][new_j];
            }
        }
        image.img_ = tmp;
    }
};

class GlassDistortion : public Filter {
public:
    void Apply(Image& image, double radius, double second) override {
        radius = std::abs(radius);
        std::random_device rd;
        std::mt19937 gen(rd());
        int32_t height = image.info_header_.height;
        int32_t width = image.info_header_.width;
        std::vector<std::vector<BGR>> tmp(height, std::vector<BGR>(width));
        for (int32_t i = 0; i < height; ++i) {
            for (int32_t j = 0; j < width; ++j) {
                std::uniform_int_distribution<> distrib(0, static_cast<int>(radius));
                int rand_x = distrib(gen);
                int rand_y = distrib(gen);
                rand_x = (j + rand_x) % width;
                rand_y = (i + rand_y) % height;
                tmp[i][j] = image.img_[rand_y][rand_x];
            }
        }
        image.img_ = tmp;
    }
};

class GaussianBlur : public Filter {
public:
    void Apply(Image& image, double sigma, double second) override {
        int32_t height = image.info_header_.height;
        int32_t width = image.info_header_.width;
        std::vector<std::vector<BGR>> tmp(height, std::vector<BGR>(width));
        int len = static_cast<int>(SIX * std::abs(sigma)) + 1;
        len += 1 - (len % 2);
        int mid = len / 2;
        std::vector<long double> coeffs(len);
        long double denom = std::sqrt(2 * M_PI * sigma * sigma);
        long double degree = 2 * sigma * sigma;
        for (int i = 0; i <= mid; ++i) {
            coeffs[mid - i] = std::pow(M_E, -static_cast<long double>(i * i) / degree) / denom;
            coeffs[mid + i] = std::pow(M_E, -static_cast<long double>(i * i) / degree) / denom;
        }
        for (int32_t i = 0; i < width; ++i) {
            for (int32_t j = 0; j < height; ++j) {
                tmp[i][j].r = 0;
                tmp[i][j].b = 0;
                tmp[i][j].b = 0;
                for (int32_t k = -mid; k <= mid; ++k) {
                    int num = std::min(height - 1, std::max(0, j + k));
                    tmp[i][j].r += image.img_[num][i].r * coeffs[k + mid];
                    tmp[i][j].b += image.img_[num][i].b * coeffs[k + mid];
                    tmp[i][j].g += image.img_[num][i].g * coeffs[k + mid];
                }
            }
        }

        for (int32_t j = 0; j < height; ++j) {
            for (int32_t i = 0; i < width; ++i) {
                image.img_[j][i].r = 0;
                image.img_[j][i].b = 0;
                image.img_[j][i].g = 0;
                for (int32_t k = -mid; k <= mid; ++k) {
                    int num = std::min(width - 1, std::max(0, i + k));
                    image.img_[j][i].r += tmp[j][num].r * coeffs[k + mid];
                    image.img_[j][i].b += tmp[j][num].b * coeffs[k + mid];
                    image.img_[j][i].g += tmp[j][num].g * coeffs[k + mid];
                }
                image.img_[j][i].r = std::max(MINN, std::min(MAXX, image.img_[j][i].r));
                image.img_[j][i].b = std::max(MINN, std::min(MAXX, image.img_[j][i].b));
                image.img_[j][i].g = std::max(MINN, std::min(MAXX, image.img_[j][i].g));
            }
        }
    }
};

void Image::ApplyFilter(Filter& filter, double first, double second) {
    filter.Apply(*this, first, second);
}

struct FiltArgs {
    std::string filter_name;
    double first = 0;
    double second = 0;
};

Image Reader(const char* input, const std::vector<FiltArgs>& filtargs) {
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;

    FILE* file = fopen(input, "rb");
    if (!file) {
        std::cout << "Error: can't open file '" << input << "'" << std::endl;
        exit(0);
    }

    size_t read_bytes = fread(&file_header, sizeof(file_header), 1, file);
    if (read_bytes != 1) {
        std::cout << "Error: can't read file header" << std::endl;
    }

    if (file_header.type != NAME) {
        std::cout << "Error: wrong image format, only BMP format is supported" << std::endl;
        exit(0);
    }

    fread(&info_header, sizeof(info_header), 1, file);

    size_t bitmap_size = file_header.size - file_header.offset;
    unsigned char* is_rgb = static_cast<unsigned char*>(malloc(bitmap_size * sizeof(unsigned char)));
    if (is_rgb == nullptr) {
        std::cout << "Error: can't allocate memory" << std::endl;
        exit(0);
    }
    fread(is_rgb, bitmap_size, 1, file);

    Image image(is_rgb, file_header, info_header);
    for (const auto& filt : filtargs) {
        if (filt.filter_name == "-gs") {
            GreyScale f1;
            image.ApplyFilter(f1, 0, 0);
        } else if (filt.filter_name == "-neg") {
            Negative f1;
            image.ApplyFilter(f1, 0, 0);
        } else if (filt.filter_name == "-sharp") {
            Sharp f1;
            image.ApplyFilter(f1, 0, 0);
        } else if (filt.filter_name == "-edge") {
            EdgeDetection f1;
            image.ApplyFilter(f1, filt.first, 0);
        } else if (filt.filter_name == "-blur") {
            GaussianBlur f1;
            image.ApplyFilter(f1, filt.first, 0);
        } else if (filt.filter_name == "-glass") {
            GlassDistortion f1;
            image.ApplyFilter(f1, filt.first, 0);
        } else if (filt.filter_name == "-sinus") {
            SinusDistortion f1;
            image.ApplyFilter(f1, filt.first, 0);
        } else if (filt.filter_name == "-crop") {
            Crop f1;
            image.ApplyFilter(f1, filt.first, filt.second);
        }
    }

    free(is_rgb);
    return image;
}

void Writer(Image& image, const char* output) {
    FILE* outfile = fopen(output, "wb");
    if (!outfile) {
        std::cout << "Error: can't create output file '" << output << "'" << std::endl;
        exit(0);
    }

    fwrite(&image.file_header_, sizeof(char), sizeof(BITMAPFILEHEADER), outfile);
    fwrite(&image.info_header_, sizeof(char), sizeof(BITMAPINFOHEADER), outfile);
    fwrite(image.CreateBinSequence(), sizeof(unsigned char), image.file_header_.size - FSIZE, outfile);
    fclose(outfile);
}

void Reference() {
    std::cout << "To apply a filter to an image, enter arguments in the command line separated by a space."
              << std::endl;
    std::cout << "The first argument is the name of the program: ./image_processor" << std::endl;
    std::cout << "The second argument is the path to the input file, example: input.bmp" << std::endl;
    std::cout << "The third argument is the path to the output file, example: /tmp/output.bmp" << std::endl;
    std::cout << "Then select one of the filters:" << std::endl;
    std::cout << "1) GreyScale: called by the -gs command, does not take parameters;" << std::endl;
    std::cout << "2) Negative: called by the -neg command, does not take parameters;" << std::endl;
    std::cout << "3) Sharp: called by the -sharp command, does not take parameters;" << std::endl;
    std::cout << "4) EdgeDetection: called by the -sharp command, takes a single parameter - threshold;" << std::endl;
    std::cout << "5) GaussianBlur: called by the -blur command, takes a single parameter - sigma;" << std::endl;
    std::cout << "6) GlassDistortion: called by the -glass command, accepts a single parameter - radius;" << std::endl;
    std::cout << "7) SinusDistortion: called by the -sinus command, takes a single parameter - wave;" << std::endl;
    std::cout
            << "8) Crop: called by the -copy command, it takes two parameters - width (in pixels) and height (in pixels)."
            << std::endl;
    std::cout << "Pay attention to the number of parameters of each filter. All filter parameters are entered after "
                 "the filter name. Each parameter is a number."
              << std::endl;
    std::cout << "When you have entered all the arguments, you should get a line like: {program name} {path to input "
                 "file} {path to output file} [-{filter name 1} [filter parameter 1] [filter parameter 2] ...] "
                 "[-{filter name 2} [filter parameter 1] [filter parameter 2] ...] ..."
              << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        Reference();
        exit(0);
    } else if (argc == 3) {
        const char* input = argv[1];
        const char* output = argv[2];
        std::vector<FiltArgs> filters;
        Image image = Reader(input, filters);
        Writer(image, output);
    } else {
        std::vector<FiltArgs> filters;
        const char* input = argv[1];
        const char* output = argv[2];
        int ind = 3;
        while (ind < argc) {
            if (static_cast<std::string>(argv[ind]) == "-crop") {
                if (ind + 1 >= argc || ind + 2 >= argc || isalpha(argv[ind + 1][0]) || isalpha(argv[ind + 2][0])) {
                    std::cout << "Incorrect arguments input: filter Crop takes exactly 2 numeric arguments"
                              << std::endl;
                    exit(0);
                } else {
                    FiltArgs tmp;
                    tmp.filter_name = "-crop";
                    tmp.first = atof(argv[ind + 1]);
                    tmp.second = atof(argv[ind + 2]);
                    filters.push_back(tmp);
                    ind += 3;
                }
            } else if (static_cast<std::string>(argv[ind]) == "-edge") {
                if (ind + 1 >= argc || isalpha(argv[ind + 1][0])) {
                    std::cout << "Incorrect arguments input: filter EdgeDetection takes exactly 1 numeric argument."
                              << std::endl;
                    exit(0);
                } else {
                    FiltArgs tmp;
                    tmp.filter_name = "-edge";
                    tmp.first = atof(argv[ind + 1]);
                    filters.push_back(tmp);
                    ind += 2;
                }
            } else if (static_cast<std::string>(argv[ind]) == "-blur") {
                if (ind + 1 >= argc || isalpha(argv[ind + 1][0])) {
                    std::cout << "Incorrect arguments input: filter GaussianBlur takes exactly 1 numeric argument."
                              << std::endl;
                    exit(0);
                } else {
                    FiltArgs tmp;
                    tmp.filter_name = "-blur";
                    tmp.first = atof(argv[ind + 1]);
                    filters.push_back(tmp);
                    ind += 2;
                }
            } else if (static_cast<std::string>(argv[ind]) == "-glass") {
                if (ind + 1 >= argc || isalpha(argv[ind + 1][0])) {
                    std::cout << "Incorrect arguments input: filter GlassDistortion takes exactly 1 numeric argument."
                              << std::endl;
                    exit(0);
                } else {
                    FiltArgs tmp;
                    tmp.filter_name = "-glass";
                    tmp.first = atof(argv[ind + 1]);
                    filters.push_back(tmp);
                    ind += 2;
                }
            } else if (static_cast<std::string>(argv[ind]) == "-sinus") {
                if (ind + 1 >= argc || isalpha(argv[ind + 1][0])) {
                    std::cout << "Incorrect arguments input: filter SinusDistortion takes exactly 1 numeric argument."
                              << std::endl;
                    exit(0);
                } else {
                    FiltArgs tmp;
                    tmp.filter_name = "-sinus";
                    tmp.first = atof(argv[ind + 1]);
                    filters.push_back(tmp);
                    ind += 2;
                }
            } else if (static_cast<std::string>(argv[ind]) == "-gs") {
                FiltArgs tmp;
                tmp.filter_name = "-gs";
                filters.push_back(tmp);
                ind += 1;
            } else if (static_cast<std::string>(argv[ind]) == "-neg") {
                FiltArgs tmp;
                tmp.filter_name = "-neg";
                filters.push_back(tmp);
                ind += 1;
            } else if (static_cast<std::string>(argv[ind]) == "-sharp") {
                FiltArgs tmp;
                tmp.filter_name = "-sharp";
                filters.push_back(tmp);
                ind += 1;
            } else {
                std::cout << "Incorrect arguments input." << std::endl;
                exit(0);
            }
        }
        Image image = Reader(input, filters);
        Writer(image, output);
    }
    return 0;
}