/**
* @Author venom on 12/1/23.
* @File contains Array declaration template class
*/

#ifndef VENLANG_ARRAY_H
#define VENLANG_ARRAY_H
#include <iostream>
#include <vector>
// Array related classes contained here
namespace VenLangArray {
    // Array template class can hold multi-D data
    template<typename T>
    class Array {
        private:
            // Dimensionality structure
            struct Dimensions {
                // Dimensionality vector in [n][n] format if array = [[]]
                std::vector<size_t> sizes;
                // Total flat index size
                size_t totalSize;
                // Constructor to set dimensions
                explicit Dimensions(const std::vector<size_t>& dims);
                // Function to get FlatIndex of the array structure
                size_t getFlatIndex(const std::vector<size_t>& indices) const;
            };
            std::vector<T> data;
            // Class variable to handle size and dimensions
            Dimensions dimensions;

        public:
            // Constructor
            explicit Array(const std::vector<size_t> &dims);

            // Function to convert indices to a flat index
            size_t getFlatIndex(const std::vector<size_t> &indices) const;

            // Function to set a value at a specific set of indices
            void set(const std::vector<size_t> &indices, const T &value);

            // Function to get a value at a specific set of indices
            T get(const std::vector<size_t> &indices) const;

            // Function to get the sizes of each dimension
            std::vector<size_t> getDimensions() const;

            // Clear the whole array
            void clear();

            // Fill the whole array wil specific values
            void fill(const T& value);

            // resize the array
            void resize(const std::vector<size_t>& newDims);

            // check if index is valid
            bool isIndexValid(const std::vector<size_t>& indices) const;

            // Check if array is empty
            bool isEmpty() const;;

            // Handling [] operator
            T& operator[](const std::vector<size_t>& indices);

            // Handle [] operator too
            const T& operator[](const std::vector<size_t>& indices) const;
    };
}

namespace VenLangArray {

    template <typename T>
    class ArrayIterator {
    private:
        const Array<T>& array;
        size_t dimension;
        size_t currentIndex;
        std::vector<size_t> indices;

    public:
        ArrayIterator(const Array<T>& arr, size_t dim);

        bool hasNext() const;

        T next();
    };
} // namespace VenLangArray

#endif //VENLANG_ARRAY_H