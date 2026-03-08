
#include <cusparse.h> 

#include "memory_manager.hpp"


namespace mcutil {


    class DenseToCOOSparseHandler : public DeviceMemoryHandlerInterface {
    protected:
        cusparseHandle_t     _handle;
        cusparseDnMatDescr_t _mat_dense;
        cusparseSpMatDescr_t _mat_sparse;
        void*                _buffer;
        int*                 _d_row;
    public:


        DenseToCOOSparseHandler(size_t size, float* dense_ptr, int* coo_index_ptr, float* coo_val_ptr);


        ~DenseToCOOSparseHandler();


        /**
        * @brief Convert dense vector to sparse vector.
        *        Using value in device array 'dense_ptr'
        * 
        * @return number of non-zero elements
        */
        size_t convert();


    };


}

