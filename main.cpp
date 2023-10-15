#include <fstream>
#include <iostream>
#include <stack>
#include <filesystem>

struct options {
    uint16_t length;
    uint16_t width;
    char* input;
    char* output;
    uint32_t maxIter = 0;
    uint32_t freq = 0;
};

options parseOptions(int argc, char* argv[]) {
    options opts;
    uint32_t d;

    for (uint32_t i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                if (strcmp(argv[i], "--length") == 0) {
                    ++i;
                    opts.length = atoi(argv[i]);
                }
                else if (strcmp(argv[i], "--width") == 0) {
                    ++i;
                    opts.width = atoi(argv[i]);
                }
                else if (strcmp(argv[i], "--input") == 0) {
                    ++i;
                    opts.input = argv[i];
                }
                else if (strcmp(argv[i], "--output") == 0) {
                    ++i;
                    opts.output = argv[i];
                }
                else if (strcmp(argv[i], "--max-iter") == 0) {
                    ++i;
                    opts.maxIter = atoi(argv[i]);
                }
                else if (strcmp(argv[i], "--freq") == 0) {
                    ++i;
                    opts.freq = atoi(argv[i]);
                }
            }
            else {
                d = 0;
                for (uint32_t j = 1; j < strlen(argv[i]); ++j) {
                    if (argv[i][j] == 'l') {
                        ++d;
                        opts.length = atoi(argv[i + d]);
                    }
                    else if (argv[i][j] == 'w') {
                        ++d;
                        opts.width = atoi(argv[i + d]);
                    }
                    else if (argv[i][j] == 'i') {
                        ++d;
                        opts.input = argv[i + d];
                    }
                    else if (argv[i][j] == 'o') {
                        ++d;
                        opts.output = argv[i + d];
                    }
                    else if (argv[i][j] == 'm') {
                        ++d;
                        opts.maxIter = atoi(argv[i + d]);
                    }
                    else if (argv[i][j] == 'f') {
                        ++d;
                        opts.freq = atoi(argv[i + d]);
                    }
                }
                i += d;
            }
        }
    }
    if (opts.maxIter == 0)
        opts.maxIter = 0xffffffff;
    return opts;
}

const uint16_t mtrxLength = 50;
const uint16_t mtrxWidth = mtrxLength;

struct sandHeapFieldPart {
    uint64_t mtrx[mtrxLength][mtrxWidth];
    uint16_t newSend[mtrxLength][mtrxWidth];
    int32_t xb;
    int32_t yb;
    int32_t xe;
    int32_t ye;
    sandHeapFieldPart* up = nullptr;
    sandHeapFieldPart* down = nullptr;
    sandHeapFieldPart* left = nullptr;
    sandHeapFieldPart* right = nullptr;

    void init() {
        for (int i = 0; i < mtrxLength; ++i) {
            for (int j = 0; j < mtrxLength; ++j) {
                mtrx[i][j] = 0;
                newSend[i][j] = 0;
            }
        }
    }
};

class sandHeapField {
    sandHeapFieldPart* part0 = nullptr;
    int32_t xb = 0;
    int32_t yb = 0;
    int32_t xe = 0;
    int32_t ye = 0;

public:
    int32_t minX() {
        return xb;
    }

    int32_t minY() {
        return yb;
    }

    int32_t maxX() {
        return xe;
    }

    int32_t maxY() {
        return ye;
    }

    uint64_t get(int32_t x, int32_t y) {
        sandHeapFieldPart* part = part0;
        while (part != nullptr) {
            if (x < part->xb)
                part = part->left;
            else if (x >= part->xe)
                part = part->right;
            else if (y < part->yb)
                part = part->down;
            else if (y >= part->ye)
                part = part->up;
            else
                return part->mtrx[x - part->xb][y - part->yb];
        }
        return 0;
    }

    void set(int32_t x, int32_t y, uint64_t val) {
        if (x < xb)
            xb = x;
        if (x >= xe)
            xe = x + 1;
        if (y < yb)
            yb = y;
        if (y >= ye)
            ye = y + 1;
        if (part0 == nullptr) {
            part0 = new sandHeapFieldPart;
            part0->init();
            part0->xb = 0;
            part0->xe = mtrxLength;
            part0->yb = 0;
            part0->ye = mtrxWidth;
        }
        sandHeapFieldPart* part = part0;
        while (true) {
            if (x < part->xb) {
                if (part->left == nullptr) {
                    part->left = new sandHeapFieldPart;
                    part->left->init();
                    part->left->right = part;
                    part->left->xb = part->xb - mtrxLength;
                    part->left->xe = part->xe - mtrxLength;
                    part->left->yb = part->yb;
                    part->left->ye = part->ye;
                }
                part = part->left;
            }
            else if (x >= part->xe) {
                if (part->right == nullptr) {
                    part->right = new sandHeapFieldPart;
                    part->right->init();
                    part->right->left = part;
                    part->right->xb = part->xb + mtrxLength;
                    part->right->xe = part->xe + mtrxLength;
                    part->right->yb = part->yb;
                    part->right->ye = part->ye;
                }
                part = part->right;
            }
            else if (y < part->yb) {
                if (part->down == nullptr) {
                    part->down = new sandHeapFieldPart;
                    part->down->init();
                    part->down->up = part;
                    part->down->xb = part->xb;
                    part->down->xe = part->xe;
                    part->down->yb = part->yb - mtrxWidth;
                    part->down->ye = part->ye - mtrxWidth;
                }
                part = part->down;
            }
            else if (y >= part->ye) {
                if (part->up == nullptr) {
                    part->up = new sandHeapFieldPart;
                    part->up->init();
                    part->up->right = part;
                    part->up->xb = part->xb;
                    part->up->xe = part->xe;
                    part->up->yb = part->yb + mtrxWidth;
                    part->up->ye = part->ye + mtrxWidth;
                }
                part = part->up;
            }
            else {
                part->mtrx[x - part->xb][y - part->yb] = val;
                return;
            }
        }
    }

    uint8_t getAddedSend(int32_t x, int32_t y) {
        sandHeapFieldPart* part = part0;
        while (part != nullptr) {
            if (x < part->xb)
                part = part->left;
            else if (x >= part->xe)
                part = part->right;
            else if (y < part->yb)
                part = part->down;
            else if (y >= part->ye)
                part = part->up;
            else {
                uint8_t res = part->newSend[x - part->xb][y - part->yb];
                part->newSend[x - part->xb][y - part->yb] = 0;
                return res;
            }
        }
        return 0;
    }

    void addSend(int32_t x, int32_t y) {
        if (x < xb)
            xb = x;
        if (x >= xe)
            xe = x + 1;
        if (y < yb)
            yb = y;
        if (y >= ye)
            ye = y + 1;
        if (part0 == nullptr) {
            part0 = new sandHeapFieldPart;
            part0->init();
            part0->xb = 0;
            part0->xe = mtrxLength;
            part0->yb = 0;
            part0->ye = mtrxWidth;
        }
        sandHeapFieldPart* part = part0;
        while (true) {
            if (x < part->xb) {
                if (part->left == nullptr) {
                    part->left = new sandHeapFieldPart;
                    part->left->init();
                    part->left->right = part;
                    part->left->xb = part->xb - mtrxLength;
                    part->left->xe = part->xe - mtrxLength;
                    part->left->yb = part->yb;
                    part->left->ye = part->ye;
                }
                part = part->left;
            }
            else if (x >= part->xe) {
                if (part->right == nullptr) {
                    part->right = new sandHeapFieldPart;
                    part->right->init();
                    part->right->left = part;
                    part->right->xb = part->xb + mtrxLength;
                    part->right->xe = part->xe + mtrxLength;
                    part->right->yb = part->yb;
                    part->right->ye = part->ye;
                }
                part = part->right;
            }
            else if (y < part->yb) {
                if (part->down == nullptr) {
                    part->down = new sandHeapFieldPart;
                    part->down->init();
                    part->down->up = part;
                    part->down->xb = part->xb;
                    part->down->xe = part->xe;
                    part->down->yb = part->yb - mtrxWidth;
                    part->down->ye = part->ye - mtrxWidth;
                }
                part = part->down;
            }
            else if (y >= part->ye) {
                if (part->up == nullptr) {
                    part->up = new sandHeapFieldPart;
                    part->up->init();
                    part->up->right = part;
                    part->up->xb = part->xb;
                    part->up->xe = part->xe;
                    part->up->yb = part->yb + mtrxWidth;
                    part->up->ye = part->ye + mtrxWidth;
                }
                part = part->up;
            }
            else {
                ++(part->newSend[x - part->xb][y - part->yb]);
                return;
            }
        }
    }

    void destroy() {
        std::stack<sandHeapFieldPart*> partStack;
        partStack.push(part0);
        sandHeapFieldPart* part;
        while (!partStack.empty()) {
            part = partStack.top();
            partStack.pop();
            if (part == nullptr)
                continue;
            partStack.push(part->up);
            partStack.push(part->down);
            partStack.push(part->left);
            partStack.push(part->right);
            delete part;
        }
    }
};

void readTsv(sandHeapField& sandHeap, char* file, int32_t length, int32_t width) {
    std::ifstream tsv;
    tsv.open(file);
    uint64_t val;
    for (int32_t x = 0; x < length; ++x) {
        for (int32_t y = 0; y < width; ++y) {
            tsv >> val;
            sandHeap.set(x, y, val);
        }
    }
    tsv.close();
}

void printInt16(short n, std::ostream& out) {
    const unsigned int bytesInInt = 2;
    const unsigned int sizeOfByte = 8;
    for (uint8_t i = 0; i < bytesInInt; ++i) {
        out << (unsigned char)(n - ((n >> sizeOfByte) << sizeOfByte));
        n >>= sizeOfByte;
    }
}

void printInt32(int n, std::ostream& out) {
    const unsigned int bytesInInt = 4;
    const unsigned int sizeOfByte = 8;
    for (uint8_t i = 0; i < bytesInInt; ++i) {
        out << (unsigned char)(n - ((n >> sizeOfByte) << sizeOfByte));
        n >>= sizeOfByte;
    }
}

void insertInt(unsigned char* arr, unsigned int val, unsigned int place) {
    const unsigned int bytesInInt = 4;
    const unsigned int sizeOfByte = 8;
    for (uint8_t i = place; i < place + bytesInInt; ++i) {
        arr[i] = (unsigned char)(val - ((val >> sizeOfByte) << sizeOfByte));
        val >>= sizeOfByte;
    }
}

void saveSandHeapToBMP(sandHeapField& heap, char* file) {
    unsigned char header[118] = {0x42, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00};
    
    unsigned int biWidth = heap.maxX() - heap.minX();
    unsigned int biHeight = heap.maxY() - heap.minY();
    unsigned int scanLength = biWidth / 2 + biWidth % 2;
    unsigned int bfSize = (unsigned int)(sizeof header) + (scanLength + (4 - (scanLength % 4)) % 4) * biHeight;

    insertInt(header, bfSize, 2);
    insertInt(header, biWidth, 18);
    insertInt(header, biHeight, 22);

    std::ofstream bmp;
    bmp.open(file, std::ios_base::binary);
    for (int i = 0; i < sizeof header; ++i)
        bmp << header[i];

    unsigned char color[] = { 15, 10, 13, 11, 0 };
    unsigned char buf = 0;
    bool bf = 1;
    unsigned char bytesWrited = 0;
    uint64_t gr;

    for (int x = heap.maxX() - 1; x >= heap.minX(); --x) {
        for (int y = heap.minY(); y <= heap.maxY(); ++y) {
            buf <<= 4;
            gr = heap.get(x, y);
            if (gr < 4)
                buf += color[gr];
            else
                buf += color[4];
            bf = !bf;
            if (bf) {
                bmp << buf;
                bytesWrited = (bytesWrited + 1) % 4;
            }
        }
        while (bytesWrited || !bf) {
            buf <<= 1;
            bf = !bf;
            if (bf) {
                bmp << buf;
                bytesWrited = (bytesWrited + 1) % 4;
            }
        }
    }
    
    bmp.close();
}

void makeBitmapPath(char* bitmapPath, char* folderPath, unsigned int bitmapNumber) {
    sprintf(bitmapPath, "%s/%i.bmp", folderPath, bitmapNumber);
}

int main(int argc, char* argv[]) {
    options opts = parseOptions(argc, argv);
    sandHeapField sandHeap;
    readTsv(sandHeap, opts.input, opts.length, opts.width);
    char bitmapPath[(sizeof opts.output) + 15];
    uint32_t i = 0;
    bool heapMutated;
    uint64_t sndCnt;
    uint32_t bmn = 0;

    std::filesystem::create_directory(opts.output);

    while (i < opts.maxIter) {
        heapMutated = 0;
        for (int x = sandHeap.minX(); x < sandHeap.maxX(); ++x) {
            for (int y = sandHeap.minY(); y < sandHeap.maxY(); ++y) {
                sndCnt = sandHeap.get(x, y);
                if (sndCnt >= 4) {
                    sandHeap.set(x, y, sndCnt - 4);
                    sandHeap.addSend(x - 1, y);
                    sandHeap.addSend(x + 1, y);
                    sandHeap.addSend(x, y - 1);
                    sandHeap.addSend(x, y + 1);
                    heapMutated = 1;
                }
            }
        }
        for (int x = sandHeap.minX(); x < sandHeap.maxX(); ++x) {
            for (int y = sandHeap.minY(); y < sandHeap.maxY(); ++y) {
                sandHeap.set(x, y, sandHeap.get(x, y) + sandHeap.getAddedSend(x, y));
            }
        }
        ++i;
        if ((opts.freq != 0) && (i % opts.freq == 0)) {
            makeBitmapPath(bitmapPath, opts.output, bmn);
            saveSandHeapToBMP(sandHeap, bitmapPath);
            ++bmn;
        }
        if (!heapMutated)
            break;
    }
    if (1) {
        makeBitmapPath(bitmapPath, opts.output, bmn);
        saveSandHeapToBMP(sandHeap, bitmapPath);
    }
    sandHeap.destroy();
    return 0;
}