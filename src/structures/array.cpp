//
// Created by venom on 12/1/23.
//

#include "array.h"

// Dimensions structure constructor
template <typename T>
VenLangArray::Array<T>::Dimensions::Dimensions(const std::vector<size_t>& dims) : sizes(dims), totalSize(0) {
    // Calculate the total size of the array
    this->totalSize = 1;
    for (size_t dim : this->sizes) {
        this->totalSize *= dim;
    }
}

// Function to convert indices to a flat index
template <typename T>
size_t VenLangArray::Array<T>::Dimensions::getFlatIndex(const std::vector<size_t>& indices) const {
    size_t flatIndex = 0;
    size_t multiplier = 1;

    // Calculate the flat index by summing up the products of each dimension
    for (int i = this->sizes.size() - 1; i >= 0; --i) {
        flatIndex += indices[i] * multiplier;
        multiplier *= this->sizes[i];
    }

    return flatIndex;
}

// Constructor
template <typename T> VenLangArray::Array<T>::Array(const std::vector<size_t> &dims) : dimensions(dims) {
    // Resize the data vector
    this->data.resize(this->dimensions.totalSize);
}

// Function to convert indices to a flat index
template <typename T> size_t VenLangArray::Array<T>::getFlatIndex(const std::vector<size_t> &indices) const {
    return this->dimensions.getFlatIndex(indices);
}

// Function to set a value at a specific set of indices
template <typename T> void VenLangArray::Array<T>::set(const std::vector<size_t> &indices, const T &value) {
    size_t flatIndex = this->getFlatIndex(indices);

    if (flatIndex < this->data.size()) {
        this->data[flatIndex] = value;
        return;
    } else {
        throw std::runtime_error("Indices out of bounds!\n");
    }
}

// Function to get a value at a specific set of indices
template <typename T> T VenLangArray::Array<T>::get(const std::vector<size_t> &indices) const {
    size_t flatIndex = this->getFlatIndex(indices);

    if (flatIndex < this->data.size()) {
        return this->data[flatIndex];
    } else {
        throw std::runtime_error("Indices out of bounds!\n");
        // Return a default value or handle the error as appropriate
        // return T{};
    }
}

// Function to get the sizes of dimensions
// getDimensions()
template <typename T>
std::vector<size_t> VenLangArray::Array<T>::getDimensions() const {
    return this->dimensions.sizes;
}

template<typename T>
void VenLangArray::Array<T>::resize(const std::vector<size_t>& newDims) {
    Dimensions newDimensions(newDims);

    // Basic error checking
    if (newDimensions.totalSize == 0 || newDimensions.totalSize != this->dimensions.totalSize) {
        throw std::runtime_error("Invalid dimensions for resize. Expected Flattened size "+ std::to_string(this->dimensions.totalSize)
        +", but got "+std::to_string(newDimensions.totalSize));
    }
    /*
    std::vector<T> newData(newDimensions.totalSize);
    std::vector<size_t> indices(newDimensions.sizes.size(), 0);

    for (size_t i = 0; i < this->dimensions.sizes.size(); ++i) {
        for (size_t j = 0; j < newDimensions.sizes.size(); ++j) {
            if (i < newDimensions.sizes.size()) {
                // Copy elements from old array to new array
                size_t copySize = std::min(this->dimensions.sizes[i], newDimensions.sizes[j]);
                for (size_t k = 0; k < copySize; ++k) {
                    indices[i] = k;
                    indices[j] = k;

                    newData[newDimensions.getFlatIndex(indices)] = data[this->dimensions.getFlatIndex(indices)];
                }
            }
        }
    }*/

    // Calculate new total size
    size_t newTotalSize = 1;
    for (const size_t dim : newDims) {
        newTotalSize *= dim;
    }

    // Resize data vector
    this->data.resize(newTotalSize);

    // Update dimensions
    this->dimensions = std::move(newDimensions);
    /*
    this->data = std::move(newData);
    this->dimensions = std::move(newDimensions);*/
}

template<typename T>
bool VenLangArray::Array<T>::isIndexValid(const std::vector<size_t>& indices) const {
    size_t numDims = this->dimensions.sizes.size();
    if (indices.size() != numDims) {
        return false;
    }

    for (size_t i = 0; i < numDims; ++i) {
        if (indices[i] >= this->dimensions.sizes[i]) {
            return false;
        }
    }

    return true;
}
template <typename T>
VenLangArray::ArrayIterator<T>::ArrayIterator(const Array<T>& arr, size_t dim) : array(arr), dimension(dim), currentIndex(0) {
    indices.resize(array.getDimensions().size(), 0);
}

template<typename T>
void VenLangArray::Array<T>::fill(const T& value) {
    std::fill(this->data.begin(), this->data.end(), value);
}
template <typename T>
bool VenLangArray::ArrayIterator<T>::hasNext() const {
    return currentIndex < array.getDimensions()[dimension];
}

template<typename T>
bool VenLangArray::Array<T>::isEmpty() const {
    return data.empty();
}

template<typename T>
T& VenLangArray::Array<T>::operator[](const std::vector<size_t>& indices) {
    if (!isIndexValid(indices)) {
        throw std::out_of_range("Index out of bounds");
    }

    return data[dimensions.getFlatIndex(indices)];
}

template<typename T>
const T& VenLangArray::Array<T>::operator[](const std::vector<size_t>& indices) const {
    if (!isIndexValid(indices)) {
        throw std::out_of_range("Index out of bounds");
    }

    return data[dimensions.getFlatIndex(indices)];
}

template<typename T>
void VenLangArray::Array<T>::clear() {
    data.clear();
    dimensions.sizes.clear();
    dimensions.totalSize = 0;
}

template <typename T>
T VenLangArray::ArrayIterator<T>::next() {
    if (!hasNext()) {
        // You can throw an exception or handle this case as appropriate.
        // Here, I'm returning a default-constructed T when there are no more elements.
        std::cerr << "No more elements in the iterator." << std::endl;
        return T{};
    }

    indices[dimension] = currentIndex;
    T result = array.get(indices);

    ++currentIndex;

    return result;
}