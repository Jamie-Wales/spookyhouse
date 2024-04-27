#include <iostream>
#include <ostream>
#include <vector>

template<typename T>
class Array2D {
public:
    std::shared_ptr<std::vector<std::vector<T>>> data = std::make_shared<std::vector<std::vector<T>>>();
    int row;
    int col;

    Array2D()
            : row(0), col(0) {};

    Array2D(int row, int col)
            : row(row), col(col) {
        (*data).resize(row);
        for (int i = 0; i < row; i++) {
            (*data)[i].resize(col);
        }
    }

    Array2D(int row, int col, T t)
            : row(row), col(col) {
        (*data).resize(row);
        for (int i = 0; i < col; i++) {
            (*data)[i].resize(row);
            for (int j = 0; j < row; j++) {
                (*data)[i][j] = t;
            }
        }
    }


    Array2D(int row, int col, T *ptr)
            : row(row), col(col) {
        (*data).resize(row);
        for (int i = 0; i < row; i++) {
            (*data)[i].resize(col);
            for (int j = 0; j < col; j++) {
                (*data)[i][j] = *ptr;
                ptr++;
            }
        }
    }

    Array2D(std::vector<std::vector<T>> data) {
        this->data = std::make_shared<std::vector<std::vector<T>>>(data);
        this->row = data.size();
        this->col = data[0].size();
    };

    T &operator()(int row, int col) {
        if (row > this->row || col > this->col || row < 0 || col < 0) {
            std::cerr << "Array2D out of bound" << std::endl;
            return (*data)[0][0];
        }
        return (*data)[row][col];
    }


    void GetMinMax(T &Min, T &Max) {
        Max = Min = (*data)[0][0];

        for (int i = 1; i < col; i++)
            for (int j = 1; j < row; j++) {
                if ((*data)[i][j] < Min) {
                    Min = (*data)[i][j];
                }

                if ((*data)[i][j] > Max) {
                    Min = (*data)[i][j];
                }
            }
    }

    void normalize(T minRange, T maxRange) {
        T min, max;
        GetMinMax(min, max);
        if (max <= min) {
            return;
        }

        T minmaxdelta = max - min;
        T minmarange = maxRange - minRange;
        for (int i = 0; i < col; i++)
            for (int j = 0; j < row; j++)
                (*data)[i][j] = (((*data)[i][j] - min) / minmaxdelta) * minmarange + minRange;

    }
};
